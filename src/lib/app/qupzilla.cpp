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
#include "qupzilla.h"
#include "tabwidget.h"
#include "tabbar.h"
#include "webpage.h"
#include "tabbedwebview.h"
#include "lineedit.h"
#include "history.h"
#include "locationbar.h"
#include "searchtoolbar.h"
#include "websearchbar.h"
#include "downloadmanager.h"
#include "cookiejar.h"
#include "cookiemanager.h"
#include "bookmarksmanager.h"
#include "bookmarkstoolbar.h"
#include "clearprivatedata.h"
#include "sourceviewer.h"
#include "siteinfo.h"
#include "preferences.h"
#include "networkmanager.h"
#include "autofillmodel.h"
#include "networkmanagerproxy.h"
#include "rssmanager.h"
#include "mainapplication.h"
#include "aboutdialog.h"
#include "pluginproxy.h"
#include "checkboxdialog.h"
#include "adblockmanager.h"
#include "clickablelabel.h"
#include "docktitlebarwidget.h"
#include "sidebar.h"
#include "iconprovider.h"
#include "progressbar.h"
#include "adblockicon.h"
#include "closedtabsmanager.h"
#include "statusbarmessage.h"
#include "browsinglibrary.h"
#include "navigationbar.h"
#include "pagescreen.h"
#include "webinspectordockwidget.h"
#include "bookmarksimportdialog.h"
#include "globalfunctions.h"
#include "enhancedmenu.h"
#include "settings.h"
#include "webtab.h"
#include "speeddial.h"
#include "qtwin.h"

#include <QKeyEvent>
#include <QSplitter>
#include <QStatusBar>
#include <QMenuBar>
#include <QTimer>
#include <QShortcut>
#include <QStackedWidget>
#include <QSqlQuery>
#include <QTextCodec>
#include <QFileDialog>
#include <QNetworkRequest>
#include <QDesktopServices>
#include <QPrintPreviewDialog>
#include <QWebFrame>
#include <QWebHistory>
#include <QMessageBox>

#ifdef Q_WS_X11
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

const QString QupZilla::VERSION = "1.3.5";
const QString QupZilla::BUILDTIME =  __DATE__" "__TIME__;
const QString QupZilla::AUTHOR = "David Rosca";
const QString QupZilla::COPYRIGHT = "2010-2012";
const QString QupZilla::WWWADDRESS = "http://www.qupzilla.com";
const QString QupZilla::WIKIADDRESS = "https://github.com/QupZilla/qupzilla/wiki";
const QString QupZilla::WEBKITVERSION = qWebKitVersion();

#ifdef Q_OS_MAC
    QMenuBar* QupZilla::m_macMenuBar = 0;
#endif

#ifdef MENUBAR_USE_STATIC_ACTIONS
    bool QupZilla::m_menuBarCreated = false;

    Menu* QupZilla::m_menuFile = 0;
    Menu* QupZilla::m_menuEdit = 0;
    Menu* QupZilla::m_menuTools = 0;
    Menu* QupZilla::m_menuHelp = 0;
    Menu* QupZilla::m_menuView = 0;
    Menu* QupZilla::m_menuBookmarks = 0;
    Menu* QupZilla::m_menuHistory = 0;
    QMenu* QupZilla::m_menuClosedTabs = 0;
    Menu* QupZilla::m_menuHistoryRecent = 0;
    Menu* QupZilla::m_menuHistoryMost = 0;
    QMenu* QupZilla::m_menuEncoding = 0;
    Menu* QupZilla::toolbarsMenu = 0;
    Menu* QupZilla::sidebarsMenu = 0;

#ifndef Q_OS_MAC
    Action* QupZilla::m_actionShowMenubar = 0;
#endif
    Action* QupZilla::m_actionAbout = 0;
    Action* QupZilla::m_actionPreferences = 0;
    Action* QupZilla::m_actionQuit = 0;
    
    QAction* QupZilla::m_actionCloseWindow = 0;
    
    Action* QupZilla::m_actionShowToolbar = 0;
    Action* QupZilla::m_actionShowBookmarksToolbar = 0;
    Action* QupZilla::m_actionShowStatusbar = 0;
    
    Action* QupZilla::m_actionShowFullScreen = 0;
    
    Action* QupZilla::m_actionStop = 0;
    Action* QupZilla::m_actionReload = 0;
    Action* QupZilla::m_actionPrivateBrowsing = 0;

    bool QupZilla::m_bookmarksMenuChanged = true;
    QAction* QupZilla::m_menuBookmarksAction = 0;
#endif

QupZilla::QupZilla(Qz::BrowserWindow type, QUrl startUrl)
    : QMainWindow(0)
    , m_historyMenuChanged(true)
    , m_isClosing(false)
    , m_isStarting(false)
    , m_startingUrl(startUrl)
    , m_startBehaviour(type)
#ifndef MENUBAR_USE_STATIC_ACTIONS
    , m_bookmarksMenuChanged(true)
    , m_menuBookmarksAction(0)
    , m_actionPrivateBrowsing(0)
#endif
    , m_sideBarManager(new SideBarManager(this))
    , m_statusBarMessage(new StatusBarMessage(this))
    , m_usingTransparentBackground(false)
{
    setObjectName("mainwindow");
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("QupZilla"));

#ifdef Q_OS_MAC
    if (!m_macMenuBar) {
        m_macMenuBar = new QMenuBar();
    }
#endif

    if (mApp->isPrivateSession()) {
        setProperty("private", QVariant(true));
    }

    m_isStarting = true;

#ifndef Q_WS_X11
    setUpdatesEnabled(false);
#endif

    setupUi();
    setupMenu();

    QTimer::singleShot(0, this, SLOT(postLaunch()));
    connect(mApp, SIGNAL(message(Qz::AppMessageType, bool)), this, SLOT(receiveMessage(Qz::AppMessageType, bool)));
}

void QupZilla::postLaunch()
{
#ifdef Q_WS_X11
    setUpdatesEnabled(false);
#endif

    loadSettings();

    if (m_startBehaviour == Qz::BW_FirstAppWindow) {
        m_tabWidget->restorePinnedTabs();
    }

    Settings settings;
    int afterLaunch = settings.value("Web-URL-Settings/afterLaunch", 1).toInt();
    bool addTab = true;
    QUrl startUrl;

    switch (afterLaunch) {
    case 0:
        startUrl = QUrl();
        break;

    case 2:
        startUrl = QUrl("qupzilla:speeddial");
        break;

    case 1:
    case 3:
        startUrl = m_homepage;
        break;

    default:
        break;
    }

    switch (m_startBehaviour) {
    case Qz::BW_FirstAppWindow:
        if (mApp->isStartingAfterCrash()) {
            addTab = true;
            startUrl = QUrl("qupzilla:restore");
        }
        else if (afterLaunch == 3 && mApp->restoreManager()) {
            addTab = !mApp->restoreStateSlot(this, mApp->restoreManager()->restoreData());
        }
        break;

    case Qz::BW_NewWindow:
        addTab = true;
        break;

    case Qz::BW_OtherRestoredWindow:
        addTab = false;
        break;
    }

    show();

    if (!m_startingUrl.isEmpty()) {
        startUrl = QUrl::fromUserInput(m_startingUrl.toString());
        addTab = true;
    }

    if (addTab) {
        QNetworkRequest request(startUrl);
        request.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));

        m_tabWidget->addView(request, Qz::NT_CleanSelectedTabAtTheEnd);

        if (startUrl.isEmpty() || startUrl.toString() == QLatin1String("qupzilla:speeddial")) {
            locationBar()->setFocus();
        }
    }

    if (m_tabWidget->getTabBar()->normalTabsCount() <= 0 && m_startBehaviour != Qz::BW_OtherRestoredWindow) {
        //Something went really wrong .. add one tab
        QNetworkRequest request(m_homepage);
        request.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));

        m_tabWidget->addView(request, Qz::NT_SelectedTabAtTheEnd);
    }

    aboutToHideEditMenu();

#ifdef Q_OS_MAC
    // fill menus even if user don't call them
    aboutToShowBookmarksMenu();
    aboutToShowHistoryMostMenu();
    aboutToShowHistoryRecentMenu();
#endif

    mApp->plugins()->emitMainWindowCreated(this);
    emit startingCompleted();

    m_isStarting = false;
    QMainWindow::setWindowTitle(m_lastWindowTitle);

    setUpdatesEnabled(true);
}

void QupZilla::setupUi()
{
    int locationBarWidth;
    int websearchBarWidth;

    Settings settings;
    settings.beginGroup("Browser-View-Settings");
    if (settings.value("WindowMaximised", false).toBool()) {
        resize(800, 550);
        setWindowState(Qt::WindowMaximized);
    }
    else {
        if (!restoreGeometry(settings.value("WindowGeometry").toByteArray())) {
            setGeometry(QRect(20, 20, 800, 550));
        }

        if (m_startBehaviour == Qz::BW_NewWindow) {
            // Moving window +40 x,y to be visible that this is new window
            QPoint p = pos();
            p.setX(p.x() + 40);
            p.setY(p.y() + 40);

            move(p);
        }
    }

    locationBarWidth = settings.value("LocationBarWidth", 480).toInt();
    websearchBarWidth = settings.value("WebSearchBarWidth", 140).toInt();
    settings.endGroup();

    QWidget* widget = new QWidget(this);
    widget->setCursor(Qt::ArrowCursor);
    setCentralWidget(widget);

    m_mainLayout = new QVBoxLayout(widget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainSplitter = new QSplitter(this);
    m_mainSplitter->setObjectName("sidebar-splitter");
    m_mainSplitter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_tabWidget = new TabWidget(this);
    m_superMenu = new QMenu(this);
    m_navigationBar = new NavigationBar(this);
    m_navigationBar->setSplitterSizes(locationBarWidth, websearchBarWidth);
    m_bookmarksToolbar = new BookmarksToolbar(this);

    m_mainSplitter->addWidget(m_tabWidget);
    m_mainLayout->addWidget(m_navigationBar);
    m_mainLayout->addWidget(m_bookmarksToolbar);
    m_mainLayout->addWidget(m_mainSplitter);
    m_mainSplitter->setCollapsible(0, false);

    statusBar()->setObjectName("mainwindow-statusbar");
    statusBar()->setCursor(Qt::ArrowCursor);
    m_progressBar = new ProgressBar(statusBar());
    m_privateBrowsing = new QLabel(this);
    m_privateBrowsing->setPixmap(QPixmap(":/icons/locationbar/privatebrowsing.png"));
    m_privateBrowsing->setVisible(false);
    m_privateBrowsing->setToolTip(tr("Private Browsing Enabled"));
    m_adblockIcon = new AdBlockIcon(this);
    m_ipLabel = new QLabel(this);
    m_ipLabel->setObjectName("statusbar-ip-label");
    m_ipLabel->setToolTip(tr("IP Address of current page"));

    statusBar()->insertPermanentWidget(0, m_progressBar);
    statusBar()->insertPermanentWidget(1, m_ipLabel);
    statusBar()->insertPermanentWidget(2, m_privateBrowsing);
    statusBar()->insertPermanentWidget(3, m_adblockIcon);
}

void QupZilla::setupMenu()
{
    StaticActionManager::setGlobalReciver(mApp, SLOT(actionsConnectionManager(bool)));

    // Standard actions - needed on Mac to be placed correctly in "application" menu
    m_actionAbout = StaticActionManager::actionInstance("act-about-qupZilla", tr("&About QupZilla"), QIcon(":/icons/qupzilla.png"), true);
    m_actionAbout->setMenuRole(QAction::AboutRole);
    doConnect(m_actionAbout, SIGNAL(triggered()), this, SLOT(aboutQupZilla()));

    m_actionPreferences = StaticActionManager::actionInstance("act-preferences", tr("Pr&eferences"), QIcon(":/icons/faenza/settings.png"), false);
    m_actionPreferences->setMenuRole(QAction::PreferencesRole);
    m_actionPreferences->setShortcut(QKeySequence(QKeySequence::Preferences));
    doConnect(m_actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferences()));

    m_actionQuit = StaticActionManager::actionInstance("act-application-exit", tr("Quit"), QIcon::fromTheme("application-exit"), true);
    m_actionQuit->setMenuRole(QAction::QuitRole);
    m_actionQuit->setShortcut(QKeySequence(QKeySequence::Quit));
    doConnect(m_actionQuit, SIGNAL(triggered()), this, SLOT(quitApp()));

#ifdef MENUBAR_USE_STATIC_ACTIONS
   if (QupZilla::m_menuBarCreated) {
       if (mApp->windowCount() == 0) {
           StaticActionManager::enableStaticActions(true);
       }
#ifndef Q_OS_MAC
       addMenusToMenuContainers();
#endif
       initializeOtherActions();
       return;
   }
   QupZilla::m_menuBarCreated = true;
#endif

    /*************
     * File Menu *
     *************/
    m_menuFile = new Menu(tr("&File"), false);
    m_menuFile->addAction(QIcon::fromTheme("window-new"), tr("&New Window"), this, SLOT(newWindow()), "act-window-new", true)->setShortcut(QKeySequence("Ctrl+N"));
    m_menuFile->addAction(QIcon(":/icons/menu/popup.png"), tr("New Tab"), this, SLOT(addTab()), "act-new-tab", true)->setShortcut(QKeySequence("Ctrl+T"));
    m_menuFile->addAction(QIcon::fromTheme("document-open-remote"), tr("Open Location"), this, SLOT(openLocation()), "act-document-open-remote", true)->setShortcut(QKeySequence("Ctrl+L"));
    m_menuFile->addAction(QIcon::fromTheme("document-open"), tr("Open &File"), this, SLOT(openFile()), "act-document-open", true)->setShortcut(QKeySequence("Ctrl+O"));
    m_menuFile->addAction(tr("Close Tab"), m_tabWidget, SLOT(closeTab()), "act-close-tab")->setShortcut(QKeySequence("Ctrl+W"));
    m_actionCloseWindow = m_menuFile->addAction(QIcon::fromTheme("window-close"), tr("Close Window"), this, SLOT(closeWindow()), "act-window-close");
    m_actionCloseWindow->setShortcut(QKeySequence("Ctrl+Shift+W"));
    m_menuFile->addSeparator();
    m_menuFile->addAction(QIcon::fromTheme("document-save"), tr("&Save Page As..."), this, SLOT(savePage()), "act-document-save")->setShortcut(QKeySequence("Ctrl+S"));
    m_menuFile->addAction(tr("Save Page Screen"), this, SLOT(savePageScreen()), "act-save-page-screen");
    m_menuFile->addAction(QIcon::fromTheme("mail-message-new"), tr("Send Link..."), this, SLOT(sendLink()), "act-mail-message-new");
    m_menuFile->addAction(QIcon::fromTheme("document-print"), tr("&Print..."), this, SLOT(printPage()), "act-document-print")->setShortcut(QKeySequence("Ctrl+P"));
    m_menuFile->addSeparator();
    m_menuFile->addSeparator();
    m_menuFile->addAction(tr("Import bookmarks..."), this, SLOT(showBookmarkImport()), "act-import-bookmarks", true);
    m_menuFile->addAction(m_actionQuit);
#ifdef Q_OS_MAC // Add standard actions to File Menu (as it won't be ever cleared) and Mac menubar should move them to "application" menu
    m_menuFile->addAction(m_actionAbout);
    m_menuFile->addAction(m_actionPreferences);
#endif
    doConnect(m_menuFile, SIGNAL(aboutToShow()), this, SLOT(aboutToShowFileMenu()));
    doConnect(m_menuFile, SIGNAL(aboutToHide()), this, SLOT(aboutToHideFileMenu()));

    /*************
     * Edit Menu *
     *************/
    m_menuEdit = new Menu(tr("&Edit"), false);
    m_menuEdit->addAction(QIcon::fromTheme("edit-undo"), tr("&Undo"), this, SLOT(editUndo()), "act-edit-undo")->setShortcut(QKeySequence("Ctrl+Z"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-redo"), tr("&Redo"), this, SLOT(editRedo()), "act-edit-redo")->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-cut"), tr("&Cut"), this, SLOT(editCut()), "act-edit-cut")->setShortcut(QKeySequence("Ctrl+X"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-copy"), tr("C&opy"), this, SLOT(editCopy()), "act-edit-copy")->setShortcut(QKeySequence("Ctrl+C"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this, SLOT(editPaste()), "act-edit-paste")->setShortcut(QKeySequence("Ctrl+V"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-select-all"), tr("Select &All"), this, SLOT(editSelectAll()), "act-edit-select-all")->setShortcut(QKeySequence("Ctrl+A"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-find"), tr("&Find"), this, SLOT(searchOnPage()), "act-edit-find")->setShortcut(QKeySequence("Ctrl+F"));
    m_menuEdit->addSeparator();
#ifdef Q_WS_X11
    m_menuEdit->addAction(m_actionPreferences);
#endif
    doConnect(m_menuEdit, SIGNAL(aboutToShow()), this, SLOT(aboutToShowEditMenu()));
    doConnect(m_menuEdit, SIGNAL(aboutToHide()), this, SLOT(aboutToHideEditMenu()));

    /*************
     * View Menu *
     *************/
    m_menuView = new Menu(tr("&View"), false);
    m_actionShowToolbar = StaticActionManager::actionInstance("act-navigation-toolbar", tr("&Navigation Toolbar"), QIcon(), false, this);
    m_actionShowToolbar->setCheckable(true);
    doConnect(m_actionShowToolbar, SIGNAL(triggered(bool)), this, SLOT(showNavigationToolbar()));
    m_actionShowBookmarksToolbar = StaticActionManager::actionInstance("act-bookmarks-toolbar", tr("&Bookmarks Toolbar"), QIcon(), false, this);
    m_actionShowBookmarksToolbar->setCheckable(true);
    doConnect(m_actionShowBookmarksToolbar, SIGNAL(triggered(bool)), this, SLOT(showBookmarksToolbar()));
    m_actionShowStatusbar = StaticActionManager::actionInstance("act-status-bar", tr("Sta&tus Bar"), QIcon(), false, this);
    m_actionShowStatusbar->setCheckable(true);
    doConnect(m_actionShowStatusbar, SIGNAL(triggered(bool)), this, SLOT(showStatusbar()));
#ifndef Q_OS_MAC
    m_actionShowMenubar = StaticActionManager::actionInstance("act-menu-bar", tr("&Menu Bar"), QIcon(), false, this);
    m_actionShowMenubar->setCheckable(true);
    doConnect(m_actionShowMenubar, SIGNAL(triggered(bool)), this, SLOT(showMenubar()));
#endif
    m_actionShowFullScreen = StaticActionManager::actionInstance("act-fullscreen", tr("&Fullscreen"), QIcon(), false, this);
    m_actionShowFullScreen->setCheckable(true);
    m_actionShowFullScreen->setShortcut(QKeySequence("F11"));
    doConnect(m_actionShowFullScreen, SIGNAL(triggered(bool)), this, SLOT(fullScreen(bool)));
    m_actionStop = StaticActionManager::actionInstance("act-stop", tr("&Stop"), qIconProvider->standardIcon(QStyle::SP_BrowserStop), false, this);
    doConnect(m_actionStop, SIGNAL(triggered()), this, SLOT(stop()));
    m_actionStop->setShortcut(QKeySequence("Esc"));
    m_actionReload = StaticActionManager::actionInstance("act-reload", tr("&Reload"), qIconProvider->standardIcon(QStyle::SP_BrowserReload), false, this);
    doConnect(m_actionReload, SIGNAL(triggered()), this, SLOT(reload()));
    m_actionReload->setShortcut(QKeySequence("F5"));
    Action* actionEncoding = StaticActionManager::actionInstance("act-character-encoding", tr("Character &Encoding"), QIcon(), false, this);
#ifdef MENUBAR_USE_STATIC_ACTIONS
    m_menuEncoding = new QMenu();
#else
    m_menuEncoding = new QMenu(this);
#endif
    actionEncoding->setMenu(m_menuEncoding);
    doConnect(m_menuEncoding, SIGNAL(aboutToShow()), this, SLOT(aboutToShowEncodingMenu()));

    toolbarsMenu = new Menu(tr("Toolbars"), false);
#ifndef Q_OS_MAC
    toolbarsMenu->addAction(m_actionShowMenubar);
#endif
    toolbarsMenu->addAction(m_actionShowToolbar);
    toolbarsMenu->addAction(m_actionShowBookmarksToolbar);
    sidebarsMenu = new Menu(tr("Sidebars"), false);
    m_sideBarManager->setSideBarMenu(sidebarsMenu);

    m_menuView->addMenu(toolbarsMenu);
    m_menuView->addMenu(sidebarsMenu);
    m_menuView->addAction(m_actionShowStatusbar);
    m_menuView->addSeparator();
    m_menuView->addAction(m_actionStop);
    m_menuView->addAction(m_actionReload);
    m_menuView->addSeparator();
    m_menuView->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom &In"), this, SLOT(zoomIn()), "act-zoom-in")->setShortcut(QKeySequence("Ctrl++"));
    m_menuView->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom &Out"), this, SLOT(zoomOut()), "act-zoom-out")->setShortcut(QKeySequence("Ctrl+-"));
    m_menuView->addAction(QIcon::fromTheme("zoom-original"), tr("Reset"), this, SLOT(zoomReset()), "act-zoom-original")->setShortcut(QKeySequence("Ctrl+0"));
    m_menuView->addSeparator();
    m_menuView->addAction(actionEncoding);
    m_menuView->addSeparator();
    m_menuView->addAction(QIcon::fromTheme("text-html"), tr("&Page Source"), this, SLOT(showSource()), "act-text-html")->setShortcut(QKeySequence("Ctrl+U"));
    m_menuView->addAction(m_actionShowFullScreen, "m_actionShowFullScreen");
    doConnect(m_menuView, SIGNAL(aboutToShow()), this, SLOT(aboutToShowViewMenu()));

    /****************
     * History Menu *
     ****************/
    m_menuHistory = new Menu(tr("Hi&story"));
    m_menuHistory->addAction(qIconProvider->standardIcon(QStyle::SP_ArrowBack), tr("&Back"), this, SLOT(goBack()), "act-back")->setShortcut(QKeySequence("Ctrl+Left"));
    m_menuHistory->addAction(qIconProvider->standardIcon(QStyle::SP_ArrowForward), tr("&Forward"), this, SLOT(goNext()), "act-forward")->setShortcut(QKeySequence("Ctrl+Right"));
    m_menuHistory->addAction(qIconProvider->fromTheme("go-home"), tr("&Home"), this, SLOT(goHome()), "act-go-home", true)->setShortcut(QKeySequence("Alt+Home"));
    m_menuHistory->addAction(QIcon(":/icons/menu/history.png"), tr("Show &All History"), this, SLOT(showHistoryManager()), "act-show-all-history")->setShortcut(QKeySequence("Ctrl+Shift+H"));
    m_menuHistory->addSeparator();
    doConnect(m_menuHistory, SIGNAL(aboutToShow()), this, SLOT(aboutToShowHistoryMenu()));
    doConnect(m_menuHistory, SIGNAL(aboutToHide()), this, SLOT(aboutToHideHistoryMenu()));

    m_menuClosedTabs = new QMenu(tr("Closed Tabs"));
    doConnect(m_menuClosedTabs, SIGNAL(aboutToShow()), this, SLOT(aboutToShowClosedTabsMenu()));

    m_menuHistoryRecent = new Menu(tr("Recently Visited"), m_menuHistory);
    doConnect(m_menuHistoryRecent, SIGNAL(aboutToShow()), this, SLOT(aboutToShowHistoryRecentMenu()));

    m_menuHistoryMost = new Menu(tr("Most Visited"), m_menuHistory);
    doConnect(m_menuHistoryMost, SIGNAL(aboutToShow()), this, SLOT(aboutToShowHistoryMostMenu()));

    m_menuHistory->addMenu(m_menuHistoryRecent);
    m_menuHistory->addMenu(m_menuHistoryMost);
    m_menuHistory->addMenu(m_menuClosedTabs);

    /******************
     * Bookmarks Menu *
     ******************/
    m_menuBookmarks = new Menu(tr("&Bookmarks"));
    m_menuBookmarks->addAction(tr("Bookmark &This Page"), this, SLOT(bookmarkPage()), "act-bookmark-this-page")->setShortcut(QKeySequence("Ctrl+D"));
    m_menuBookmarks->addAction(tr("Bookmark &All Tabs"), this, SLOT(bookmarkAllTabs()), "act-bookmark-all-tabs");
    m_menuBookmarks->addAction(qIconProvider->fromTheme("user-bookmarks"), tr("Organize &Bookmarks"), this, SLOT(showBookmarksManager()), "act-user-bookmarks")->setShortcut(QKeySequence("Ctrl+Shift+O"));
    m_menuBookmarks->addSeparator();

    doConnect(m_menuBookmarks, SIGNAL(aboutToShow()), this, SLOT(aboutToShowBookmarksMenu()));
    doConnect(m_menuBookmarks, SIGNAL(menuMiddleClicked(Menu*)), this, SLOT(loadFolderBookmarks(Menu*)));

    /**************
     * Tools Menu *
     **************/
    m_menuTools = new Menu(tr("&Tools"), false);
    m_menuTools->addAction(tr("&Web Search"), this, SLOT(webSearch()), "act-web-search")->setShortcut(QKeySequence("Ctrl+K"));
    m_menuTools->addAction(QIcon::fromTheme("dialog-information"), tr("Page &Info"), this, SLOT(showPageInfo()), "act-dialog-information")->setShortcut(QKeySequence("Ctrl+I"));
    m_menuTools->addSeparator();
    m_menuTools->addAction(tr("&Download Manager"), this, SLOT(showDownloadManager()), "act-download-manager")->setShortcut(QKeySequence("Ctrl+Y"));
    m_menuTools->addAction(tr("&Cookies Manager"), this, SLOT(showCookieManager()), "act-cookies-manager");
    m_menuTools->addAction(tr("&AdBlock"), AdBlockManager::instance(), SLOT(showDialog()), "act-ad-block");
    m_menuTools->addAction(QIcon(":/icons/menu/rss.png"), tr("RSS &Reader"), this,  SLOT(showRSSManager()), "act-rss-reader");
    m_menuTools->addAction(tr("Web In&spector"), this, SLOT(showWebInspector()), "act-web-inspector")->setShortcut(QKeySequence("Ctrl+Shift+I"));
    m_menuTools->addAction(QIcon::fromTheme("edit-clear"), tr("Clear Recent &History"), this, SLOT(showClearPrivateData()), "act-edit-clear")->setShortcut(QKeySequence("Ctrl+Shift+Del"));
    m_actionPrivateBrowsing = StaticActionManager::actionInstance("act-private-browsing", tr("&Private Browsing"), QIcon(), true);
    m_actionPrivateBrowsing->setShortcut(QKeySequence("Ctrl+Shift+P"));
    m_actionPrivateBrowsing->setVisible(!mApp->isPrivateSession());
    doConnect(m_actionPrivateBrowsing, SIGNAL(triggered(bool)), mApp, SLOT(startPrivateBrowsing()));
    m_menuTools->addAction(m_actionPrivateBrowsing);
    m_menuTools->addSeparator();
#if !defined(Q_WS_X11) && !defined(Q_OS_MAC)
    m_menuTools->addAction(m_actionPreferences);
#endif

    /*************
     * Help Menu *
     *************/
    m_menuHelp = new Menu(tr("&Help"), false);
#ifndef Q_OS_MAC
    m_menuHelp->addAction(QIcon(":/icons/menu/qt.png"), tr("About &Qt"), qApp, SLOT(aboutQt()), "act-about-qt", true);
    m_menuHelp->addAction(m_actionAbout);
    m_menuHelp->addSeparator();
#endif
    Action *infoAction = m_menuHelp->addAction(tr("Information about application"), this, SLOT(loadActionUrlInNewTab()), "act-info-about-app", true);
    infoAction->setData(QUrl("qupzilla:about"));
    infoAction->setShortcut(QKeySequence(QKeySequence::HelpContents));
    m_menuHelp->addAction(tr("Configuration Information"), this, SLOT(loadActionUrlInNewTab()), "act-configuration-information", true)->setData(QUrl("qupzilla:config"));
    m_menuHelp->addAction(tr("Report &Issue"), this, SLOT(loadActionUrlInNewTab()), "act-report-issue", true)->setData(QUrl("qupzilla:reportbug"));

    /************
     * Menu Bar *
     ************/
    addMenusToMenuContainers();

    /*****************
     * Other Actions *
     *****************/
    initializeOtherActions();

#ifdef MENUBAR_USE_STATIC_ACTIONS
    createMenuConnections();
#endif
}

void QupZilla::addMenusToMenuContainers()
{
    menuBar()->setObjectName("mainwindow-menubar");
    menuBar()->setCursor(Qt::ArrowCursor);
    menuBar()->addMenu(m_menuFile);
    menuBar()->addMenu(m_menuEdit);
    menuBar()->addMenu(m_menuView);
    menuBar()->addMenu(m_menuHistory);
    menuBar()->addMenu(m_menuBookmarks);
    menuBar()->addMenu(m_menuTools);
    menuBar()->addMenu(m_menuHelp);
    menuBar()->setContextMenuPolicy(Qt::CustomContextMenu);

#ifndef Q_OS_MAC
    m_superMenu->addMenu(m_menuFile);
    m_superMenu->addMenu(m_menuEdit);
    m_superMenu->addMenu(m_menuView);
    m_superMenu->addMenu(m_menuHistory);
    m_superMenu->addMenu(m_menuBookmarks);
    m_superMenu->addMenu(m_menuTools);
    m_superMenu->addMenu(m_menuHelp);
#endif
}

void QupZilla::loadSettings()
{
    Settings settings;

    //Url settings
    settings.beginGroup("Web-URL-Settings");
    m_homepage = settings.value("homepage", "qupzilla:start").toUrl();
    settings.endGroup();

    QWebSettings* websettings = mApp->webSettings();
    websettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);

    //Browser Window settings
    settings.beginGroup("Browser-View-Settings");
    bool showStatusBar = settings.value("showStatusBar", true).toBool();
    bool showHomeIcon = settings.value("showHomeButton", true).toBool();
    bool showWebSearchBar = settings.value("showWebSearchBar", true).toBool();
    bool showBackForwardIcons = settings.value("showBackForwardButtons", true).toBool();
    bool showBookmarksToolbar = settings.value("showBookmarksToolbar", true).toBool();
    bool showNavigationToolbar = settings.value("showNavigationToolbar", true).toBool();
    bool showMenuBar = settings.value("showMenubar", true).toBool();
    bool showAddTab = settings.value("showAddTabButton", false).toBool();
    bool makeTransparent = settings.value("useTransparentBackground", false).toBool();
    m_sideBarWidth = settings.value("SideBarWidth", 250).toInt();
    m_webViewWidth = settings.value("WebViewWidth", 2000).toInt();
    const QString &activeSideBar = settings.value("SideBar", "None").toString();
    settings.endGroup();
    bool adBlockEnabled = settings.value("AdBlock/enabled", true).toBool();

    m_adblockIcon->setEnabled(adBlockEnabled);

    statusBar()->setVisible(showStatusBar);
    m_bookmarksToolbar->setVisible(showBookmarksToolbar);
    m_navigationBar->setVisible(showNavigationToolbar);
    menuBar()->setVisible(showMenuBar);

#ifndef Q_OS_MAC
    m_navigationBar->buttonSuperMenu()->setVisible(!showMenuBar);
#endif
    m_navigationBar->buttonHome()->setVisible(showHomeIcon);
    m_navigationBar->buttonBack()->setVisible(showBackForwardIcons);
    m_navigationBar->buttonNext()->setVisible(showBackForwardIcons);
    m_navigationBar->searchLine()->setVisible(showWebSearchBar);
    m_navigationBar->buttonAddTab()->setVisible(showAddTab);

    m_sideBarManager->showSideBar(activeSideBar, false);

    //Private browsing
    m_privateBrowsing->setVisible(mApp->isPrivateSession());

#ifdef Q_OS_WIN
    if (m_usingTransparentBackground && !makeTransparent) {
        QtWin::extendFrameIntoClientArea(this, 0, 0, 0, 0);
        QtWin::enableBlurBehindWindow(this, false);
        m_usingTransparentBackground = false;
    }
#endif

    if (!makeTransparent) {
        return;
    }
    //Opacity
#ifdef Q_WS_X11
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, false);
    QPalette pal = palette();
    QColor bg = pal.window().color();
    bg.setAlpha(180);
    pal.setColor(QPalette::Window, bg);
    setPalette(pal);
    ensurePolished(); // workaround Oxygen filling the background
    setAttribute(Qt::WA_StyledBackground, false);
#endif

#ifdef Q_OS_WIN
    if (QtWin::isCompositionEnabled()) {
        setContentsMargins(0, 0, 0, 0);

        m_usingTransparentBackground = true;

        applyBlurToMainWindow();
        //install event filter
        menuBar()->installEventFilter(this);
        m_navigationBar->installEventFilter(this);
        m_bookmarksToolbar->installEventFilter(this);
        statusBar()->installEventFilter(this);
    }
#endif
}

void QupZilla::goNext()
{
    weView()->forward();
}

void QupZilla::goBack()
{
    weView()->back();
}

QMenuBar* QupZilla::menuBar() const
{
#ifdef Q_OS_MAC
    return m_macMenuBar;
#else
    return QMainWindow::menuBar();
#endif
}

TabbedWebView* QupZilla::weView() const
{
    return weView(m_tabWidget->currentIndex());
}

TabbedWebView* QupZilla::weView(int index) const
{
    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    if (!webTab) {
        return 0;
    }

    return webTab->view();
}

LocationBar* QupZilla::locationBar() const
{
    return qobject_cast<LocationBar*>(m_tabWidget->locationBars()->currentWidget());
}

#ifdef MENUBAR_USE_STATIC_ACTIONS
void QupZilla::actionsConnectionManager(const QString &actionName, bool checked, QObject *senderObject, bool isNewWindow)
{
    if (actionName == QLatin1String("act-window-new") ||
            actionName == QLatin1String("act-document-open") ||
            actionName == QLatin1String("act-about-qupZilla") ||
            actionName == QLatin1String("act-about-qt") ||
            actionName == QLatin1String("act-import-bookmarks") ||
            actionName == QLatin1String("act-private-browsing")) {
        return;
    }
    else if (actionName == QLatin1String("act-new-tab")) {
        if (!isNewWindow || weView()->url().toString() == "qupzilla:restore") {
            addTab();
        }
    }
    else if (actionName == QLatin1String("act-application-exit")) {
        quitApp();
    }
    else if (actionName == QLatin1String("act-info-about-app") ||
             actionName == QLatin1String("act-configuration-information") ||
             actionName == QLatin1String("act-report-issue") ||
             actionName.startsWith(QLatin1String("act-bookmark-item-in")) || // left-clicked on a bookmark or history item
             actionName.startsWith(QLatin1String("act-history-item-in"))) {
        if (isNewWindow && weView()->url().toString() != "qupzilla:restore") {
            loadActionUrl(senderObject);
        }
        else {
            loadActionUrlInNewTab(senderObject);
        }
    }
    else if (actionName == QLatin1String("act-document-open-remote")) {
        openLocation();
    }
    else if (actionName == QLatin1String("act-close-tab")) {
        m_tabWidget->closeTab();
    }
    else if (actionName == QLatin1String("act-window-close")) {
        closeWindow();
    }
    else if (actionName == QLatin1String("act-document-save")) {
        savePage();
    }
    else if (actionName == QLatin1String("act-save-page-screen")) {
        savePageScreen();
    }
    else if (actionName == QLatin1String("act-mail-message-new")) {
        sendLink();
    }
    else if (actionName == QLatin1String("act-document-print")) {
        printPage();
    }
    else if (actionName == QLatin1String("act-edit-undo")) {
        editUndo();
    }
    else if (actionName == QLatin1String("act-edit-redo")) {
        editRedo();
    }
    else if (actionName == QLatin1String("act-edit-cut")) {
        editCut();
    }
    else if (actionName == QLatin1String("act-edit-copy")) {
        editCopy();
    }
    else if (actionName == QLatin1String("act-edit-paste")) {
        editPaste();
    }
    else if (actionName == QLatin1String("act-edit-select-all")) {
        editSelectAll();
    }
    else if (actionName == QLatin1String("act-edit-find")) {
        searchOnPage();
    }
    else if (actionName == QLatin1String("act-zoom-in")) {
        zoomIn();
    }
    else if (actionName == QLatin1String("act-zoom-out")) {
        zoomOut();
    }
    else if (actionName == QLatin1String("act-zoom-original")) {
        zoomReset();
    }
    else if (actionName == QLatin1String("act-text-html")) {
        showSource();
    }
    else if (actionName == QLatin1String("act-back")) {
        goBack();
    }
    else if (actionName == QLatin1String("act-forward")) {
        goNext();
    }
    else if (actionName == QLatin1String("act-go-home")) {
        goHome();
    }
    else if (actionName == QLatin1String("act-show-all-history")) {
        showHistoryManager();
    }
    else if (actionName == QLatin1String("act-bookmark-this-page")) {
        bookmarkPage();
    }
    else if (actionName == QLatin1String("act-bookmark-all-tabs")) {
        bookmarkAllTabs();
    }
    else if (actionName == QLatin1String("act-user-bookmarks")) {
        showBookmarksManager();
    }
    else if (actionName == QLatin1String("act-web-search")) {
        webSearch();
    }
    else if (actionName == QLatin1String("act-dialog-information")) {
        showPageInfo();
    }
    else if (actionName == QLatin1String("act-download-manager")) {
        showDownloadManager();
    }
    else if (actionName == QLatin1String("act-cookies-manager")) {
        showCookieManager();
    }
    else if (actionName == QLatin1String("act-ad-block")) {
        AdBlockManager::instance()->showDialog();
    }
    else if (actionName == QLatin1String("act-rss-reader")) {
        showRSSManager();
    }
    else if (actionName == QLatin1String("act-web-inspector")) {
        showWebInspector();
    }
    else if (actionName == QLatin1String("act-edit-clear")) {
        showClearPrivateData();
    }

    else if (actionName == QLatin1String("act-navigation-toolbar")) {
        showNavigationToolbar();
    }
    else if (actionName == QLatin1String("act-bookmarks-toolbar")) {
        showBookmarksToolbar();
    }
    else if (actionName == QLatin1String("act-status-bar")) {
        showStatusbar();
    }
    else if (actionName == QLatin1String("act-menu-bar")) {
        showMenubar();
    }
    else if (actionName == QLatin1String("act-fullscreen")) {
        fullScreen(checked);
    }
    else if (actionName == QLatin1String("act-stop")) {
        stop();
    }
    else if (actionName == QLatin1String("act-reload")) {
        reload();
    }
    else if (actionName == QLatin1String("act-preferences")) {
        showPreferences();
    }
    else if (actionName == QLatin1String("act-character-encoding")) {
        return;// it's a submenu and managed by mApp->menusManageAboutToShow()
    }
}

void QupZilla::menusConnectionManager(const QString &menuName, bool aboutToShow)
{
    if (aboutToShow) {
        if (menuName == QLatin1String("menu-file")) {
            aboutToShowFileMenu();
        }
        else if (menuName == QLatin1String("menu-edit")) {
            aboutToShowEditMenu();
        }
        else if (menuName == QLatin1String("menu-encoding")) {
            aboutToShowEncodingMenu();
        }
        else if (menuName == QLatin1String("menu-view")) {
            aboutToShowViewMenu();
        }
        else if (menuName == QLatin1String("menu-history")) {
            aboutToShowHistoryMenu();
        }
        else if (menuName == QLatin1String("menu-closed-tabs")) {
            aboutToShowClosedTabsMenu();
        }
        else if (menuName == QLatin1String("menu-history-recent")) {
            aboutToShowHistoryRecentMenu();
        }
        else if (menuName == QLatin1String("menu-history-most")) {
            aboutToShowHistoryMostMenu();
        }
        else if (menuName == QLatin1String("menu-bookmarks")) {
            aboutToShowBookmarksMenu();
        }
    }
    else {
        if (menuName == QLatin1String("menu-file")) {
            aboutToHideFileMenu();
        }
        else if (menuName == QLatin1String("menu-edit")) {
            aboutToHideEditMenu();
        }
        else if (menuName == QLatin1String("menu-history")) {
            aboutToHideHistoryMenu();
        }
    }
}

void QupZilla::resetMenuActionsState()
{
    // there is no window just enable some actions.
    if (m_menuClosedTabs) {
        m_menuClosedTabs->clear();
    }
    if (sidebarsMenu) {
        sidebarsMenu->clear();
    }
    StaticActionManager::enableStaticActions(false);
}

void QupZilla::createMenuConnections()
{
    // set name for static objects, they're used in reciver's slot
    m_menuFile->setObjectName("menu-file");
    m_menuEdit->setObjectName("menu-edit");
    m_menuEncoding->setObjectName("menu-encoding");
    m_menuView->setObjectName("menu-view");
    m_menuHistory->setObjectName("menu-history");
    m_menuClosedTabs->setObjectName("menu-closed-tabs");
    m_menuHistoryRecent->setObjectName("menu-history-recent");
    m_menuHistoryMost->setObjectName("menu-history-most");
    m_menuBookmarks->setObjectName("menu-bookmarks");

    connect(m_menuFile, SIGNAL(aboutToShow()), mApp, SLOT(menusManageAboutToShow()));
    connect(m_menuFile, SIGNAL(aboutToHide()), mApp, SLOT(menusManageAboutToHide()));
    connect(m_menuEdit, SIGNAL(aboutToShow()), mApp, SLOT(menusManageAboutToShow()));
    connect(m_menuEdit, SIGNAL(aboutToHide()), mApp, SLOT(menusManageAboutToHide()));
    connect(m_menuEncoding, SIGNAL(aboutToShow()), mApp, SLOT(menusManageAboutToShow()));
    connect(m_menuView, SIGNAL(aboutToShow()), mApp, SLOT(menusManageAboutToShow()));
    connect(m_menuHistory, SIGNAL(aboutToShow()), mApp, SLOT(menusManageAboutToShow()));
    connect(m_menuHistory, SIGNAL(aboutToHide()), mApp, SLOT(menusManageAboutToHide()));
    connect(m_menuClosedTabs, SIGNAL(aboutToShow()), mApp, SLOT(menusManageAboutToShow()));
    connect(m_menuHistoryRecent, SIGNAL(aboutToShow()), mApp, SLOT(menusManageAboutToShow()));
    connect(m_menuHistoryMost, SIGNAL(aboutToShow()), mApp, SLOT(menusManageAboutToShow()));
    connect(m_menuBookmarks, SIGNAL(aboutToShow()), mApp, SLOT(menusManageAboutToShow()));
    connect(m_menuBookmarks, SIGNAL(menuMiddleClicked(Menu*)), mApp, SLOT(manageMiddleClicks(Menu*)));
}
#endif

void QupZilla::setWindowTitle(const QString &t)
{
    QString title = t;

    if (mApp->isPrivateSession()) {
        title.append(tr(" (Private Browsing)"));
    }

    if (m_isStarting) {
        m_lastWindowTitle = title;
        return;
    }

    QMainWindow::setWindowTitle(title);
}

void QupZilla::receiveMessage(Qz::AppMessageType mes, bool state)
{
    switch (mes) {
    case Qz::AM_SetAdBlockIconEnabled:
        m_adblockIcon->setEnabled(state);
        break;

    case Qz::AM_CheckPrivateBrowsing:
        m_privateBrowsing->setVisible(state);
        m_actionPrivateBrowsing->setChecked(state);
        weView()->titleChanged();
        break;

    case Qz::AM_ReloadSettings:
        loadSettings();
        m_tabWidget->loadSettings();
        break;

    case Qz::AM_HistoryStateChanged:
        m_historyMenuChanged = true;
        break;

    case Qz::AM_BookmarksChanged:
        m_bookmarksMenuChanged = true;
        break;

    default:
        qWarning("Unresolved message sent! This could never happen!");
        break;
    }
}

void QupZilla::aboutToShowFileMenu()
{
#ifdef Q_OS_MAC
    m_actionCloseWindow->setEnabled(mApp->windowCount() > 0);
#else
    m_actionCloseWindow->setEnabled(mApp->windowCount() > 1);
#endif
}

void QupZilla::aboutToHideFileMenu()
{
    m_actionCloseWindow->setEnabled(true);
}

void QupZilla::aboutToShowBookmarksMenu()
{
    if (!m_bookmarksMenuChanged) {
        if (m_menuBookmarksAction) {
            m_menuBookmarksAction->setVisible(m_bookmarksToolbar->isVisible());
        }
        return;
    }
    m_bookmarksMenuChanged = false;

    while (m_menuBookmarks->actions().count() != 4) {
        QAction* act = m_menuBookmarks->actions().at(4);
        if (act->menu()) {
            act->menu()->clear();
        }
        m_menuBookmarks->removeAction(act);
        delete act;
    }

    QSqlQuery query;
    query.exec("SELECT title, url, icon FROM bookmarks WHERE folder='bookmarksMenu'");
    while (query.next()) {
        QString title = query.value(0).toString();
        const QUrl &url = query.value(1).toUrl();
        const QIcon &icon = qIconProvider->iconFromImage(QImage::fromData(query.value(2).toByteArray()));
        if (title.length() > 40) {
            title.truncate(40);
            title += "..";
        }

        Action* act = new Action(icon, title);
        act->setData(url);
#ifndef MENUBAR_USE_STATIC_ACTIONS
        connect(act, SIGNAL(triggered()), this, SLOT(loadActionUrl()));
        connect(act, SIGNAL(middleClicked()), this, SLOT(loadActionUrlInNewNotSelectedTab()));
#else
        act->setObjectName("act-bookmark-item-in-menu-folder");
        connect(act, SIGNAL(triggered()), mApp, SLOT(actionsConnectionManager()));
        connect(act, SIGNAL(middleClicked()), mApp, SLOT(manageMiddleClicks()));
#endif
        m_menuBookmarks->addAction(act);
    }

    Menu* menuBookmarks = new Menu(_bookmarksToolbar, m_menuBookmarks);
    menuBookmarks->setIcon(QIcon(style()->standardIcon(QStyle::SP_DirOpenIcon)));

    query.exec("SELECT title, url, icon FROM bookmarks WHERE folder='bookmarksToolbar'");
    while (query.next()) {
        QString title = query.value(0).toString();
        const QUrl &url = query.value(1).toUrl();
        const QIcon &icon = qIconProvider->iconFromImage(QImage::fromData(query.value(2).toByteArray()));
        if (title.length() > 40) {
            title.truncate(40);
            title += "..";
        }

        Action* act = new Action(icon, title);
        act->setData(url);
#ifndef MENUBAR_USE_STATIC_ACTIONS
        connect(act, SIGNAL(triggered()), this, SLOT(loadActionUrl()));
        connect(act, SIGNAL(middleClicked()), this, SLOT(loadActionUrlInNewNotSelectedTab()));
#else
        act->setObjectName("act-bookmark-item-in-toolbar-folder");
        connect(act, SIGNAL(triggered()), mApp, SLOT(actionsConnectionManager()));
        connect(act, SIGNAL(middleClicked()), mApp, SLOT(manageMiddleClicks()));
#endif
        menuBookmarks->addAction(act);
    }
    if (menuBookmarks->isEmpty()) {
        menuBookmarks->addAction(tr("Empty"))->setEnabled(false);
    }
    m_menuBookmarksAction = m_menuBookmarks->addMenu(menuBookmarks);

    query.exec("SELECT name FROM folders");
    while (query.next()) {
        const QString &folderName = query.value(0).toString();
        Menu* tempFolder = new Menu(folderName, m_menuBookmarks);
        tempFolder->setIcon(QIcon(style()->standardIcon(QStyle::SP_DirOpenIcon)));

        QSqlQuery query2;
        query2.prepare("SELECT title, url, icon FROM bookmarks WHERE folder=?");
        query2.addBindValue(folderName);
        query2.exec();
        while (query2.next()) {
            QString title = query2.value(0).toString();
            const QUrl &url = query2.value(1).toUrl();
            const QIcon &icon = qIconProvider->iconFromImage(QImage::fromData(query2.value(2).toByteArray()));
            if (title.length() > 40) {
                title.truncate(40);
                title += "..";
            }

            Action* act = new Action(icon, title);
            act->setData(url);
#ifndef MENUBAR_USE_STATIC_ACTIONS
            connect(act, SIGNAL(triggered()), this, SLOT(loadActionUrl()));
            connect(act, SIGNAL(middleClicked()), this, SLOT(loadActionUrlInNewNotSelectedTab()));
#else
            act->setObjectName("act-bookmark-item-in-user-folders");
            connect(act, SIGNAL(triggered()), mApp, SLOT(actionsConnectionManager()));
            connect(act, SIGNAL(middleClicked()), mApp, SLOT(manageMiddleClicks()));
#endif
            tempFolder->addAction(act);
        }
        if (tempFolder->isEmpty()) {
            tempFolder->addAction(tr("Empty"))->setEnabled(false);
        }
        m_menuBookmarks->addMenu(tempFolder);
    }

    m_menuBookmarks->addSeparator();

    query.exec("SELECT title, url, icon FROM bookmarks WHERE folder='unsorted'");
    while (query.next()) {
        QString title = query.value(0).toString();
        const QUrl &url = query.value(1).toUrl();
        const QIcon &icon = qIconProvider->iconFromImage(QImage::fromData(query.value(2).toByteArray()));
        if (title.length() > 40) {
            title.truncate(40);
            title += "..";
        }

        Action* act = new Action(icon, title);
        act->setData(url);
#ifndef MENUBAR_USE_STATIC_ACTIONS
        connect(act, SIGNAL(triggered()), this, SLOT(loadActionUrl()));
        connect(act, SIGNAL(middleClicked()), this, SLOT(loadActionUrlInNewNotSelectedTab()));
#else
        act->setObjectName("act-bookmark-item-in-unsorted-folder");
        connect(act, SIGNAL(triggered()), mApp, SLOT(actionsConnectionManager()));
        connect(act, SIGNAL(middleClicked()), mApp, SLOT(manageMiddleClicks()));
#endif
        m_menuBookmarks->addAction(act);
    }

    m_menuBookmarksAction->setVisible(m_bookmarksToolbar->isVisible());
}

void QupZilla::aboutToShowHistoryMenu()
{
    TabbedWebView* view = weView();
    if (!view) {
        return;
    }

    m_menuHistory->actions().at(0)->setEnabled(view->history()->canGoBack());
    m_menuHistory->actions().at(1)->setEnabled(view->history()->canGoForward());
}

void QupZilla::aboutToHideHistoryMenu()
{
    m_menuHistory->actions().at(0)->setEnabled(true);
    m_menuHistory->actions().at(1)->setEnabled(true);
}

void QupZilla::aboutToShowClosedTabsMenu()
{
    m_menuClosedTabs->clear();
    int i = 0;
    foreach(const ClosedTabsManager::Tab & tab, m_tabWidget->closedTabsManager()->allClosedTabs()) {
        QString title = tab.title;
        if (title.length() > 40) {
            title.truncate(40);
            title += "..";
        }
        m_menuClosedTabs->addAction(_iconForUrl(tab.url), title, m_tabWidget, SLOT(restoreClosedTab()))->setData(i);
        i++;
    }
    m_menuClosedTabs->addSeparator();
    if (i == 0) {
        m_menuClosedTabs->addAction(tr("Empty"))->setEnabled(false);
    }
    else {
        m_menuClosedTabs->addAction(tr("Restore All Closed Tabs"), m_tabWidget, SLOT(restoreAllClosedTabs()));
        m_menuClosedTabs->addAction(tr("Clear list"), m_tabWidget, SLOT(clearClosedTabsList()));
    }
}

void QupZilla::aboutToShowHistoryRecentMenu()
{
    m_menuHistoryRecent->clear();
    QSqlQuery query;
    query.exec("SELECT title, url FROM history ORDER BY date DESC LIMIT 15");
    while (query.next()) {
        const QUrl &url = query.value(1).toUrl();
        QString title = query.value(0).toString();
        if (title.length() > 40) {
            title.truncate(40);
            title += "..";
        }

        Action* act = new Action(_iconForUrl(url), title);
        act->setData(url);
#ifndef MENUBAR_USE_STATIC_ACTIONS
        connect(act, SIGNAL(triggered()), this, SLOT(loadActionUrl()));
        connect(act, SIGNAL(middleClicked()), this, SLOT(loadActionUrlInNewNotSelectedTab()));
#else
        act->setObjectName("act-history-item-in-recent");
        connect(act, SIGNAL(triggered()), mApp, SLOT(actionsConnectionManager()));
        connect(act, SIGNAL(middleClicked()), mApp, SLOT(manageMiddleClicks()));
#endif
        m_menuHistoryRecent->addAction(act);
    }

    if (m_menuHistoryRecent->isEmpty()) {
        m_menuHistoryRecent->addAction(tr("Empty"))->setEnabled(false);
    }
}

void QupZilla::aboutToShowHistoryMostMenu()
{
    m_menuHistoryMost->clear();

    const QList<HistoryEntry> &mostList = mApp->history()->mostVisited(10);

    foreach(const HistoryEntry & entry, mostList) {
        QString title = entry.title;
        if (title.length() > 40) {
            title.truncate(40);
            title += "..";
        }

        Action* act = new Action(_iconForUrl(entry.url), title);
        act->setData(entry.url);
#ifndef MENUBAR_USE_STATIC_ACTIONS
        connect(act, SIGNAL(triggered()), this, SLOT(loadActionUrl()));
        connect(act, SIGNAL(middleClicked()), this, SLOT(loadActionUrlInNewNotSelectedTab()));
#else
        act->setObjectName("act-history-item-in-most");
        connect(act, SIGNAL(triggered()), mApp, SLOT(actionsConnectionManager()));
        connect(act, SIGNAL(middleClicked()), mApp, SLOT(manageMiddleClicks()));
#endif
        m_menuHistoryMost->addAction(act);
    }

    if (m_menuHistoryMost->isEmpty()) {
        m_menuHistoryMost->addAction(tr("Empty"))->setEnabled(false);
    }
}

void QupZilla::aboutToShowViewMenu()
{
    m_actionShowToolbar->setChecked(m_navigationBar->isVisible());
#ifndef Q_OS_MAC
    m_actionShowMenubar->setChecked(menuBar()->isVisible());
#endif
    m_actionShowStatusbar->setChecked(statusBar()->isVisible());
    m_actionShowBookmarksToolbar->setChecked(m_bookmarksToolbar->isVisible());

    m_sideBarManager->setSideBarMenu(sidebarsMenu);
}

void QupZilla::aboutToShowEditMenu()
{
    WebView* view = weView();

    m_menuEdit->actions().at(0)->setEnabled(view->pageAction(QWebPage::Undo)->isEnabled());
    m_menuEdit->actions().at(1)->setEnabled(view->pageAction(QWebPage::Redo)->isEnabled());
    // Separator
    m_menuEdit->actions().at(3)->setEnabled(view->pageAction(QWebPage::Cut)->isEnabled());
    m_menuEdit->actions().at(4)->setEnabled(view->pageAction(QWebPage::Copy)->isEnabled());
    m_menuEdit->actions().at(5)->setEnabled(view->pageAction(QWebPage::Paste)->isEnabled());
    // Separator
    m_menuEdit->actions().at(7)->setEnabled(view->pageAction(QWebPage::SelectAll)->isEnabled());
}

void QupZilla::aboutToHideEditMenu()
{
    foreach(QAction * act, m_menuEdit->actions()) {
        act->setEnabled(false);
    }

    m_menuEdit->actions().at(8)->setEnabled(true);
    m_actionPreferences->setEnabled(true);
}

void QupZilla::aboutToShowEncodingMenu()
{
    m_menuEncoding->clear();
    QMenu* menuISO = new QMenu("ISO", this);
    QMenu* menuUTF = new QMenu("UTF", this);
    QMenu* menuWindows = new QMenu("Windows", this);
    QMenu* menuIscii = new QMenu("Iscii", this);
    QMenu* menuOther = new QMenu(tr("Other"), this);

    QList<QByteArray> available = QTextCodec::availableCodecs();
    qSort(available);
    const QString &activeCodec = mApp->webSettings()->defaultTextEncoding();

    foreach(const QByteArray & name, available) {
        if (QTextCodec::codecForName(name)->aliases().contains(name)) {
            continue;
        }

        const QString nameString = QString::fromUtf8(name);

        QAction* action = new QAction(nameString, 0);
        action->setData(nameString);
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), this, SLOT(changeEncoding()));
        if (activeCodec.compare(nameString, Qt::CaseInsensitive) == 0) {
            action->setChecked(true);
        }

        if (nameString.startsWith(QLatin1String("ISO"))) {
            menuISO->addAction(action);
        }
        else if (nameString.startsWith(QLatin1String("UTF"))) {
            menuUTF->addAction(action);
        }
        else if (nameString.startsWith(QLatin1String("windows"))) {
            menuWindows->addAction(action);
        }
        else if (nameString.startsWith(QLatin1String("Iscii"))) {
            menuIscii->addAction(action);
        }
        else if (nameString == QLatin1String("System")) {
            m_menuEncoding->addAction(action);
        }
        else {
            menuOther->addAction(action);
        }
    }

    m_menuEncoding->addSeparator();
    if (!menuISO->isEmpty()) {
        m_menuEncoding->addMenu(menuISO);
    }
    if (!menuUTF->isEmpty()) {
        m_menuEncoding->addMenu(menuUTF);
    }
    if (!menuWindows->isEmpty()) {
        m_menuEncoding->addMenu(menuWindows);
    }
    if (!menuIscii->isEmpty()) {
        m_menuEncoding->addMenu(menuIscii);
    }
    if (!menuOther->isEmpty()) {
        m_menuEncoding->addMenu(menuOther);
    }
}

void QupZilla::changeEncoding()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        const QString &encoding = action->data().toString();
        mApp->webSettings()->setDefaultTextEncoding(encoding);

        Settings settings;
        settings.setValue("Web-Browser-Settings/DefaultEncoding", encoding);

        reload();
    }
}

void QupZilla::bookmarkPage()
{
    mApp->browsingLibrary()->bookmarksManager()->addBookmark(weView());
}

void QupZilla::addBookmark(const QUrl &url, const QString &title, const QIcon &icon)
{
    mApp->browsingLibrary()->bookmarksManager()->insertBookmark(url, title, icon);
}

void QupZilla::bookmarkAllTabs()
{
    mApp->browsingLibrary()->bookmarksManager()->insertAllTabs();
}

void QupZilla::newWindow()
{
    mApp->makeNewWindow(Qz::BW_NewWindow);
}

void QupZilla::goHome()
{
    loadAddress(m_homepage);
}

void QupZilla::editUndo()
{
    weView()->triggerPageAction(QWebPage::Undo);
}

void QupZilla::editRedo()
{
    weView()->triggerPageAction(QWebPage::Redo);
}

void QupZilla::editCut()
{
    weView()->triggerPageAction(QWebPage::Cut);
}

void QupZilla::editCopy()
{
    weView()->triggerPageAction(QWebPage::Copy);
}

void QupZilla::editPaste()
{
    weView()->triggerPageAction(QWebPage::Paste);
}

void QupZilla::editSelectAll()
{
    weView()->selectAll();
}

void QupZilla::zoomIn()
{
    weView()->zoomIn();
}

void QupZilla::zoomOut()
{
    weView()->zoomOut();
}

void QupZilla::zoomReset()
{
    weView()->zoomReset();
}

void QupZilla::goHomeInNewTab()
{
    m_tabWidget->addView(m_homepage, Qz::NT_SelectedTab);
}

void QupZilla::stop()
{
    weView()->stop();
}

void QupZilla::reload()
{
    weView()->reload();
}

void QupZilla::reloadByPassCache()
{
    weView()->triggerPageAction(QWebPage::ReloadAndBypassCache);
}

void QupZilla::loadActionUrl(QObject *senderObject)
{
    if (!senderObject) {
        senderObject = sender();
    }
    if (QAction* action = qobject_cast<QAction*>(senderObject)) {
        loadAddress(action->data().toUrl());
    }
}

void QupZilla::loadActionUrlInNewTab(QObject *senderObject)
{
    if (!senderObject) {
        senderObject = sender();
    }
    if (QAction* action = qobject_cast<QAction*>(senderObject)) {
        m_tabWidget->addView(action->data().toUrl(), Qz::NT_SelectedTabAtTheEnd);
    }
}

void QupZilla::loadActionUrlInNewNotSelectedTab(QObject *senderObject)
{
    if (!senderObject) {
        senderObject = sender();
    }
    if (QAction* action = qobject_cast<QAction*>(senderObject)) {
        m_tabWidget->addView(action->data().toUrl(), Qz::NT_NotSelectedTab);
    }
}

void QupZilla::loadFolderBookmarks(Menu* menu)
{
    const QString &folder = BookmarksModel::fromTranslatedFolder(menu->title());
    if (folder.isEmpty()) {
        return;
    }

    foreach(const Bookmark & b, mApp->bookmarksModel()->folderBookmarks(folder)) {
        tabWidget()->addView(b.url, b.title, Qz::NT_NotSelectedTab);
    }
}

void QupZilla::loadAddress(const QUrl &url)
{
    weView()->setFocus();
    weView()->load(url);
}

void QupZilla::showCookieManager()
{
    CookieManager* m = mApp->cookieManager();
    m->refreshTable();

    m->show();
    m->raise();
}


void QupZilla::showHistoryManager()
{
    mApp->browsingLibrary()->showHistory(this);
}

void QupZilla::showRSSManager()
{
    mApp->browsingLibrary()->showRSS(this);
}

void QupZilla::showBookmarksManager()
{
    mApp->browsingLibrary()->showBookmarks(this);
}

void QupZilla::showClearPrivateData()
{
    ClearPrivateData clear(this, this);
    clear.exec();
}

void QupZilla::showDownloadManager()
{
    mApp->downManager()->show();
}

void QupZilla::showPreferences()
{
    Preferences* prefs = new Preferences(mApp->getWindow(), mApp->getWindow());
    prefs->show();
}

void QupZilla::showSource(QWebFrame* frame, const QString &selectedHtml)
{
    if (!frame) {
        frame = weView()->page()->mainFrame();
    }

    SourceViewer* source = new SourceViewer(frame, selectedHtml);
    qz_centerWidgetToParent(source, this);
    source->show();
}

void QupZilla::showPageInfo()
{
    SiteInfo* info = new SiteInfo(weView(), this);
    info->setAttribute(Qt::WA_DeleteOnClose);
    info->show();
}

void QupZilla::showBookmarksToolbar()
{
    bool status = m_bookmarksToolbar->isVisible();
    m_bookmarksToolbar->setVisible(!status);

    Settings settings;
    settings.setValue("Browser-View-Settings/showBookmarksToolbar", !status);
}

SideBar* QupZilla::addSideBar()
{
    if (m_sideBar) {
        return m_sideBar.data();
    }

    m_sideBar = new SideBar(m_sideBarManager, this);

    m_mainSplitter->insertWidget(0, m_sideBar.data());
    m_mainSplitter->setCollapsible(0, false);

    m_mainSplitter->setSizes(QList<int>() << m_sideBarWidth << m_webViewWidth);

#ifdef Q_OS_WIN
    if (QtWin::isCompositionEnabled()) {
        applyBlurToMainWindow();
        m_sideBar.data()->installEventFilter(this);
    }
#endif

    return m_sideBar.data();
}

void QupZilla::saveSideBarWidth()
{
    // That +1 is important here, without it, the sidebar width would
    // decrease by 1 pixel every close

    m_sideBarWidth = m_mainSplitter->sizes().at(0) + 1;
    m_webViewWidth = width() - m_sideBarWidth;
}

void QupZilla::showNavigationToolbar()
{
    if (!menuBar()->isVisible() && !m_actionShowToolbar->isChecked()) {
        showMenubar();
    }

    bool status = m_navigationBar->isVisible();
    m_navigationBar->setVisible(!status);

    Settings settings;
    settings.setValue("Browser-View-Settings/showNavigationToolbar", !status);
}

void QupZilla::showMenubar()
{
#ifndef Q_OS_MAC
    if (!m_navigationBar->isVisible() && !m_actionShowMenubar->isChecked()) {
        showNavigationToolbar();
    }

    menuBar()->setVisible(!menuBar()->isVisible());
    m_navigationBar->buttonSuperMenu()->setVisible(!menuBar()->isVisible());

    Settings settings;
    settings.setValue("Browser-View-Settings/showMenubar", menuBar()->isVisible());
#endif
}

void QupZilla::showStatusbar()
{
    bool status = statusBar()->isVisible();
    statusBar()->setVisible(!status);

    Settings settings;
    settings.setValue("Browser-View-Settings/showStatusbar", !status);
}

void QupZilla::showWebInspector(bool toggle)
{
    if (m_webInspectorDock) {
        if (toggle) {
            m_webInspectorDock.data()->toggleVisibility();
        }
        else  {
            m_webInspectorDock.data()->show();
        }
        return;
    }

    m_webInspectorDock = new WebInspectorDockWidget(this);
    connect(m_tabWidget, SIGNAL(currentChanged(int)), m_webInspectorDock.data(), SLOT(tabChanged()));
    addDockWidget(Qt::BottomDockWidgetArea, m_webInspectorDock.data());

#ifdef Q_OS_WIN
    if (QtWin::isCompositionEnabled()) {
        applyBlurToMainWindow();
        m_webInspectorDock.data()->installEventFilter(this);
    }
#endif
}

void QupZilla::showBookmarkImport()
{
    BookmarksImportDialog* b = new BookmarksImportDialog(mApp->getWindow());
    b->show();
}

void QupZilla::refreshHistory()
{
    m_navigationBar->refreshHistory();
}

void QupZilla::currentTabChanged()
{
    TabbedWebView* view = weView();
    if (!view) {
        return;
    }

    setWindowTitle(tr("%1 - QupZilla").arg(view->title()));
    m_ipLabel->setText(view->getIp());
    view->setFocus();

    updateLoadingActions();

    // Setting correct tab order (LocationBar -> WebSearchBar -> WebView)
    setTabOrder(locationBar(), m_navigationBar->searchLine());
    setTabOrder(m_navigationBar->searchLine(), view);
}

void QupZilla::updateLoadingActions()
{
    TabbedWebView* view = weView();
    if (!view) {
        return;
    }

    bool isLoading = view->isLoading();

    m_ipLabel->setVisible(!isLoading);
    m_progressBar->setVisible(isLoading);
    m_actionStop->setEnabled(isLoading);
    m_actionReload->setEnabled(!isLoading);

    if (isLoading) {
        m_progressBar->setValue(view->loadingProgress());
        m_navigationBar->showStopButton();
    }
    else {
        m_navigationBar->showReloadButton();
    }
}

void QupZilla::addDeleteOnCloseWidget(QWidget* widget)
{
    if (!m_deleteOnCloseWidgets.contains(widget)) {
        m_deleteOnCloseWidgets.append(widget);
    }
}

void QupZilla::restoreWindowState(const RestoreManager::WindowData &d)
{
    restoreState(d.windowState);
    m_tabWidget->restoreState(d.tabsState, d.currentTab);
}

void QupZilla::aboutQupZilla()
{
    AboutDialog about(mApp->getWindow());
    about.exec();
}

void QupZilla::addTab()
{
    m_tabWidget->addView(QUrl(), Qz::NT_SelectedTabAtTheEnd, true);
}

void QupZilla::webSearch()
{
    m_navigationBar->searchLine()->setFocus();
    m_navigationBar->searchLine()->selectAll();
}

void QupZilla::searchOnPage()
{
    if (m_mainLayout->count() == 4) {
        SearchToolBar* search = qobject_cast<SearchToolBar*>(m_mainLayout->itemAt(3)->widget());
        if (!search) {
            return;
        }

        search->focusSearchLine();
        return;
    }

    SearchToolBar* search = new SearchToolBar(this);
    m_mainLayout->insertWidget(3, search);
    search->focusSearchLine();

#ifdef Q_OS_WIN
    if (QtWin::isCompositionEnabled()) {
        applyBlurToMainWindow();
        search->installEventFilter(this);
    }
#endif
}

void QupZilla::openFile()
{
    const QString &fileTypes = QString("%1(*.html *.htm *.shtml *.shtm *.xhtml);;"
                                       "%2(*.png *.jpg *.jpeg *.bmp *.gif *.svg *.tiff);;"
                                       "%3(*.txt);;"
                                       "%4(*.*)").arg(tr("HTML files"), tr("Image files"), tr("Text files"), tr("All files"));

    const QString &filePath = QFileDialog::getOpenFileName(mApp->getWindow(), tr("Open file..."), QDir::homePath(), fileTypes);

    if (!filePath.isEmpty()) {
        if (mApp->windowCount() == 0) {
            mApp->makeNewWindow(Qz::BW_FirstAppWindow, QUrl::fromLocalFile(filePath));
        }
        else {
            mApp->getWindow()->loadAddress(QUrl::fromLocalFile(filePath));
        }
    }
}

void QupZilla::openLocation()
{
    locationBar()->setFocus();
    locationBar()->selectAll();
}

void QupZilla::showNavigationWithFullscreen()
{
    bool state;
    if (m_navigationVisible) {
        state = !m_navigationBar->isVisible();
    }
    else {
        state = !m_tabWidget->getTabBar()->isVisible();
    }

    if (m_navigationVisible) {
        m_navigationBar->setVisible(state);
    }

    m_tabWidget->getTabBar()->updateVisibilityWithFullscreen(state);

    if (m_bookmarksToolBarVisible) {
        m_bookmarksToolbar->setVisible(state);
    }
}

void QupZilla::fullScreen(bool make)
{
    if (make) {
        m_menuBarVisible = menuBar()->isVisible();
        m_statusBarVisible = statusBar()->isVisible();
        m_navigationVisible = m_navigationBar->isVisible();
        m_bookmarksToolBarVisible = m_bookmarksToolbar->isVisible();

        setWindowState(windowState() | Qt::WindowFullScreen);

        menuBar()->hide();
        statusBar()->hide();
        bookmarksToolbar()->hide();
        m_navigationBar->hide();
        m_tabWidget->getTabBar()->hide();
#ifdef Q_OS_WIN
        if (m_usingTransparentBackground) {
            QtWin::extendFrameIntoClientArea(this, 0, 0, 0 , 0);
            QtWin::enableBlurBehindWindow(this, false);
        }
#endif
    }
    else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);

        menuBar()->setVisible(m_menuBarVisible);
        statusBar()->setVisible(m_statusBarVisible);
        m_bookmarksToolbar->setVisible(m_bookmarksToolBarVisible);
        m_navigationBar->setVisible(m_navigationVisible);
        m_tabWidget->showTabBar();
#ifdef Q_OS_WIN
        if (m_usingTransparentBackground) {
            applyBlurToMainWindow(true);
        }
#endif
    }

    m_actionShowFullScreen->setChecked(make);
    m_navigationBar->buttonExitFullscreen()->setVisible(make);

    emit setWebViewMouseTracking(make);
}

void QupZilla::savePage()
{
    weView()->savePageAs();
}

void QupZilla::sendLink()
{
    weView()->sendPageByMail();
}

void QupZilla::printPage(QWebFrame* frame)
{
    QPrintPreviewDialog* dialog = new QPrintPreviewDialog(this);
    dialog->resize(800, 750);

    if (!frame) {
        connect(dialog, SIGNAL(paintRequested(QPrinter*)), weView(), SLOT(print(QPrinter*)));
    }
    else {
        connect(dialog, SIGNAL(paintRequested(QPrinter*)), frame, SLOT(print(QPrinter*)));
    }

    dialog->exec();

    dialog->deleteLater();
}

void QupZilla::savePageScreen()
{
    PageScreen* p = new PageScreen(weView(), this);
    p->show();
}

void QupZilla::resizeEvent(QResizeEvent* event)
{
    m_bookmarksToolbar->setMaximumWidth(width());

    QMainWindow::resizeEvent(event);
}

void QupZilla::keyPressEvent(QKeyEvent* event)
{
    if (mApp->plugins()->processKeyPress(Qz::ON_QupZilla, this, event)) {
        return;
    }

    int number = -1;

    switch (event->key()) {
    case Qt::Key_Back:
        weView()->back();
        event->accept();
        break;

    case Qt::Key_Forward:
        weView()->forward();
        event->accept();
        break;

    case Qt::Key_Stop:
        weView()->stop();
        event->accept();
        break;

    case Qt::Key_Refresh:
        weView()->reload();
        event->accept();
        break;

    case Qt::Key_HomePage:
        goHome();
        event->accept();
        break;

    case Qt::Key_Favorites:
        showBookmarksManager();
        event->accept();
        break;

    case Qt::Key_Search:
        searchOnPage();
        event->accept();
        break;

    case Qt::Key_F6:
    case Qt::Key_OpenUrl:
        openLocation();
        event->accept();
        break;

    case Qt::Key_History:
        showHistoryManager();
        event->accept();
        break;

    case Qt::Key_AddFavorite:
        bookmarkPage();
        event->accept();
        break;

    case Qt::Key_News:
        showRSSManager();
        event->accept();
        break;

    case Qt::Key_Tools:
        showPreferences();
        event->accept();
        break;

    case Qt::Key_Tab:
        if (event->modifiers() == Qt::ControlModifier) {
            m_tabWidget->nextTab();
            event->accept();
        }
        break;

    case Qt::Key_Backtab:
        if (event->modifiers() == (Qt::ControlModifier + Qt::ShiftModifier)) {
            m_tabWidget->previousTab();
            event->accept();
        }
        break;

    case Qt::Key_PageDown:
        if (event->modifiers() == Qt::ControlModifier) {
            m_tabWidget->nextTab();
            event->accept();
        }
        break;

    case Qt::Key_PageUp:
        if (event->modifiers() == Qt::ControlModifier) {
            m_tabWidget->previousTab();
            event->accept();
        }
        break;

    case Qt::Key_Equal:
        if (event->modifiers() == Qt::ControlModifier) {
            weView()->zoomIn();
            event->accept();
        }
        break;

    case Qt::Key_1:
        number = 1;
        break;
    case Qt::Key_2:
        number = 2;
        break;
    case Qt::Key_3:
        number = 3;
        break;
    case Qt::Key_4:
        number = 4;
        break;
    case Qt::Key_5:
        number = 5;
        break;
    case Qt::Key_6:
        number = 6;
        break;
    case Qt::Key_7:
        number = 7;
        break;
    case Qt::Key_8:
        number = 8;
        break;
    case Qt::Key_9:
        number = 9;
        break;

    default:
        break;
    }

    if (number != -1) {
        if (event->modifiers() & Qt::AltModifier) {
            if (number == 9) {
                number = m_tabWidget->count();
            }
            m_tabWidget->setCurrentIndex(number - 1);
            return;
        }
        if (event->modifiers() & Qt::ControlModifier) {
            const QUrl &url = mApp->plugins()->speedDial()->urlForShortcut(number - 1);
            if (url.isValid()) {
                loadAddress(url);
                return;
            }
        }
    }

    QMainWindow::keyPressEvent(event);
}

void QupZilla::keyReleaseEvent(QKeyEvent* event)
{
    if (mApp->plugins()->processKeyRelease(Qz::ON_QupZilla, this, event)) {
        return;
    }

    QMainWindow::keyReleaseEvent(event);
}

void QupZilla::closeEvent(QCloseEvent* event)
{
    if (mApp->isClosing()) {
        return;
    }

    Settings settings;
    int afterLaunch = settings.value("Web-URL-Settings/afterLaunch", 1).toInt();
    bool askOnClose = settings.value("Browser-Tabs-Settings/AskOnClosing", true).toBool();

    if (afterLaunch == 3 && mApp->windowCount() == 1) {
        askOnClose = false;
    }

    if (askOnClose && m_tabWidget->normalTabsCount() > 1) {
        CheckBoxDialog dialog(QDialogButtonBox::Yes | QDialogButtonBox::No, this);
        dialog.setText(tr("There are still %1 open tabs and your session won't be stored. \nAre you sure to quit QupZilla?").arg(m_tabWidget->count()));
        dialog.setCheckBoxText(tr("Don't ask again"));
        dialog.setWindowTitle(tr("There are still open tabs"));
        dialog.setIcon(qIconProvider->standardIcon(QStyle::SP_MessageBoxWarning));

        if (dialog.exec() != QDialog::Accepted) {
            event->ignore();
            return;
        }

        if (dialog.isChecked()) {
            settings.setValue("Browser-Tabs-Settings/AskOnClosing", false);
        }
    }

    m_isClosing = true;
    mApp->saveStateSlot();

#ifndef Q_OS_MAC
    if (mApp->windowCount() == 1) {
        if (quitApp()) {
            disconnectObjects();
            event->accept();
        }
        else {
            event->ignore();
        }

        return;
    }
#endif

#ifdef MENUBAR_USE_STATIC_ACTIONS
    if (mApp->windowCount() == 1) {
        resetMenuActionsState();
    }
#endif

    mApp->aboutToCloseWindow(this);

    disconnectObjects();
    event->accept();
}

void QupZilla::disconnectObjects()
{
    // Disconnecting all important widgets before deleting this window
    // so it cannot happen that slots will be invoked after the object
    // is deleted.
    // We have to do it this way, because ~QObject is deleting all child
    // objects with plain delete - not deleteLater().
    //
    // Also using own disconnectObjects() method, not default disconnect()
    // because we need to retain connections to destroyed(QObject*) signal
    // in order to avoid crashes for example with setting stylesheets
    // (QStyleSheet backend is holding list of all widgets)

    m_tabWidget->disconnectObjects();
    m_tabWidget->getTabBar()->disconnectObjects();

    foreach(WebTab * tab, m_tabWidget->allTabs()) {
        tab->disconnectObjects();
        tab->view()->disconnectObjects();
        tab->view()->page()->disconnectObjects();
    }

    foreach(const QWeakPointer<QWidget> &pointer, m_deleteOnCloseWidgets) {
        if (pointer) {
            pointer.data()->deleteLater();
        }
    }

    mApp->plugins()->emitMainWindowDeleted(this);
}

void QupZilla::closeWindow()
{
    if (mApp->windowCount() > 1) {
        close();
    }
}

bool QupZilla::quitApp()
{
    if (m_sideBar) {
        saveSideBarWidth();
    }

    if (!mApp->isPrivateSession()) {
        Settings settings;
        settings.beginGroup("Browser-View-Settings");
        settings.setValue("WindowMaximised", windowState().testFlag(Qt::WindowMaximized));
        settings.setValue("WindowGeometry", saveGeometry());
        settings.setValue("LocationBarWidth", m_navigationBar->splitter()->sizes().at(0));
        settings.setValue("WebSearchBarWidth", m_navigationBar->splitter()->sizes().at(1));
        settings.setValue("SideBarWidth", m_sideBarWidth);
        settings.setValue("WebViewWidth", m_webViewWidth);
        settings.endGroup();
    }

    mApp->quitApplication();
    return true;
}

QByteArray QupZilla::saveState(int version) const
{
#ifdef Q_WS_X11
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << QMainWindow::saveState(version);
    stream << getCurrentVirtualDesktop();

    return data;
#else
    return QMainWindow::saveState(version);
#endif
}

bool QupZilla::restoreState(const QByteArray &state, int version)
{
#ifdef Q_WS_X11
    QByteArray windowState;
    int desktopId = -1;

    QDataStream stream(state);
    stream >> windowState;
    stream >> desktopId;

    moveToVirtualDesktop(desktopId);

    return QMainWindow::restoreState(windowState, version);
#else
    return QMainWindow::restoreState(state, version);
#endif
}

#ifdef Q_WS_X11
int QupZilla::getCurrentVirtualDesktop() const
{
    Display* display = QX11Info::display();
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes;
    unsigned long* data;

    Atom net_wm_desktop = XInternAtom(display, "_NET_WM_DESKTOP", False);
    if (net_wm_desktop == None) {
        return -1;
    }

    int status = XGetWindowProperty(display, winId(), net_wm_desktop, 0, 1,
                                    False, XA_CARDINAL, &actual_type, &actual_format,
                                    &nitems, &bytes, (unsigned char**) &data);

    if (status != Success || data == NULL) {
        return -1;
    }

    int desktop = *data;
    XFree(data);

    return desktop;
}

void QupZilla::moveToVirtualDesktop(int desktopId)
{
    // Don't move when window is already visible or it is first app window
    if (desktopId < 0 || isVisible() || m_startBehaviour == Qz::BW_FirstAppWindow) {
        return;
    }

    Display* display = QX11Info::display();

    Atom net_wm_desktop = XInternAtom(display, "_NET_WM_DESKTOP", False);
    if (net_wm_desktop == None) {
        return;
    }

    XChangeProperty(display, winId(), net_wm_desktop, XA_CARDINAL,
                    32, PropModeReplace, (unsigned char*) &desktopId, 1L);
}
#endif

#ifdef Q_OS_WIN
bool QupZilla::winEvent(MSG* message, long* result)
{
    if (message && message->message == WM_DWMCOMPOSITIONCHANGED) {
        Settings settings;
        settings.beginGroup("Browser-View-Settings");
        m_usingTransparentBackground = settings.value("useTransparentBackground", false).toBool();
        settings.endGroup();
        if (m_usingTransparentBackground && QtWin::isCompositionEnabled()) {
            setUpdatesEnabled(false);

            QtWin::extendFrameIntoClientArea(this, 0, 0, 0, 0);
            QTimer::singleShot(0, this, SLOT(applyBlurToMainWindow()));

            //install event filter
            menuBar()->installEventFilter(this);
            m_navigationBar->installEventFilter(this);
            m_bookmarksToolbar->installEventFilter(this);
            statusBar()->installEventFilter(this);

            if (m_sideBar) {
                m_sideBar.data()->installEventFilter(this);
            }

            if (m_mainLayout->count() == 4) {
                SearchToolBar* search = qobject_cast<SearchToolBar*>(m_mainLayout->itemAt(3)->widget());
                if (search) {
                    search->installEventFilter(this);
                }
            }

            if (m_webInspectorDock) {
                m_webInspectorDock.data()->installEventFilter(this);
            }

            if (isVisible()) {
                hide();
                show();
            }
            setUpdatesEnabled(true);
        }
    }
    return QMainWindow::winEvent(message, result);
}

void QupZilla::applyBlurToMainWindow(bool force)
{
    if (!force && (m_actionShowFullScreen->isChecked() || !m_usingTransparentBackground)) {
        return;
    }
    int topMargin = 0;
    int bottomMargin = 1;
    int rightMargin = 1;
    int leftMargin = 1;

    if (m_sideBar) {
        if (isRightToLeft()) {
            rightMargin += m_sideBar.data()->width() + m_mainSplitter->handleWidth();
        }
        else {
            leftMargin += m_sideBar.data()->width() + m_mainSplitter->handleWidth();
        }
    }

    topMargin += menuBar()->isVisible() ? menuBar()->height() : 0;
    topMargin += m_navigationBar->isVisible() ? m_navigationBar->height() : 0;
    topMargin += m_bookmarksToolbar->isVisible() ? m_bookmarksToolbar->height() : 0;
    topMargin += m_tabWidget->getTabBar()->height();

    if (m_mainLayout->count() == 4) {
        SearchToolBar* search = qobject_cast<SearchToolBar*>(m_mainLayout->itemAt(3)->widget());
        if (search) {
            bottomMargin += search->height();
        }
    }

    bottomMargin += statusBar()->isVisible() ? statusBar()->height() : 0;

    if (m_webInspectorDock) {
        bottomMargin += m_webInspectorDock.data()->isVisible()
                        ? m_webInspectorDock.data()->height()
                        + m_webInspectorDock.data()->style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent)
                        : 0;
    }

    QtWin::extendFrameIntoClientArea(this, leftMargin, topMargin, rightMargin, bottomMargin);
}

bool QupZilla::eventFilter(QObject* object, QEvent* event)
{
    switch (event->type()) {
    case QEvent::DeferredDelete:
    case QEvent::Show:
    case QEvent::Hide:
    case QEvent::Resize:
        applyBlurToMainWindow();
        break;
    default:
        break;
    }

    return QMainWindow::eventFilter(object, event);
}
#endif

bool QupZilla::doConnect(const QObject* sender, const char* signal, const QObject* receiver, const char* member)
{
#ifndef MENUBAR_USE_STATIC_ACTIONS
    return QObject::connect(sender, signal, receiver, member);
#else
    Q_UNUSED(sender);
    Q_UNUSED(signal);
    Q_UNUSED(receiver);
    Q_UNUSED(member);
    return false;
#endif
}

void QupZilla::initializeOtherActions()
{
    m_actionRestoreTab = new QAction(QIcon::fromTheme("user-trash"), tr("Restore &Closed Tab"), this);
    m_actionRestoreTab->setShortcut(QKeySequence("Ctrl+Shift+T"));
    doConnect(m_actionRestoreTab, SIGNAL(triggered()), m_tabWidget, SLOT(restoreClosedTab()));
    addAction(m_actionRestoreTab);

    QShortcut* reloadByPassCacheAction = new QShortcut(QKeySequence("Ctrl+F5"), this);
    QShortcut* reloadByPassCacheAction2 = new QShortcut(QKeySequence("Ctrl+Shift+R"), this);
    connect(reloadByPassCacheAction, SIGNAL(activated()), this, SLOT(reloadByPassCache()));
    connect(reloadByPassCacheAction2, SIGNAL(activated()), this, SLOT(reloadByPassCache()));

    QShortcut* reloadAction = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(reloadAction, SIGNAL(activated()), this, SLOT(reload()));

    QShortcut* backAction = new QShortcut(QKeySequence("Alt+Left"), this);
    connect(backAction, SIGNAL(activated()), this, SLOT(goBack()));

    QShortcut* forwardAction = new QShortcut(QKeySequence("Alt+Right"), this);
    connect(forwardAction, SIGNAL(activated()), this, SLOT(goNext()));

    QShortcut* openLocationAction = new QShortcut(QKeySequence("Alt+D"), this);
    connect(openLocationAction, SIGNAL(activated()), this, SLOT(openLocation()));

    // Make shortcuts available even in fullscreen (menu hidden)
    QList<QAction*> actions = menuBar()->actions();
    foreach(QAction * action, actions) {
        if (action->menu()) {
            actions += action->menu()->actions();
        }
        addAction(action);
    }
}

QupZilla::~QupZilla()
{
}
