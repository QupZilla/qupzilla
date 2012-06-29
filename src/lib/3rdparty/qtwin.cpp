/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#include "qtwin.h"
#include <QLibrary>
#include <QApplication>
#include <QWidget>
#include <QList>
#include "history.h"
#include "mainapplication.h"

#ifdef Q_WS_WIN
#include <qt_windows.h>

// Blur behind data structures
#define DWM_BB_ENABLE                 0x00000001  // fEnable has been specified
#define DWM_BB_BLURREGION             0x00000002  // hRgnBlur has been specified
#define DWM_BB_TRANSITIONONMAXIMIZED  0x00000004  // fTransitionOnMaximized has been specified
#define WM_DWMCOMPOSITIONCHANGED        0x031E    // Composition changed window message

typedef struct _DWM_BLURBEHIND {
    DWORD dwFlags;
    BOOL fEnable;
    HRGN hRgnBlur;
    BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND, *PDWM_BLURBEHIND;

typedef struct _MARGINS {
    int cxLeftWidth;
    int cxRightWidth;
    int cyTopHeight;
    int cyBottomHeight;
} MARGINS, *PMARGINS;

typedef HRESULT(WINAPI* PtrDwmIsCompositionEnabled)(BOOL* pfEnabled);
typedef HRESULT(WINAPI* PtrDwmExtendFrameIntoClientArea)(HWND hWnd, const MARGINS* pMarInset);
typedef HRESULT(WINAPI* PtrDwmEnableBlurBehindWindow)(HWND hWnd, const DWM_BLURBEHIND* pBlurBehind);
typedef HRESULT(WINAPI* PtrDwmGetColorizationColor)(DWORD* pcrColorization, BOOL* pfOpaqueBlend);

static PtrDwmIsCompositionEnabled pDwmIsCompositionEnabled = 0;
static PtrDwmEnableBlurBehindWindow pDwmEnableBlurBehindWindow = 0;
static PtrDwmExtendFrameIntoClientArea pDwmExtendFrameIntoClientArea = 0;
static PtrDwmGetColorizationColor pDwmGetColorizationColor = 0;


/*
 * Internal helper class that notifies windows if the
 * DWM compositing state changes and updates the widget
 * flags correspondingly.
 */
class QT_QUPZILLA_EXPORT WindowNotifier : public QWidget
{
public:
    WindowNotifier() {
        winId();
    }
    void addWidget(QWidget* widget) {
        widgets.append(widget);
    }
    void removeWidget(QWidget* widget) {
        widgets.removeAll(widget);
    }
    bool winEvent(MSG* message, long* result);

private:
    QWidgetList widgets;
};

static bool resolveLibs()
{
    if (!pDwmIsCompositionEnabled) {
        QLibrary dwmLib(QString::fromAscii("dwmapi"));
        pDwmIsCompositionEnabled = (PtrDwmIsCompositionEnabled)dwmLib.resolve("DwmIsCompositionEnabled");
        pDwmExtendFrameIntoClientArea = (PtrDwmExtendFrameIntoClientArea)dwmLib.resolve("DwmExtendFrameIntoClientArea");
        pDwmEnableBlurBehindWindow = (PtrDwmEnableBlurBehindWindow)dwmLib.resolve("DwmEnableBlurBehindWindow");
        pDwmGetColorizationColor = (PtrDwmGetColorizationColor)dwmLib.resolve("DwmGetColorizationColor");
    }
    return pDwmIsCompositionEnabled != 0;
}

#endif

/*!
  * Chekcs and returns true if Windows version
  * currently running is Windows 7
  *
  * This function is useful when you are using
  * Windows7 new TaskBar API
  *
  */
bool QtWin::isRunningWindows7()
{
#ifdef Q_WS_WIN
    return QSysInfo::windowsVersion() == QSysInfo::WV_WINDOWS7;
#else
    return false;
#endif
}

/*!
  * Checks and returns true if Windows DWM composition
  * is currently enabled on the system.
  *
  * To get live notification on the availability of
  * this feature, you will currently have to
  * reimplement winEvent() on your widget and listen
  * for the WM_DWMCOMPOSITIONCHANGED event to occur.
  *
  */
bool QtWin::isCompositionEnabled()
{
#ifdef Q_WS_WIN
    if (resolveLibs()) {
        HRESULT hr = S_OK;
        BOOL isEnabled = false;
        hr = pDwmIsCompositionEnabled(&isEnabled);
        if (SUCCEEDED(hr)) {
            return isEnabled;
        }
    }
#endif
    return false;
}

/*!
  * Enables Blur behind on a Widget.
  *
  * \a enable tells if the blur should be enabled or not
  */
bool QtWin::enableBlurBehindWindow(QWidget* widget, bool enable)
{
    Q_UNUSED(enable);
    Q_UNUSED(widget);
    Q_ASSERT(widget);
    bool result = false;
#ifdef Q_WS_WIN
    if (resolveLibs()) {
        DWM_BLURBEHIND bb = {0};
        HRESULT hr = S_OK;
        bb.fEnable = enable;
        bb.dwFlags = DWM_BB_ENABLE;
        bb.hRgnBlur = NULL;
        widget->setAttribute(Qt::WA_TranslucentBackground, enable);
        widget->setAttribute(Qt::WA_NoSystemBackground, enable);
        hr = pDwmEnableBlurBehindWindow(widget->winId(), &bb);
        if (SUCCEEDED(hr)) {
            result = true;
            windowNotifier()->addWidget(widget);
        }
    }
#endif
    return result;
}

/*!
  * ExtendFrameIntoClientArea.
  *
  * This controls the rendering of the frame inside the window.
  * Note that passing margins of -1 (the default value) will completely
  * remove the frame from the window.
  *
  * \note you should not call enableBlurBehindWindow before calling
  *       this functions
  *
  * \a enable tells if the blur should be enabled or not
  */
bool QtWin::extendFrameIntoClientArea(QWidget* widget, int left, int top, int right, int bottom)
{

    Q_ASSERT(widget);
    Q_UNUSED(left);
    Q_UNUSED(top);
    Q_UNUSED(right);
    Q_UNUSED(bottom);
    Q_UNUSED(widget);

    bool result = false;
#ifdef Q_WS_WIN
    if (resolveLibs()) {
        QLibrary dwmLib(QString::fromAscii("dwmapi"));
        HRESULT hr = S_OK;
        MARGINS m = {left, top, right, bottom};
        hr = pDwmExtendFrameIntoClientArea(widget->winId(), &m);
        if (SUCCEEDED(hr)) {
            result = true;
            windowNotifier()->addWidget(widget);
        }
        widget->setAttribute(Qt::WA_TranslucentBackground, result);
    }
#endif
    return result;
}

/*!
  * Returns the current colorizationColor for the window.
  *
  * \a enable tells if the blur should be enabled or not
  */
QColor QtWin::colorizationColor()
{
    QColor resultColor = QApplication::palette().window().color();

#ifdef Q_WS_WIN
    if (resolveLibs()) {
        DWORD color = 0;
        BOOL opaque = FALSE;
        QLibrary dwmLib(QString::fromAscii("dwmapi"));
        HRESULT hr = S_OK;
        hr = pDwmGetColorizationColor(&color, &opaque);
        if (SUCCEEDED(hr)) {
            resultColor = QColor(color);
        }
    }
#endif
    return resultColor;
}

#ifdef Q_WS_WIN
WindowNotifier* QtWin::windowNotifier()
{
    static WindowNotifier* windowNotifierInstance = 0;
    if (!windowNotifierInstance) {
        windowNotifierInstance = new WindowNotifier;
    }
    return windowNotifierInstance;
}


/* Notify all enabled windows that the DWM state changed */
bool WindowNotifier::winEvent(MSG* message, long* result)
{
    if (message && message->message == WM_DWMCOMPOSITIONCHANGED) {
        bool compositionEnabled = QtWin::isCompositionEnabled();
        foreach(QWidget * widget, widgets) {
            if (widget) {
                widget->setAttribute(Qt::WA_NoSystemBackground, compositionEnabled);
                widget->update();
            }
        }
    }
    return QWidget::winEvent(message, result);
}

#ifdef W7API
IShellLink* QtWin::CreateShellLink(const QString &title, const QString &description,
                                   const QString &app_path, const QString &app_args,
                                   const QString &icon_path, int app_index)
{

    const wchar_t* _title = reinterpret_cast<const wchar_t*>(title.utf16());
    const wchar_t* _description = reinterpret_cast<const wchar_t*>(description.utf16());
    const wchar_t* _app_path = reinterpret_cast<const wchar_t*>(app_path.utf16());
    const wchar_t* _icon_path = reinterpret_cast<const wchar_t*>(icon_path.utf16());
    const wchar_t* _app_args = reinterpret_cast<const wchar_t*>(app_args.utf16());

    IShellLink* shell_link = NULL;
    IPropertyStore* prop_store = NULL;
    bool is_not_separator = (app_path.length() > 0);

    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink,
                                  reinterpret_cast<void**>(&(shell_link)));
    if (SUCCEEDED(hr)) {
        if (is_not_separator) {
            shell_link->SetPath(_app_path);
            shell_link->SetArguments(_app_args);
            shell_link->SetIconLocation(_icon_path, app_index);
            shell_link->SetDescription(_description);
        }

        hr = shell_link->QueryInterface(IID_IPropertyStore, reinterpret_cast<void**>(&(prop_store)));

        if (SUCCEEDED(hr)) {
            PROPVARIANT pv;

            if (is_not_separator) {
                hr = InitPropVariantFromString(_title, &pv);
                if (SUCCEEDED(hr)) {
                    hr = prop_store->SetValue(PKEY_Title, pv);
                }
            }
            else {
                hr = InitPropVariantFromBoolean(TRUE, &pv);

                if (SUCCEEDED(hr)) {
                    hr = prop_store->SetValue(PKEY_AppUserModel_IsDestListSeparator, pv);
                }
            }

            //Save the changes we made to the property store
            prop_store->Commit();
            prop_store->Release();

            PropVariantClear(&pv);
        }
    }
    return shell_link;
}

void QtWin::populateFrequentSites(IObjectCollection* collection, const QString &appPath)
{
    HistoryModel* historyModel = mApp->history();
    QList<HistoryEntry> mostList = historyModel->mostVisited(6);
    foreach(const HistoryEntry & entry, mostList)
    collection->AddObject(CreateShellLink(entry.title, entry.url.toString(), appPath, QString(" " + entry.url.toEncoded()), appPath, 1));

    collection->AddObject(CreateShellLink("", "", "", "", "", 0)); //Spacer
}

void QtWin::AddTasksToList(ICustomDestinationList* destinationList)
{
    IObjectArray* object_array;
    IObjectCollection* obj_collection;

    CoCreateInstance(CLSID_EnumerableObjectCollection, NULL,
                     CLSCTX_INPROC, IID_IObjectCollection, reinterpret_cast<void**>(&(obj_collection)));

    obj_collection->QueryInterface(IID_IObjectArray, reinterpret_cast<void**>(&(object_array)));

    QString icons_source = qApp->applicationFilePath();
    QString app_path = qApp->applicationFilePath();

    populateFrequentSites(obj_collection, icons_source);

    obj_collection->AddObject(CreateShellLink(tr("Open new tab"), tr("Opens a new tab if browser is running"),
                              app_path, "--new-tab",
                              icons_source, 0));

    obj_collection->AddObject(CreateShellLink(tr("Open new window"), tr("Opens a new window if browser is running"),
                              app_path, "--new-window",
                              icons_source, 0));

    obj_collection->AddObject(CreateShellLink(tr("Open download manager"), tr("Opens a download manager if browser is running"),
                              app_path, "--download-manager",
                              icons_source, 0));

    destinationList->AddUserTasks(object_array);

    object_array->Release();
    obj_collection->Release();
}
#endif //W7API
#endif //Q_WS_WIN

void QtWin::setupJumpList()
{
#ifdef W7API
    if (!isRunningWindows7()) {
        return;
    }

    UINT max_count = 0;
    IObjectArray* objectArray;
    ICustomDestinationList* destinationList;

    //create the custom jump list object
    CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_ICustomDestinationList,
                     reinterpret_cast<void**>(&(destinationList)));

    //initialize list
    destinationList->BeginList(&max_count, IID_IObjectArray, reinterpret_cast<void**>(&(objectArray)));
    AddTasksToList(destinationList);

    //commit list
    destinationList->CommitList();
    objectArray->Release();
    destinationList->Release();
#endif
}
