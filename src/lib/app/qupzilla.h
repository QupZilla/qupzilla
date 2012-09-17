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
#include "qwebkitversion.h"

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
class Action;

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
    inline Menu* menuHelp() { return m_menuHelp; }
    inline QAction* actionRestoreTab() { return m_actionRestoreTab; }
    inline Action* actionReload() { return m_actionReload; }
    inline QMenu* superMenu() { return m_superMenu; }

    inline bool isClosing() { return m_isClosing; }
    inline QUrl homepageUrl() { return m_homepage; }
#ifdef MENUBAR_USE_STATIC_ACTIONS
    void actionsConnectionManager(const QString &actionName, bool checked = false,
                                  QObject* senderObject = 0, bool isNewWindow = false);
    void menusConnectionManager(const QString &menuName, bool aboutToShow = true);
    static void resetMenuActionsState();
#endif

signals:
    void startingCompleted();
    void message(Qz::AppMessageType mes, bool state);
    void setWebViewMouseTracking(bool state);

public slots:
    void setWindowTitle(const QString &t);

    void showWebInspector(bool toggle = true);
    void showBookmarksToolbar();
    void loadActionUrl(QObject* senderObject = 0);
    void loadActionUrlInNewTab(QObject* senderObject = 0);
    void loadActionUrlInNewNotSelectedTab(QObject* senderObject = 0);
    void loadFolderBookmarks(Menu* menu);

    void bookmarkPage();
    void loadAddress(const QUrl &url);
    void showSource(QWebFrame* frame = 0, const QString &selectedHtml = QString());
    void printPage(QWebFrame* frame = 0);
    void showPageInfo();
    void receiveMessage(Qz::AppMessageType mes, bool state);

    //static
    static void openFile();
    static void showBookmarkImport();
    static void aboutQupZilla();
    static void showPreferences();

private slots:
    void postLaunch();
    void goNext();
    void goBack();
    void goHome();
    void goHomeInNewTab();
    void stop();
    void reload();
    void reloadByPassCache();
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

    void refreshHistory();
    void bookmarkAllTabs();
    void newWindow();

    void openLocation();
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
    void addMenusToMenuContainers();

    void disconnectObjects();

#ifdef Q_OS_WIN
    bool winEvent(MSG* message, long* result);
    bool eventFilter(QObject* object, QEvent* event);
#endif

#ifdef Q_WS_X11
    int getCurrentVirtualDesktop() const;
    void moveToVirtualDesktop(int desktopId);
#endif

    static bool doConnect(const QObject *sender, const char *signal,
                     const QObject *receiver, const char *member);
    void initializeOtherActions();
#ifdef MENUBAR_USE_STATIC_ACTIONS
    void createMenuConnections();
#endif

    bool m_historyMenuChanged;
    bool m_isClosing;
    bool m_isStarting;
    QUrl m_startingUrl;
    QUrl m_homepage;
    Qz::BrowserWindow m_startBehaviour;

    QVBoxLayout* m_mainLayout;
    QSplitter* m_mainSplitter;
    QMenu* m_superMenu;

#ifndef MENUBAR_USE_STATIC_ACTIONS
    Menu* m_menuFile;
    Menu* m_menuEdit;
    Menu* m_menuTools;
    Menu* m_menuHelp;
    Menu* m_menuView;
    Menu* m_menuBookmarks;
    Menu* m_menuHistory;
    QMenu* m_menuClosedTabs;
    Menu* m_menuHistoryRecent;
    Menu* m_menuHistoryMost;
    QMenu* m_menuEncoding;
    Menu* toolbarsMenu;
    Menu* sidebarsMenu;

    // we sure this is not mac
    Action* m_actionShowMenubar;
    // actions
    Action* m_actionAbout;
    Action* m_actionPreferences;
    Action* m_actionQuit;
    
    QAction* m_actionCloseWindow;
    Action* m_actionShowToolbar;
    Action* m_actionShowBookmarksToolbar;
    Action* m_actionShowStatusbar;
    
    Action* m_actionShowFullScreen;
    
    Action* m_actionStop;
    Action* m_actionReload;
    Action* m_actionPrivateBrowsing;
    
    bool m_bookmarksMenuChanged;
    QAction* m_menuBookmarksAction;
#else
    static bool m_menuBarCreated;

    static Menu* m_menuFile;
    static Menu* m_menuEdit;
    static Menu* m_menuTools;
    static Menu* m_menuHelp;
    static Menu* m_menuView;
    static Menu* m_menuBookmarks;
    static Menu* m_menuHistory;
    static QMenu* m_menuClosedTabs;
    static Menu* m_menuHistoryRecent;
    static Menu* m_menuHistoryMost;
    static QMenu* m_menuEncoding;
    static Menu* toolbarsMenu;
    static Menu* sidebarsMenu;

    // if another OS other than mac use static actions
#ifndef Q_OS_MAC
    static Action* m_actionShowMenubar;
#endif
    //actions
    static Action* m_actionAbout;
    static Action* m_actionPreferences;
    static Action* m_actionQuit;

    static QAction* m_actionCloseWindow;
    static Action* m_actionShowToolbar;
    static Action* m_actionShowBookmarksToolbar;
    static Action* m_actionShowStatusbar;

    static Action* m_actionShowFullScreen;

    static Action* m_actionStop;
    static Action* m_actionReload;
    static Action* m_actionPrivateBrowsing;
    
    static bool m_bookmarksMenuChanged;
    static QAction* m_menuBookmarksAction;
#endif

#ifdef Q_OS_MAC
    static QMenuBar* m_macMenuBar;
#endif

    QAction* m_actionShowBookmarksSideBar;
    QAction* m_actionShowHistorySideBar;
    QAction* m_actionShowRssSideBar;
    QAction* m_actionRestoreTab;

    QLabel* m_privateBrowsing;
    AdBlockIcon* m_adblockIcon;
    QWeakPointer<WebInspectorDockWidget> m_webInspectorDock;

    BookmarksToolbar* m_bookmarksToolbar;
    TabWidget* m_tabWidget;
    QWeakPointer<SideBar> m_sideBar;
    SideBarManager* m_sideBarManager;
    StatusBarMessage* m_statusBarMessage;
    NavigationBar* m_navigationBar;

    ProgressBar* m_progressBar;
    QLabel* m_ipLabel;

    QString m_lastWindowTitle;

    int m_sideBarWidth;
    int m_webViewWidth;
    bool m_usingTransparentBackground;

    //Used for F11 FullScreen remember visibility
    //of menubar and statusbar
    bool m_menuBarVisible;
    bool m_statusBarVisible;
    bool m_navigationVisible;
    bool m_bookmarksToolBarVisible;

    QList<QWeakPointer<QWidget> > m_deleteOnCloseWidgets;
};

#endif // QUPZILLA_H
