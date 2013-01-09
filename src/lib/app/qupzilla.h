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
#ifndef QUPZILLA_H
#define QUPZILLA_H

#include <QMainWindow>
#include <QUrl>

#include "restoremanager.h"
#include "qz_namespace.h"

class QMenuBar;
class QLabel;
class QVBoxLayout;
class QSplitter;
class QWebFrame;

class Menu;
class TabWidget;
class TabbedWebView;
class LineEdit;
class SearchToolBar;
class BookmarksToolbar;
class AutoFillModel;
class MainApplication;
class WebTab;
class AdBlockIcon;
class SideBar;
class SideBarManager;
class ProgressBar;
class StatusBarMessage;
class NavigationBar;
class ClickableLabel;
class WebInspectorDockWidget;
class LocationBar;

class QT_QUPZILLA_EXPORT QupZilla : public QMainWindow
{
    Q_OBJECT

public:
    static const QString VERSION;
    static const QString BUILDTIME;
    static const QString AUTHOR;
    static const QString COPYRIGHT;
    static const QString WWWADDRESS;
    static const QString WIKIADDRESS;
    static const QString WEBKITVERSION;

    explicit QupZilla(Qz::BrowserWindow type, QUrl startUrl = QUrl());
    ~QupZilla();

    void loadSettings();
    void showNavigationWithFullscreen();
    void saveSideBarWidth();

    void currentTabChanged();
    void updateLoadingActions();

    void addBookmark(const QUrl &url, const QString &title, const QIcon &icon);
    void addDeleteOnCloseWidget(QWidget* widget);

    void restoreWindowState(const RestoreManager::WindowData &d);

    SideBar* addSideBar();
    virtual QMenuBar* menuBar() const;

    QByteArray saveState(int version = 0) const;
    bool restoreState(const QByteArray &state, int version = 0);

    TabbedWebView* weView() const;
    TabbedWebView* weView(int index) const;
    LocationBar* locationBar() const;

    inline TabWidget* tabWidget() { return m_tabWidget; }
    inline BookmarksToolbar* bookmarksToolbar() { return m_bookmarksToolbar; }
    inline StatusBarMessage* statusBarMessage() { return m_statusBarMessage; }
    inline NavigationBar* navigationBar() { return m_navigationBar; }
    inline SideBarManager* sideBarManager() { return m_sideBarManager; }
    inline ProgressBar* progressBar() { return m_progressBar; }
    inline QLabel* ipLabel() { return m_ipLabel; }
    inline AdBlockIcon* adBlockIcon() { return m_adblockIcon; }
    inline QMenu* menuHelp() { return m_menuHelp; }
    inline QAction* actionRestoreTab() { return m_actionRestoreTab; }
    inline QAction* actionReload() { return m_actionReload; }
    inline QMenu* superMenu() { return m_superMenu; }

    inline bool isClosing() { return m_isClosing; }
    inline QUrl homepageUrl() { return m_homepage; }

signals:
    void startingCompleted();
    void message(Qz::AppMessageType mes, bool state);
    void setWebViewMouseTracking(bool state);

public slots:
    void setWindowTitle(const QString &t);

    void showWebInspector(bool toggle = true);
    void showBookmarksToolbar();
    void loadActionUrl();
    void loadActionUrlInNewTab();
    void loadActionUrlInNewNotSelectedTab();
    void loadFolderBookmarks(Menu* menu);

    void bookmarkPage();
    void loadAddress(const QUrl &url);
    void showSource(QWebFrame* frame = 0, const QString &selectedHtml = QString());
    void printPage(QWebFrame* frame = 0);
    void showPageInfo();
    void receiveMessage(Qz::AppMessageType mes, bool state);

private slots:
    void postLaunch();
    void goNext();
    void goBack();
    void goHome();
    void goHomeInNewTab();
    void stop();
    void reload();
    void reloadByPassCache();
    void aboutQupZilla();
    void addTab();
    void savePageScreen();

    void aboutToShowFileMenu();
    void aboutToHideFileMenu();
    void aboutToShowHistoryMenu();
    void aboutToHideHistoryMenu();
    void aboutToShowClosedTabsMenu();
    void aboutToShowBookmarksMenu();
    void aboutToShowViewMenu();
    void aboutToShowEditMenu();
    void aboutToHideEditMenu();
    void aboutToShowEncodingMenu();

    void searchOnPage();
    void showCookieManager();
    void showHistoryManager();
    void showBookmarksManager();
    void showRSSManager();
    void showDownloadManager();
    void showMenubar();
    void showNavigationToolbar();
    void showStatusbar();
    void showClearPrivateData();
    void aboutToShowHistoryRecentMenu();
    void aboutToShowHistoryMostMenu();
    void showPreferences();
    void showBookmarkImport();

    void refreshHistory();
    void bookmarkAllTabs();
    void newWindow();

    void openLocation();
    void openFile();
    void savePage();
    void sendLink();
    void webSearch();

    // Edit menu actions
    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editSelectAll();

    void zoomIn();
    void zoomOut();
    void zoomReset();
    void fullScreen(bool make);
    void changeEncoding();

    void triggerCaretBrowsing();

    void closeWindow();
    bool quitApp();
#ifdef Q_OS_WIN
    void applyBlurToMainWindow(bool force = false);
#endif

private:
    void resizeEvent(QResizeEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void closeEvent(QCloseEvent* event);

    void setupUi();
    void setupMenu();

    void disconnectObjects();

#ifdef Q_OS_WIN
    bool winEvent(MSG* message, long* result);
    bool eventFilter(QObject* object, QEvent* event);
#endif

#ifdef QZ_WS_X11
    int getCurrentVirtualDesktop() const;
    void moveToVirtualDesktop(int desktopId);
#endif

    bool m_historyMenuChanged;
    bool m_bookmarksMenuChanged;
    bool m_isClosing;
    bool m_isStarting;
    QUrl m_startingUrl;
    QUrl m_homepage;
    Qz::BrowserWindow m_startBehaviour;

    QVBoxLayout* m_mainLayout;
    QSplitter* m_mainSplitter;
    QMenu* m_superMenu;
    QMenu* m_menuFile;
    QMenu* m_menuEdit;
    QMenu* m_menuTools;
    QMenu* m_menuHelp;
    QMenu* m_menuView;
    Menu* m_menuBookmarks;
    Menu* m_menuHistory;
    QMenu* m_menuClosedTabs;
    Menu* m_menuHistoryRecent;
    Menu* m_menuHistoryMost;
    QMenu* m_menuEncoding;
    QAction* m_menuBookmarksAction;
#ifdef Q_OS_MAC
    QMenuBar* m_macMenuBar;
#endif

    QAction* m_actionAbout;
    QAction* m_actionPreferences;
    QAction* m_actionQuit;

    QAction* m_actionCloseWindow;
    QAction* m_actionShowToolbar;
    QAction* m_actionShowBookmarksToolbar;
    QAction* m_actionShowStatusbar;
#ifndef Q_OS_MAC
    QAction* m_actionShowMenubar;
#endif
    QAction* m_actionShowFullScreen;
    QAction* m_actionShowBookmarksSideBar;
    QAction* m_actionShowHistorySideBar;
    QAction* m_actionShowRssSideBar;
    QAction* m_actionPrivateBrowsing;
    QAction* m_actionStop;
    QAction* m_actionReload;
    QAction* m_actionCaretBrowsing;
    QAction* m_actionRestoreTab;

    QLabel* m_privateBrowsing;
    AdBlockIcon* m_adblockIcon;
    QPointer<WebInspectorDockWidget> m_webInspectorDock;

    BookmarksToolbar* m_bookmarksToolbar;
    TabWidget* m_tabWidget;
    QPointer<SideBar> m_sideBar;
    SideBarManager* m_sideBarManager;
    StatusBarMessage* m_statusBarMessage;
    NavigationBar* m_navigationBar;

    ProgressBar* m_progressBar;
    QLabel* m_ipLabel;

    QString m_lastWindowTitle;

    int m_sideBarWidth;
    int m_webViewWidth;
    bool m_usingTransparentBackground;

    // Shortcuts
    bool m_useTabNumberShortcuts;
    bool m_useSpeedDialNumberShortcuts;

    // Used for F11 FullScreen remember visibility
    // of menubar and statusbar
    bool m_menuBarVisible;
    bool m_statusBarVisible;
    bool m_navigationVisible;
    bool m_bookmarksToolBarVisible;

    QList<QPointer<QWidget> > m_deleteOnCloseWidgets;
};

#endif // QUPZILLA_H
