/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#include "webview.h"
#include "lineedit.h"
#include "historymodel.h"
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
#include "qtwin.h"
#include "ui_closedialog.h"
#include "adblockmanager.h"
#include "clickablelabel.h"
#include "docktitlebarwidget.h"
#include "sidebar.h"
#include "iconprovider.h"
#include "progressbar.h"
#include "adblockicon.h"
#include "closedtabsmanager.h"
#include "statusbarmessage.h"
#include "locationbarsettings.h"
#include "browsinglibrary.h"
#include "navigationbar.h"
#include "pagescreen.h"
#include "webinspectordockwidget.h"
#include "bookmarksimportdialog.h"
#include "globalfunctions.h"
#include "webhistorywrapper.h"
#include "enhancedmenu.h"

const QString QupZilla::VERSION = "1.1.0";
const QString QupZilla::BUILDTIME =  __DATE__" "__TIME__;
const QString QupZilla::AUTHOR = "David Rosca";
const QString QupZilla::COPYRIGHT = "2010-2011";
const QString QupZilla::WWWADDRESS = "http://qupzilla.co.cc";
const QString QupZilla::WIKIADDRESS = "https://github.com/nowrep/QupZilla/wiki";
const QString QupZilla::WEBKITVERSION = qWebKitVersion();

const QIcon QupZilla::qupzillaIcon()
{
    QIcon i;
    i.addFile(":icons/exeicons/qupzilla16.png");
    i.addFile(":icons/exeicons/qupzilla32.png");
    i.addFile(":icons/exeicons/qupzilla48.png");
    i.addFile(":icons/exeicons/qupzilla64.png");
    i.addFile(":icons/exeicons/qupzilla128.png");
    i.addFile(":icons/exeicons/qupzilla256.png");
    return i;
}

QupZilla::QupZilla(StartBehaviour behaviour, QUrl startUrl)
    : QMainWindow(0)
    , m_historyMenuChanged(true)
    , m_bookmarksMenuChanged(true)
    , m_isClosing(false)
    , m_startingUrl(startUrl)
    , m_startBehaviour(behaviour)
    , m_menuBookmarksAction(0)
    , m_actionPrivateBrowsing(0)
    , m_statusBarMessage(new StatusBarMessage(this))
    , m_sideBarWidth(0)
{
    setObjectName("mainwindow");
    setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowTitle("QupZilla");
    setUpdatesEnabled(false);

    m_activeProfil = mApp->getActiveProfilPath();
    m_activeLanguage = mApp->getActiveLanguage();

    QDesktopServices::setUrlHandler("http", this, "loadAddress");

    setupUi();
    setupMenu();
    QTimer::singleShot(0, this, SLOT(postLaunch()));
    connect(mApp, SIGNAL(message(MainApplication::MessageType, bool)), this, SLOT(receiveMessage(MainApplication::MessageType, bool)));
}

void QupZilla::postLaunch()
{
    loadSettings();
    m_tabWidget->restorePinnedTabs();

    //Open tab from command line argument
    bool addTab = true;
    QStringList arguments = qApp->arguments();
    for (int i = 0; i < qApp->arguments().count(); i++) {
        QString arg = arguments.at(i);
        if (arg.startsWith("-url=")) {
            m_tabWidget->addView(QUrl(arg.replace("-url=", "")));
            addTab = false;
        }
    }

    QSettings settings(m_activeProfil + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-URL-Settings");
    int afterLaunch = settings.value("afterLaunch", 1).toInt();
    settings.endGroup();
    settings.beginGroup("SessionRestore");
    bool startingAfterCrash = settings.value("isCrashed", false).toBool();
    settings.endGroup();

    QUrl startUrl;
    switch (afterLaunch) {
    case 0:
        startUrl = QUrl("");
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
    case FirstAppWindow:
        if (startingAfterCrash || (addTab && afterLaunch == 3)) {
            addTab = !mApp->restoreStateSlot(this);
        }
        break;

    case NewWindow:
        addTab = true;
        break;

    case OtherRestoredWindow:
        addTab = false;
        break;
    }

    if (!m_startingUrl.isEmpty()) {
        startUrl = WebView::guessUrlFromString(m_startingUrl.toString());
        addTab = true;
    }

    aboutToShowHistoryMenu(false);
    aboutToShowBookmarksMenu();

    if (addTab) {
        int index = m_tabWidget->addView(startUrl, tr("New tab"), TabWidget::CleanPage);
        m_tabWidget->setCurrentIndex(index);

        if (startUrl.isEmpty() || startUrl.toString() == "qupzilla:speeddial") {
            locationBar()->setFocus();
        }
    }

    if (m_tabWidget->getTabBar()->normalTabsCount() <= 0) { //Something went really wrong .. add one tab
        m_tabWidget->addView(m_homepage);
    }

    setUpdatesEnabled(true);

    emit startingCompleted();
}

void QupZilla::setupUi()
{
    int locationBarWidth;
    int websearchBarWidth;

    QSettings settings(m_activeProfil + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Browser-View-Settings");
    if (settings.value("WindowMaximised", false).toBool()) {
        resize(800, 550);
        setWindowState(Qt::WindowMaximized);
    }
    else {
        setGeometry(settings.value("WindowGeometry", QRect(20, 20, 800, 550)).toRect());
        if (m_startBehaviour == NewWindow) {
            // Moving window +40 x,y to be visible that this is new window
            QPoint p = pos();
            p.setX(p.x() + 40);
            p.setY(p.y() + 40);

            move(p);
        }
    }

    locationBarWidth = settings.value("LocationBarWidth", 480).toInt();
    websearchBarWidth = settings.value("WebSearchBarWidth", 140).toInt();

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
    menuBar()->setObjectName("mainwindow-menubar");
    menuBar()->setCursor(Qt::ArrowCursor);
    m_menuTools = new QMenu(tr("&Tools"));
    m_menuHelp = new QMenu(tr("&Help"));
    m_menuBookmarks = new Menu(tr("&Bookmarks"));
    m_menuHistory = new Menu(tr("Hi&story"));
    connect(m_menuHistory, SIGNAL(aboutToShow()), this, SLOT(aboutToShowHistoryMenu()));
    connect(m_menuHistory, SIGNAL(aboutToHide()), this, SLOT(aboutToHideHistoryMenu()));
    connect(m_menuBookmarks, SIGNAL(aboutToShow()), this, SLOT(aboutToShowBookmarksMenu()));
    connect(m_menuBookmarks, SIGNAL(menuMiddleClicked(Menu*)), this, SLOT(loadFolderBookmarks(Menu*)));
    connect(m_menuHelp, SIGNAL(aboutToShow()), this, SLOT(aboutToShowHelpMenu()));
    connect(m_menuTools, SIGNAL(aboutToShow()), this, SLOT(aboutToShowToolsMenu()));

    m_menuFile = new QMenu(tr("&File"));
    m_menuFile->addAction(QIcon::fromTheme("window-new"), tr("&New Window"), this, SLOT(newWindow()))->setShortcut(QKeySequence("Ctrl+N"));
    m_menuFile->addAction(QIcon(":/icons/menu/popup.png"), tr("New Tab"), this, SLOT(addTab()))->setShortcut(QKeySequence("Ctrl+T"));
    m_menuFile->addAction(tr("Open Location"), this, SLOT(openLocation()))->setShortcut(QKeySequence("Ctrl+L"));
    m_menuFile->addAction(QIcon::fromTheme("document-open"), tr("Open &File"), this, SLOT(openFile()))->setShortcut(QKeySequence("Ctrl+O"));
    m_menuFile->addAction(tr("Close Tab"), m_tabWidget, SLOT(closeTab()))->setShortcut(QKeySequence("Ctrl+W"));
    m_actionCloseWindow = m_menuFile->addAction(QIcon::fromTheme("window-close"), tr("Close Window"), this, SLOT(close()));
    m_actionCloseWindow->setShortcut(QKeySequence("Ctrl+Shift+W"));
    m_menuFile->addSeparator();
    m_menuFile->addAction(QIcon::fromTheme("document-save"), tr("&Save Page As..."), this, SLOT(savePage()))->setShortcut(QKeySequence("Ctrl+S"));
    m_menuFile->addAction(tr("Save Page Screen"), this, SLOT(savePageScreen()));
    m_menuFile->addAction(tr("Send Link..."), this, SLOT(sendLink()));
    m_menuFile->addAction(QIcon::fromTheme("document-print"), tr("&Print"), this, SLOT(printPage()));
    m_menuFile->addSeparator();
    m_menuFile->addSeparator();
    m_menuFile->addAction(tr("Import bookmarks..."), this, SLOT(showBookmarkImport()));
    m_menuFile->addAction(QIcon::fromTheme("application-exit"), tr("Quit"), this, SLOT(quitApp()))->setShortcut(QKeySequence("Ctrl+Q"));
    menuBar()->addMenu(m_menuFile);
    connect(m_menuFile, SIGNAL(aboutToShow()), this, SLOT(aboutToShowFileMenu()));
    connect(m_menuFile, SIGNAL(aboutToHide()), this, SLOT(aboutToHideFileMenu()));

    m_menuEdit = new QMenu(tr("&Edit"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-undo"), tr("&Undo"))->setShortcut(QKeySequence("Ctrl+Z"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-redo"), tr("&Redo"))->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-cut"), tr("&Cut"))->setShortcut(QKeySequence("Ctrl+X"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-copy"), tr("C&opy"), this, SLOT(copy()))->setShortcut(QKeySequence("Ctrl+C"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-paste"), tr("&Paste"))->setShortcut(QKeySequence("Ctrl+V"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-delete"), tr("&Delete"))->setShortcut(QKeySequence("Del"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-select-all"), tr("Select &All"), this, SLOT(selectAll()))->setShortcut(QKeySequence("Ctrl+A"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-find"), tr("&Find"), this, SLOT(searchOnPage()))->setShortcut(QKeySequence("Ctrl+F"));
    m_menuEdit->addSeparator();
#ifdef Q_WS_X11
    m_menuEdit->addAction(QIcon(":/icons/faenza/settings.png"), tr("Pr&eferences"), this, SLOT(showPreferences()))->setShortcut(QKeySequence("Ctrl+P"));
#endif
    menuBar()->addMenu(m_menuEdit);
    connect(m_menuEdit, SIGNAL(aboutToShow()), this, SLOT(aboutToShowEditMenu()));
    connect(m_menuEdit, SIGNAL(aboutToHide()), this, SLOT(aboutToHideEditMenu()));
    aboutToHideEditMenu();

    m_menuView = new QMenu(tr("&View"));
    m_actionShowToolbar = new QAction(tr("&Navigation Toolbar"), this);
    m_actionShowToolbar->setCheckable(true);
    connect(m_actionShowToolbar, SIGNAL(triggered(bool)), this, SLOT(showNavigationToolbar()));
    m_actionShowBookmarksToolbar = new QAction(tr("&Bookmarks Toolbar"), this);
    m_actionShowBookmarksToolbar->setCheckable(true);
    connect(m_actionShowBookmarksToolbar, SIGNAL(triggered(bool)), this, SLOT(showBookmarksToolbar()));
    m_actionShowStatusbar = new QAction(tr("Sta&tus Bar"), this);
    m_actionShowStatusbar->setCheckable(true);
    connect(m_actionShowStatusbar, SIGNAL(triggered(bool)), this, SLOT(showStatusbar()));
    m_actionShowMenubar = new QAction(tr("&Menu Bar"), this);
    m_actionShowMenubar->setCheckable(true);
    connect(m_actionShowMenubar, SIGNAL(triggered(bool)), this, SLOT(showMenubar()));
    m_actionShowFullScreen = new QAction(tr("&Fullscreen"), this);
    m_actionShowFullScreen->setCheckable(true);
    m_actionShowFullScreen->setShortcut(QKeySequence("F11"));
    connect(m_actionShowFullScreen, SIGNAL(triggered(bool)), this, SLOT(fullScreen(bool)));
    m_actionStop = new QAction(IconProvider::standardIcon(QStyle::SP_BrowserStop), tr("&Stop"), this);
    connect(m_actionStop, SIGNAL(triggered()), this, SLOT(stop()));
    m_actionStop->setShortcut(QKeySequence("Esc"));
    m_actionReload = new QAction(IconProvider::standardIcon(QStyle::SP_BrowserReload), tr("&Reload"), this);
    connect(m_actionReload, SIGNAL(triggered()), this, SLOT(reload()));
    m_actionReload->setShortcut(QKeySequence("F5"));
    QAction* actionEncoding = new QAction(tr("Character &Encoding"), this);
    m_menuEncoding = new QMenu(this);
    actionEncoding->setMenu(m_menuEncoding);
    connect(m_menuEncoding, SIGNAL(aboutToShow()), this, SLOT(aboutToShowEncodingMenu()));

    m_actionShowBookmarksSideBar = new QAction(tr("Bookmarks"), this);
    m_actionShowBookmarksSideBar->setCheckable(true);
    m_actionShowBookmarksSideBar->setShortcut(QKeySequence("Ctrl+B"));
    connect(m_actionShowBookmarksSideBar, SIGNAL(triggered()), this, SLOT(showBookmarksSideBar()));
    m_actionShowHistorySideBar = new QAction(tr("History"), this);
    m_actionShowHistorySideBar->setCheckable(true);
    m_actionShowHistorySideBar->setShortcut(QKeySequence("Ctrl+H"));
    connect(m_actionShowHistorySideBar, SIGNAL(triggered()), this, SLOT(showHistorySideBar()));
//    m_actionShowRssSideBar = new QAction(tr("RSS Reader"), this);
//    m_actionShowRssSideBar->setCheckable(true);
//    connect(m_actionShowRssSideBar, SIGNAL(triggered()), this, SLOT(showRssSideBar()));

    QMenu* toolbarsMenu = new QMenu(tr("Toolbars"));
    toolbarsMenu->addAction(m_actionShowMenubar);
    toolbarsMenu->addAction(m_actionShowToolbar);
    toolbarsMenu->addAction(m_actionShowBookmarksToolbar);
    QMenu* sidebarsMenu = new QMenu(tr("Sidebars"));
    sidebarsMenu->addAction(m_actionShowBookmarksSideBar);
    sidebarsMenu->addAction(m_actionShowHistorySideBar);
//    sidebarsMenu->addAction(m_actionShowRssSideBar);

    m_menuView->addMenu(toolbarsMenu);
    m_menuView->addMenu(sidebarsMenu);
    m_menuView->addAction(m_actionShowStatusbar);
    m_menuView->addSeparator();
    m_menuView->addAction(m_actionStop);
    m_menuView->addAction(m_actionReload);
    m_menuView->addSeparator();
    m_menuView->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom &In"), this, SLOT(zoomIn()))->setShortcut(QKeySequence("Ctrl++"));
    m_menuView->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom &Out"), this, SLOT(zoomOut()))->setShortcut(QKeySequence("Ctrl+-"));
    m_menuView->addAction(QIcon::fromTheme("zoom-original"), tr("Reset"), this, SLOT(zoomReset()))->setShortcut(QKeySequence("Ctrl+0"));
    m_menuView->addSeparator();
    m_menuView->addAction(actionEncoding);
    m_menuView->addSeparator();
    m_menuView->addAction(QIcon::fromTheme("text-html"), tr("&Page Source"), this, SLOT(showSource()))->setShortcut(QKeySequence("Ctrl+U"));
    m_menuView->addAction(m_actionShowFullScreen);
    menuBar()->addMenu(m_menuView);
    connect(m_menuView, SIGNAL(aboutToShow()), this, SLOT(aboutToShowViewMenu()));
    connect(m_menuView, SIGNAL(aboutToHide()), this, SLOT(aboutToHideViewMenu()));

    menuBar()->addMenu(m_menuHistory);
    menuBar()->addMenu(m_menuBookmarks);
    menuBar()->addMenu(m_menuTools);
    menuBar()->addMenu(m_menuHelp);

    menuBar()->setContextMenuPolicy(Qt::CustomContextMenu);

    m_menuClosedTabs = new QMenu(tr("Closed Tabs"));
    connect(m_menuClosedTabs, SIGNAL(aboutToShow()), this, SLOT(aboutToShowClosedTabsMenu()));

    aboutToShowToolsMenu();
    aboutToShowHelpMenu();

    m_actionRestoreTab = new QAction(QIcon::fromTheme("user-trash"), tr("Restore &Closed Tab"), this);
    m_actionRestoreTab->setShortcut(QKeySequence("Ctrl+Shift+T"));
    connect(m_actionRestoreTab, SIGNAL(triggered()), m_tabWidget, SLOT(restoreClosedTab()));
    addAction(m_actionRestoreTab);

    QAction* reloadByPassCacheAction = new QAction(this);
    reloadByPassCacheAction->setShortcut(QKeySequence("Ctrl+F5"));
    connect(reloadByPassCacheAction, SIGNAL(triggered()), this, SLOT(reloadByPassCache()));
    addAction(reloadByPassCacheAction);

    // Make shortcuts available even in fullscreen (menu hidden)
    QList<QAction*> actions = menuBar()->actions();
    foreach(QAction * action, actions) {
        if (action->menu()) {
            actions += action->menu()->actions();
        }
        addAction(action);
    }

    m_superMenu->addMenu(m_menuFile);
    m_superMenu->addMenu(m_menuEdit);
    m_superMenu->addMenu(m_menuView);
    m_superMenu->addMenu(m_menuHistory);
    m_superMenu->addMenu(m_menuBookmarks);
    m_superMenu->addMenu(m_menuTools);
    m_superMenu->addMenu(m_menuHelp);
}

void QupZilla::loadSettings()
{
    QSettings settings(m_activeProfil + "settings.ini", QSettings::IniFormat);

    //Url settings
    settings.beginGroup("Web-URL-Settings");
    m_homepage = settings.value("homepage", "qupzilla:start").toUrl();
    m_newtab = settings.value("newTabUrl", "qupzilla:speeddial").toUrl();
    settings.endGroup();

    QWebSettings* websettings = mApp->webSettings();
    websettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);

    //Browser Window settings
    settings.beginGroup("Browser-View-Settings");
    m_menuTextColor = settings.value("menuTextColor", QColor(Qt::black)).value<QColor>();
    bool showStatusBar = settings.value("showStatusBar", true).toBool();
    bool showHomeIcon = settings.value("showHomeButton", true).toBool();
    bool showBackForwardIcons = settings.value("showBackForwardButtons", true).toBool();
    bool showBookmarksToolbar = settings.value("showBookmarksToolbar", true).toBool();
    bool showNavigationToolbar = settings.value("showNavigationToolbar", true).toBool();
    bool showMenuBar = settings.value("showMenubar", true).toBool();
    bool showAddTab = settings.value("showAddTabButton", false).toBool();
    bool makeTransparent = settings.value("useTransparentBackground", false).toBool();
    m_sideBarWidth = settings.value("SideBarWidth", 250).toInt();
    QString activeSideBar = settings.value("SideBar", "None").toString();
    settings.endGroup();
    bool adBlockEnabled = settings.value("AdBlock/enabled", true).toBool();

    m_adblockIcon->setEnabled(adBlockEnabled);

    statusBar()->setVisible(showStatusBar);
    m_bookmarksToolbar->setVisible(showBookmarksToolbar);
    m_navigationBar->setVisible(showNavigationToolbar);
    menuBar()->setVisible(showMenuBar);

    m_navigationBar->buttonSuperMenu()->setVisible(!showMenuBar);
    m_navigationBar->buttonHome()->setVisible(showHomeIcon);
    m_navigationBar->buttonBack()->setVisible(showBackForwardIcons);
    m_navigationBar->buttonNext()->setVisible(showBackForwardIcons);
    m_navigationBar->buttonAddTab()->setVisible(showAddTab);

    if (activeSideBar != "None") {
        if (activeSideBar == "Bookmarks") {
            m_actionShowBookmarksSideBar->trigger();
        }
        else if (activeSideBar == "History") {
            m_actionShowHistorySideBar->trigger();
        }
    }

    //Private browsing
    m_actionPrivateBrowsing->setChecked(mApp->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled));
    m_privateBrowsing->setVisible(mApp->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled));

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
    if (QtWin::isCompositionEnabled()) {
        QtWin::extendFrameIntoClientArea(this);
        setContentsMargins(0, 0, 0, 0);
    }
}

void QupZilla::setWindowTitle(const QString &t)
{
    if (mApp->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled)) {
        QMainWindow::setWindowTitle(t + tr(" (Private Browsing)"));
    }
    else {
        QMainWindow::setWindowTitle(t);
    }
}

void QupZilla::receiveMessage(MainApplication::MessageType mes, bool state)
{
    switch (mes) {
    case MainApplication::SetAdBlockIconEnabled:
        m_adblockIcon->setEnabled(state);
        break;

    case MainApplication::CheckPrivateBrowsing:
        m_privateBrowsing->setVisible(state);
        m_actionPrivateBrowsing->setChecked(state);
        if (state) {
            setWindowTitle(windowTitle());
        }
        else {
            setWindowTitle(windowTitle().remove(tr(" (Private Browsing)")));
        }
        break;

    case MainApplication::ReloadSettings:
        loadSettings();
        m_tabWidget->loadSettings();
        LocationBarSettings::instance()->loadSettings();
        break;

    case MainApplication::HistoryStateChanged:
        m_historyMenuChanged = true;
        break;

    case MainApplication::BookmarksChanged:
        m_bookmarksMenuChanged = true;
        break;

    default:
        qWarning("Unresolved message sent! This could never happen!");
        break;
    }
}

void QupZilla::aboutToShowFileMenu()
{
    m_actionCloseWindow->setEnabled(mApp->windowCount() > 1);
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

    m_menuBookmarks->clear();
    m_menuBookmarks->addAction(tr("Bookmark &This Page"), this, SLOT(bookmarkPage()))->setShortcut(QKeySequence("Ctrl+D"));
    m_menuBookmarks->addAction(tr("Bookmark &All Tabs"), this, SLOT(bookmarkAllTabs()));
    m_menuBookmarks->addAction(IconProvider::fromTheme("user-bookmarks"), tr("Organize &Bookmarks"), this, SLOT(showBookmarksManager()))->setShortcut(QKeySequence("Ctrl+Shift+O"));
    m_menuBookmarks->addSeparator();
    QSqlQuery query;
    query.exec("SELECT title, url, icon FROM bookmarks WHERE folder='bookmarksMenu'");
    while (query.next()) {
        QString title = query.value(0).toString();
        QUrl url = query.value(1).toUrl();
        QIcon icon = IconProvider::iconFromBase64(query.value(2).toByteArray());
        if (title.length() > 40) {
            title.truncate(40);
            title += "..";
        }

        Action* act = new Action(icon, title);
        act->setData(url);
        connect(act, SIGNAL(triggered()), this, SLOT(loadActionUrl()));
        connect(act, SIGNAL(middleClicked()), this, SLOT(loadActionUrlInNewNotSelectedTab()));
        m_menuBookmarks->addAction(act);
    }

    Menu* menuBookmarks = new Menu(_bookmarksToolbar, m_menuBookmarks);
    menuBookmarks->setIcon(QIcon(style()->standardIcon(QStyle::SP_DirOpenIcon)));

    query.exec("SELECT title, url, icon FROM bookmarks WHERE folder='bookmarksToolbar'");
    while (query.next()) {
        QString title = query.value(0).toString();
        QUrl url = query.value(1).toUrl();
        QIcon icon = IconProvider::iconFromBase64(query.value(2).toByteArray());
        if (title.length() > 40) {
            title.truncate(40);
            title += "..";
        }

        Action* act = new Action(icon, title);
        act->setData(url);
        connect(act, SIGNAL(triggered()), this, SLOT(loadActionUrl()));
        connect(act, SIGNAL(middleClicked()), this, SLOT(loadActionUrlInNewNotSelectedTab()));
        menuBookmarks->addAction(act);
    }
    if (menuBookmarks->isEmpty()) {
        menuBookmarks->addAction(tr("Empty"));
    }
    m_menuBookmarksAction = m_menuBookmarks->addMenu(menuBookmarks);

    query.exec("SELECT name FROM folders");
    while (query.next()) {
        QString folderName = query.value(0).toString();
        Menu* tempFolder = new Menu(folderName, m_menuBookmarks);
        tempFolder->setIcon(QIcon(style()->standardIcon(QStyle::SP_DirOpenIcon)));

        QSqlQuery query2;
        query2.exec("SELECT title, url, icon FROM bookmarks WHERE folder='" + folderName + "'");
        while (query2.next()) {
            QString title = query2.value(0).toString();
            QUrl url = query2.value(1).toUrl();
            QIcon icon = IconProvider::iconFromBase64(query2.value(2).toByteArray());
            if (title.length() > 40) {
                title.truncate(40);
                title += "..";
            }

            Action* act = new Action(icon, title);
            act->setData(url);
            connect(act, SIGNAL(triggered()), this, SLOT(loadActionUrl()));
            connect(act, SIGNAL(middleClicked()), this, SLOT(loadActionUrlInNewNotSelectedTab()));
            tempFolder->addAction(act);
        }
        if (tempFolder->isEmpty()) {
            tempFolder->addAction(tr("Empty"));
        }
        m_menuBookmarks->addMenu(tempFolder);
    }

    m_menuBookmarksAction->setVisible(m_bookmarksToolbar->isVisible());
}

void QupZilla::aboutToShowHistoryMenu(bool loadHistory)
{
    if (!weView()) {
        return;
    }

    if (!m_historyMenuChanged) {
        if (!m_menuHistory || m_menuHistory->actions().count() < 3) {
            return;
        }

        m_menuHistory->actions().at(0)->setEnabled(WebHistoryWrapper::canGoBack(weView()->history()));
        m_menuHistory->actions().at(1)->setEnabled(WebHistoryWrapper::canGoForward(weView()->history()));
        return;
    }
    m_historyMenuChanged = false;
    if (!loadHistory) {
        m_historyMenuChanged = true;
    }

    m_menuHistory->clear();
    m_menuHistory->addAction(IconProvider::standardIcon(QStyle::SP_ArrowBack), tr("&Back"), this, SLOT(goBack()))->setShortcut(QKeySequence("Ctrl+Left"));
    m_menuHistory->addAction(IconProvider::standardIcon(QStyle::SP_ArrowForward), tr("&Forward"), this, SLOT(goNext()))->setShortcut(QKeySequence("Ctrl+Right"));
    m_menuHistory->addAction(IconProvider::fromTheme("go-home"), tr("&Home"), this, SLOT(goHome()))->setShortcut(QKeySequence("Alt+Home"));

    m_menuHistory->actions().at(0)->setEnabled(WebHistoryWrapper::canGoBack(weView()->history()));
    m_menuHistory->actions().at(1)->setEnabled(WebHistoryWrapper::canGoForward(weView()->history()));

    m_menuHistory->addAction(QIcon(":/icons/menu/history.png"), tr("Show &All History"), this, SLOT(showHistoryManager()))->setShortcut(QKeySequence("Ctrl+Shift+H"));
    m_menuHistory->addSeparator();

    if (loadHistory) {
        QSqlQuery query;
        query.exec("SELECT title, url FROM history ORDER BY date DESC LIMIT 10");
        while (query.next()) {
            QUrl url = query.value(1).toUrl();
            QString title = query.value(0).toString();
            if (title.length() > 40) {
                title.truncate(40);
                title += "..";
            }

            Action* act = new Action(_iconForUrl(url), title);
            act->setData(url);
            connect(act, SIGNAL(triggered()), this, SLOT(loadActionUrl()));
            connect(act, SIGNAL(middleClicked()), this, SLOT(loadActionUrlInNewNotSelectedTab()));
            m_menuHistory->addAction(act);
        }
        m_menuHistory->addSeparator();
    }
    m_menuHistory->addMenu(m_menuClosedTabs);
}

void QupZilla::aboutToHideHistoryMenu()
{
    if (!m_menuHistory || m_menuHistory->actions().count() < 3) {
        return;
    }

    m_menuHistory->actions().at(0)->setEnabled(true);
    m_menuHistory->actions().at(1)->setEnabled(true);
}

void QupZilla::aboutToShowClosedTabsMenu()
{
    m_menuClosedTabs->clear();
    int i = 0;
    foreach(ClosedTabsManager::Tab tab, m_tabWidget->closedTabsManager()->allClosedTabs()) {
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

void QupZilla::aboutToShowHelpMenu()
{
    m_menuHelp->clear();
    mApp->plugins()->populateHelpMenu(m_menuHelp);
    m_menuHelp->addSeparator();
    m_menuHelp->addAction(QIcon(":/icons/menu/qt.png"), tr("About &Qt"), qApp, SLOT(aboutQt()));
    m_menuHelp->addAction(QIcon(":/icons/qupzilla.png"), tr("&About QupZilla"), this, SLOT(aboutQupZilla()));
    m_menuHelp->addSeparator();
    QAction* infoAction = new QAction(QIcon(":/icons/menu/informations.png"), tr("Informations about application"), m_menuHelp);
    infoAction->setData(QUrl("qupzilla:about"));
    infoAction->setShortcut(QKeySequence(QKeySequence::HelpContents));
    connect(infoAction, SIGNAL(triggered()), this, SLOT(loadActionUrlInNewTab()));
    m_menuHelp->addAction(infoAction);
    m_menuHelp->addAction(tr("Report &Issue"), this, SLOT(loadActionUrlInNewTab()))->setData(QUrl("qupzilla:reportbug"));
}

void QupZilla::aboutToShowToolsMenu()
{
    m_menuTools->clear();
    m_menuTools->addAction(tr("&Web Search"), this, SLOT(webSearch()))->setShortcut(QKeySequence("Ctrl+K"));
    m_menuTools->addAction(QIcon::fromTheme("dialog-information"), tr("Page &Info"), this, SLOT(showPageInfo()))->setShortcut(QKeySequence("Ctrl+I"));
    m_menuTools->addSeparator();
    m_menuTools->addAction(tr("&Download Manager"), this, SLOT(showDownloadManager()))->setShortcut(QKeySequence("Ctrl+Y"));
    m_menuTools->addAction(tr("&Cookies Manager"), this, SLOT(showCookieManager()));
    m_menuTools->addAction(tr("&AdBlock"), AdBlockManager::instance(), SLOT(showDialog()));
    m_menuTools->addAction(QIcon(":/icons/menu/rss.png"), tr("RSS &Reader"), this,  SLOT(showRSSManager()));
    m_menuTools->addAction(QIcon::fromTheme("edit-clear"), tr("Clear Recent &History"), this, SLOT(showClearPrivateData()));
    m_actionPrivateBrowsing = new QAction(tr("&Private Browsing"), this);
    m_actionPrivateBrowsing->setShortcut(QKeySequence("Ctrl+Shift+P"));
    m_actionPrivateBrowsing->setCheckable(true);
    m_actionPrivateBrowsing->setChecked(mApp->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled));
    connect(m_actionPrivateBrowsing, SIGNAL(triggered(bool)), this, SLOT(startPrivate(bool)));
    m_menuTools->addAction(m_actionPrivateBrowsing);
    m_menuTools->addSeparator();
    mApp->plugins()->populateToolsMenu(m_menuTools);
#ifdef Q_WS_WIN
    m_menuTools->addAction(QIcon(":/icons/faenza/settings.png"), tr("Pr&eferences"), this, SLOT(showPreferences()))->setShortcut(QKeySequence("Ctrl+P"));
#endif
}

void QupZilla::aboutToShowViewMenu()
{
    if (!weView()) {
        return;
    }

    if (weView()->isLoading()) {
        m_actionStop->setEnabled(true);
        m_actionReload->setEnabled(false);
    }
    else {
        m_actionStop->setEnabled(false);
        m_actionReload->setEnabled(true);
    }

    m_actionShowToolbar->setChecked(m_navigationBar->isVisible());
    m_actionShowMenubar->setChecked(menuBar()->isVisible());
    m_actionShowStatusbar->setChecked(statusBar()->isVisible());
    m_actionShowBookmarksToolbar->setChecked(m_bookmarksToolbar->isVisible());

    if (!m_sideBar.data()) {
        m_actionShowBookmarksSideBar->setChecked(false);
        m_actionShowHistorySideBar->setChecked(false);
//        m_actionShowRssSideBar->setChecked(false);
    }
    else {
        SideBar::SideWidget actWidget = m_sideBar.data()->activeWidget();
        m_actionShowBookmarksSideBar->setChecked(actWidget == SideBar::Bookmarks);
        m_actionShowHistorySideBar->setChecked(actWidget == SideBar::History);
//        m_actionShowRssSideBar->setChecked(actWidget == SideBar::RSS);
    }
}

void QupZilla::aboutToHideViewMenu()
{
    m_actionReload->setEnabled(true);
    m_actionStop->setEnabled(true);

    if (m_mainLayout->count() == 4) {
        SearchToolBar* search = qobject_cast<SearchToolBar*>(m_mainLayout->itemAt(3)->widget());
        if (!search) {
            return;
        }
        m_actionStop->setEnabled(false);
    }
}

void QupZilla::aboutToShowEditMenu()
{
    foreach(QAction * act, m_menuEdit->actions()) {
        act->setEnabled(true);
    }
}

void QupZilla::aboutToHideEditMenu()
{
    foreach(QAction * act, m_menuEdit->actions()) {
        act->setEnabled(false);
    }

    m_menuEdit->actions().at(9)->setEnabled(true);
#ifdef Q_WS_X11
    m_menuEdit->actions().at(11)->setEnabled(true);
#endif
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
    QString activeCodec = mApp->webSettings()->defaultTextEncoding();

    foreach(QByteArray name, available) {
        if (QTextCodec::codecForName(name)->aliases().contains(name)) {
            continue;
        }
        QAction* action = new QAction(name == "System" ? tr("Default") : name, this);
        action->setData(name);
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), this, SLOT(changeEncoding()));
        if (activeCodec.compare(name, Qt::CaseInsensitive) == 0) {
            action->setChecked(true);
        }

        if (name.startsWith("ISO")) {
            menuISO->addAction(action);
        }
        else if (name.startsWith("UTF")) {
            menuUTF->addAction(action);
        }
        else if (name.startsWith("windows")) {
            menuWindows->addAction(action);
        }
        else if (name.startsWith("Iscii")) {
            menuIscii->addAction(action);
        }
        else if (name == "System") {
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
        mApp->webSettings()->setDefaultTextEncoding(action->data().toString());
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

void QupZilla::goHome()
{
    loadAddress(m_homepage);
}

void QupZilla::copy()
{
    if (!weView()->selectedText().isEmpty()) {
        QApplication::clipboard()->setText(weView()->selectedText());
    }
}

void QupZilla::goHomeInNewTab()
{
    m_tabWidget->addView(m_homepage, tr("New tab"), TabWidget::NewSelectedTab);
}

void QupZilla::loadActionUrl()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        loadAddress(action->data().toUrl());
    }
}

void QupZilla::loadActionUrlInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        m_tabWidget->addView(action->data().toUrl());
    }
}

void QupZilla::loadActionUrlInNewNotSelectedTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        m_tabWidget->addView(action->data().toUrl(), tr("New tab"), TabWidget::NewNotSelectedTab);
    }
}

void QupZilla::loadFolderBookmarks(Menu *menu)
{
    QString folder = BookmarksModel::fromTranslatedFolder(menu->title());
    if (folder.isEmpty()) {
        return;
    }

    foreach(Bookmark b, mApp->bookmarksModel()->folderBookmarks(folder)) {
        tabWidget()->addView(b.url, b.title, TabWidget::NewNotSelectedTab);
    }
}

void QupZilla::loadAddress(const QUrl &url)
{
    weView()->load(url);
    locationBar()->setText(url.toEncoded());
}

void QupZilla::urlEnter()
{
    if (locationBar()->text().isEmpty()) {
        return;
    }
    loadAddress(QUrl(WebView::guessUrlFromString(locationBar()->text())));
    weView()->setFocus();
}

void QupZilla::showCookieManager()
{
    CookieManager* m = mApp->cookieManager();
    m->refreshTable();
    m->show();
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
    Preferences* prefs = new Preferences(this, this);
    prefs->show();
}

void QupZilla::showSource(const QString &selectedHtml)
{
    SourceViewer* source = new SourceViewer(weView()->page(), selectedHtml);
    qz_centerWidgetToParent(source, this);
    source->show();
}

void QupZilla::showPageInfo()
{
    SiteInfo* info = new SiteInfo(this, this);
    info->setAttribute(Qt::WA_DeleteOnClose);
    info->show();
}

void QupZilla::showBookmarksToolbar()
{
    bool status = m_bookmarksToolbar->isVisible();
    m_bookmarksToolbar->setVisible(!status);

    QSettings settings(activeProfil() + "settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/showBookmarksToolbar", !status);
}

void QupZilla::showBookmarksSideBar()
{
    addSideBar();

    if (m_sideBar.data()->activeWidget() != SideBar::Bookmarks) {
        m_sideBar.data()->showBookmarks();
    }
    else {
        m_sideBar.data()->close();
    }
}

void QupZilla::showHistorySideBar()
{
    addSideBar();

    if (m_sideBar.data()->activeWidget() != SideBar::History) {
        m_sideBar.data()->showHistory();
    }
    else {
        m_sideBar.data()->close();
    }
}

void QupZilla::addSideBar()
{
    if (m_sideBar.data()) {
        return;
    }

    m_sideBar = new SideBar(this);

    m_mainSplitter->insertWidget(0, m_sideBar.data());
    m_mainSplitter->setCollapsible(0, false);

    QList<int> sizes;
    sizes << m_sideBarWidth << width() - m_sideBarWidth;
    m_mainSplitter->setSizes(sizes);
}

void QupZilla::saveSideBarWidth()
{
    // That +1 is important here, without it, the sidebar width would
    // decrease by 1 pixel every close
    m_sideBarWidth = m_mainSplitter->sizes().at(0) + 1;
}

void QupZilla::showNavigationToolbar()
{
    if (!menuBar()->isVisible() && !m_actionShowToolbar->isChecked()) {
        showMenubar();
    }

    bool status = m_navigationBar->isVisible();
    m_navigationBar->setVisible(!status);

    QSettings settings(activeProfil() + "settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/showNavigationToolbar", !status);
}

void QupZilla::showMenubar()
{
    if (!m_navigationBar->isVisible() && !m_actionShowMenubar->isChecked()) {
        showNavigationToolbar();
    }

    menuBar()->setVisible(!menuBar()->isVisible());
    m_navigationBar->buttonSuperMenu()->setVisible(!menuBar()->isVisible());

    QSettings settings(activeProfil() + "settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/showMenubar", menuBar()->isVisible());
}

void QupZilla::showStatusbar()
{
    bool status = statusBar()->isVisible();
    statusBar()->setVisible(!status);

    QSettings settings(activeProfil() + "settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/showStatusbar", !status);
}

void QupZilla::showWebInspector()
{
#ifdef Q_WS_WIN
    weView()->triggerPageAction(QWebPage::InspectElement);
#else
    if (m_webInspectorDock.data()) {
        m_webInspectorDock.data()->setPage(weView()->webPage());
        m_webInspectorDock.data()->show();
        return;
    }

    m_webInspectorDock = new WebInspectorDockWidget(this);
    connect(m_tabWidget, SIGNAL(currentChanged(int)), m_webInspectorDock.data(), SLOT(tabChanged()));
    addDockWidget(Qt::BottomDockWidgetArea, m_webInspectorDock.data());
#endif
}

void QupZilla::showBookmarkImport()
{
    BookmarksImportDialog* b = new BookmarksImportDialog(this);
    b->show();
}

void QupZilla::refreshHistory()
{
    m_navigationBar->refreshHistory();
}

void QupZilla::aboutQupZilla()
{
    AboutDialog about(this);
    about.exec();
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

        search->searchLine()->setFocus();
        return;
    }

    SearchToolBar* search = new SearchToolBar(this);
    m_mainLayout->insertWidget(3, search);
    search->searchLine()->setFocus();
}

void QupZilla::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open file..."), QDir::homePath(), "(*.html *.htm *.jpg *.png)");
    if (!filePath.isEmpty()) {
        loadAddress(QUrl::fromLocalFile(filePath));
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
    }
    else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);

        menuBar()->setVisible(m_menuBarVisible);
        statusBar()->setVisible(m_statusBarVisible);
        m_bookmarksToolbar->setVisible(m_bookmarksToolBarVisible);
        m_navigationBar->setVisible(m_navigationVisible);
    }

    m_actionShowFullScreen->setChecked(make);
    m_navigationBar->buttonExitFullscreen()->setVisible(make);
    m_tabWidget->getTabBar()->setVisible(!make);

    emit setWebViewMouseTracking(make);
}

void QupZilla::savePage()
{
    QNetworkRequest request(weView()->url());

    DownloadManager* dManager = mApp->downManager();
    dManager->download(request, weView()->webPage(), false);
}

void QupZilla::sendLink()
{
    QUrl url = QUrl("mailto:?body=" + weView()->url().toString());
    QDesktopServices::openUrl(url);
}

void QupZilla::printPage()
{
    QPrintPreviewDialog* dialog = new QPrintPreviewDialog(this);
    connect(dialog, SIGNAL(paintRequested(QPrinter*)), weView(), SLOT(print(QPrinter*)));
    dialog->exec();

    dialog->deleteLater();
}

void QupZilla::savePageScreen()
{
    PageScreen* p = new PageScreen(weView(), this);
    p->show();
}

void QupZilla::startPrivate(bool state)
{
    QSettings settings(m_activeProfil + "settings.ini", QSettings::IniFormat);
    bool askNow = settings.value("Browser-View-Settings/AskOnPrivate", true).toBool();

    if (state && askNow) {
        QString title = tr("Are you sure you want to turn on private browsing?");
        QString text1 = tr("When private browsing is turned on, some actions concerning your privacy will be disabled:");

        QStringList actions;
        actions.append(tr("Webpages are not added to the history."));
        actions.append(tr("Current cookies cannot be accessed."));
        actions.append(tr("Your session is not stored."));

        QString text2 = tr("Until you close the window, you can still click the Back and Forward "
                           "buttons to return to the webpages you have opened.");

        QString message = QString(QLatin1String("<b>%1</b><p>%2</p><ul><li>%3</li></ul><p>%4</p>")).arg(title, text1, actions.join(QLatin1String("</li><li>")), text2);

        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Start Private Browsing"),
                                             message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (button != QMessageBox::Yes) {
            return;
        }

        settings.setValue("Browser-View-Settings/AskOnPrivate", false);
    }

    mApp->webSettings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, state);
    mApp->history()->setSaving(!state);
    mApp->cookieJar()->turnPrivateJar(state);
    emit message(MainApplication::CheckPrivateBrowsing, state);
}

void QupZilla::keyPressEvent(QKeyEvent* event)
{
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
            m_tabWidget->createKeyPressEvent(event);
        }
        break;
    case Qt::Key_Backtab:
        if (event->modifiers() == (Qt::ControlModifier + Qt::ShiftModifier)) {
            m_tabWidget->createKeyPressEvent(event);
        }
        break;

    default:
        QMainWindow::keyPressEvent(event);
        return;
    }
}

void QupZilla::mousePressEvent(QMouseEvent* event)
{
    switch (event->button()) {
    case Qt::XButton1:
        weView()->back();
        break;
    case Qt::XButton2:
        weView()->forward();
        break;

    default:
        QMainWindow::mousePressEvent(event);
        break;
    }
}

void QupZilla::closeEvent(QCloseEvent* event)
{
    if (mApp->isClosing()) {
        return;
    }

    m_isClosing = true;
    mApp->saveStateSlot();
    mApp->aboutToCloseWindow(this);

    if (mApp->windowCount() == 0) {
        quitApp() ? event->accept() : event->ignore();
        return;
    }

    event->accept();
}

bool QupZilla::quitApp()
{
    if (m_sideBar.data()) {
        saveSideBarWidth();
    }

    QSettings settings(m_activeProfil + "settings.ini", QSettings::IniFormat);
    int afterLaunch = settings.value("Web-URL-Settings/afterLaunch", 1).toInt();
    bool askOnClose = settings.value("Browser-Tabs-Settings/AskOnClosing", false).toBool();

    settings.beginGroup("Browser-View-Settings");
    settings.setValue("WindowMaximised", windowState().testFlag(Qt::WindowMaximized));
    settings.setValue("WindowGeometry", geometry());
    settings.setValue("LocationBarWidth", m_navigationBar->splitter()->sizes().at(0));
    settings.setValue("WebSearchBarWidth", m_navigationBar->splitter()->sizes().at(1));
    settings.setValue("SideBarWidth", m_sideBar.data() ? m_mainSplitter->sizes().at(0) : m_sideBarWidth);

    if (askOnClose && afterLaunch != 3 && m_tabWidget->count() > 1) {
        QDialog* dialog = new QDialog(this);
        Ui_CloseDialog* ui = new Ui_CloseDialog();
        ui->setupUi(dialog);
        ui->textLabel->setText(tr("There are still %1 open tabs and your session won't be stored. Are you sure to quit QupZilla?").arg(m_tabWidget->count()));
        ui->iconLabel->setPixmap(style()->standardPixmap(QStyle::SP_MessageBoxWarning));
        if (dialog->exec() != QDialog::Accepted) {
            return false;
        }
        if (ui->dontAskAgain->isChecked()) {
            settings.setValue("Browser-Tabs-Settings/AskOnClosing", false);
        }
    }

    mApp->quitApplication();
    return true;
}

QupZilla::~QupZilla()
{
}
