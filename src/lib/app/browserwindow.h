/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef QUPZILLA_H
#define QUPZILLA_H

#include <QMainWindow>
#include <QPointer>
#include <QUrl>

#include "restoremanager.h"
#include "qzcommon.h"

class QLabel;
class QVBoxLayout;
class QSplitter;
class QWebFrame;
class QTimer;

class Menu;
class MainMenu;
class TabWidget;
class TabbedWebView;
class LineEdit;
class SearchToolBar;
class HistoryMenu;
class BookmarksMenu;
class BookmarksToolbar;
class AutoFill;
class MainApplication;
class WebTab;
class AdBlockIcon;
class SideBar;
class SideBarManager;
class ProgressBar;
class StatusBarMessage;
class NavigationBar;
class NavigationContainer;
class ClickableLabel;
class LocationBar;

class QUPZILLA_EXPORT BrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const QString WEBKITVERSION;

    explicit BrowserWindow(Qz::BrowserWindowType type, const QUrl &url = QUrl());
    ~BrowserWindow();

    void setStartTab(WebTab* tab);

    void restoreWindowState(const RestoreManager::WindowData &d);
    void saveSideBarWidth();

    bool fullScreenNavigationVisible() const;
    void showNavigationWithFullScreen();
    void hideNavigationWithFullScreen();

    void currentTabChanged();
    void updateLoadingActions();

    void addBookmark(const QUrl &url, const QString &title);
    void addDeleteOnCloseWidget(QWidget* widget);

    void createToolbarsMenu(QMenu* menu);
    void createSidebarsMenu(QMenu* menu);
    void createEncodingMenu(QMenu* menu);

    void removeActions(const QList<QAction*> &actions);

    SideBar* addSideBar();

    QByteArray saveState(int version = 0) const;
    bool restoreState(const QByteArray &state, int version = 0);

    TabbedWebView* weView() const;
    TabbedWebView* weView(int index) const;

    Qz::BrowserWindowType windowType() const;
    LocationBar* locationBar() const;
    TabWidget* tabWidget() const;
    BookmarksToolbar* bookmarksToolbar() const;
    StatusBarMessage* statusBarMessage() const;
    NavigationBar* navigationBar() const;
    SideBarManager* sideBarManager() const;
    QLabel* ipLabel() const;
    AdBlockIcon* adBlockIcon() const;
    QMenu* superMenu() const;

    QUrl homepageUrl() const;

    bool isTransparentBackgroundAllowed() const;

    QAction* action(const QString &name) const;

signals:
    void startingCompleted();

public slots:
    void goHome();
    void goHomeInNewTab();
    void goBack();
    void goForward();

    void reload();
    void reloadBypassCache();

    void setWindowTitle(const QString &t);

    void showWebInspector();
    void toggleWebInspector();
    void showHistoryManager();

    void toggleShowMenubar();
    void toggleShowStatusBar();
    void toggleShowBookmarksToolbar();
    void toggleShowNavigationToolbar();
    void toggleTabsOnTop(bool enable);

    void toggleCaretBrowsing();
    void toggleFullScreen();
    void toggleOfflineMode();

    void loadActionUrl(QObject* obj = 0);
    void loadActionUrlInNewTab(QObject* obj = 0);

    void bookmarkPage();
    void bookmarkAllTabs();
    void loadAddress(const QUrl &url);
    void showSource(QWebFrame* frame = 0, const QString &selectedHtml = QString());
    void printPage(QWebFrame* frame = 0);

private slots:
    void addTab();
    void openLocation();
    void openFile();
    void closeWindow();
    void closeTab();
    void loadSettings();
    void postLaunch();

    void savePageScreen();
    void refreshHistory();
    void webSearch();
    void searchOnPage();
    void changeEncoding();

    bool quitApp();
    void hideNavigationSlot();

private:
    bool event(QEvent* event);
    void resizeEvent(QResizeEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void closeEvent(QCloseEvent* event);

    SearchToolBar* searchToolBar() const;

    void setupUi();
    void setupMenu();

    QAction *createEncodingAction(const QString &codecName, const QString &activeCodecName,
                                  QMenu *menu);
    void createEncodingSubMenu(const QString &name, QStringList &codecNames, QMenu *menu);

    QUrl m_startUrl;
    QUrl m_homepage;
    Qz::BrowserWindowType m_windowType;
    WebTab* m_startTab;

    QVBoxLayout* m_mainLayout;
    QSplitter* m_mainSplitter;

    AdBlockIcon* m_adblockIcon;

    TabWidget* m_tabWidget;
    QPointer<SideBar> m_sideBar;
    SideBarManager* m_sideBarManager;
    StatusBarMessage* m_statusBarMessage;

    NavigationContainer* m_navigationContainer;
    NavigationBar* m_navigationToolbar;
    BookmarksToolbar* m_bookmarksToolbar;

    ProgressBar* m_progressBar;
    QLabel* m_ipLabel;

    QMenu* m_superMenu;
    MainMenu* m_mainMenu;

    int m_sideBarWidth;
    int m_webViewWidth;

    bool m_useTransparentBackground;

    // Shortcuts
    bool m_useTabNumberShortcuts;
    bool m_useSpeedDialNumberShortcuts;
    bool m_useSingleKeyShortcuts;

    // Remember visibility of menubar and statusbar after entering Fullscreen
    bool m_menuBarVisible;
    bool m_statusBarVisible;
    Qt::WindowStates m_windowStates;
    QTimer* m_hideNavigationTimer;

    QList<QPointer<QWidget> > m_deleteOnCloseWidgets;

#ifdef QZ_WS_X11
private:
    int getCurrentVirtualDesktop() const;
    void moveToVirtualDesktop(int desktopId);
#endif

#ifdef Q_OS_WIN
private slots:
    void applyBlurToMainWindow(bool force = false);

private:
#if (QT_VERSION < 0x050000)
    bool winEvent(MSG* message, long* result);
#else
    bool nativeEvent(const QByteArray &eventType, void* _message, long* result);
#endif

    void paintEvent(QPaintEvent* event);
    bool eventFilter(QObject* object, QEvent* event);
#endif

};

#endif // QUPZILLA_H
