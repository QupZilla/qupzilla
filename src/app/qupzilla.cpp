/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "historymanager.h"
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

const QString QupZilla::VERSION = "1.0.0-b2";
//const QString QupZilla::BUILDTIME = QLocale(QLocale::English).toDateTime(__DATE__" "__TIME__, "MMM d yyyy hh:mm:ss").toString("MM/dd/yyyy hh:ss");
const QString QupZilla::BUILDTIME =  __DATE__" "__TIME__;
const QString QupZilla::AUTHOR = "nowrep";
const QString QupZilla::COPYRIGHT = "2010-2011";
const QString QupZilla::WWWADDRESS = "http://qupzilla.ic.cz";
const QString QupZilla::WEBKITVERSION = qWebKitVersion();

QupZilla::QupZilla(bool tryRestore, QUrl startUrl) :
    QMainWindow(0)
    ,m_tryRestore(tryRestore)
    ,m_startingUrl(startUrl)
    ,m_actionPrivateBrowsing(0)
    ,m_webInspectorDock(0)
    ,m_webSearchToolbar(0)
    ,m_sideBar(0)
    ,m_statusBarMessage(new StatusBarMessage(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    this->resize(640,480);
    this->setWindowState(Qt::WindowMaximized);
    this->setWindowTitle("QupZilla");
    setUpdatesEnabled(false);

    m_activeProfil = mApp->getActiveProfil();
    m_activeLanguage = mApp->getActiveLanguage();

    QDesktopServices::setUrlHandler("http", this, "loadAddress");

    setupUi();
    setupMenu();
    QTimer::singleShot(0, this, SLOT(postLaunch()));
    connect(mApp, SIGNAL(message(MainApplication::MessageType,bool)), this, SLOT(receiveMessage(MainApplication::MessageType,bool)));
}

void QupZilla::postLaunch()
{
    loadSettings();
    m_tabWidget->restorePinnedTabs();

    //Open tab from command line argument
    bool addTab = true;
    QStringList arguments = qApp->arguments();
    for (int i = 0;i<qApp->arguments().count();i++) {
        QString arg = arguments.at(i);
        if (arg.startsWith("-url=")) {
            m_tabWidget->addView(QUrl(arg.replace("-url=","")));
            addTab = false;
        }
    }

    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-URL-Settings");
    int afterLaunch = settings.value("afterLaunch",1).toInt();
    settings.endGroup();
    settings.beginGroup("SessionRestore");
    bool startingAfterCrash = settings.value("isCrashed",false).toBool();
    settings.endGroup();

    QUrl startUrl;
    if (m_tryRestore) {
        if (afterLaunch == 0)
            startUrl = QUrl("");
        else if (afterLaunch == 1)
            startUrl = m_homepage;
        else
            startUrl = m_homepage;

        if ( startingAfterCrash || (addTab && afterLaunch == 2) )
            addTab = !mApp->restoreStateSlot(this);
    } else
        startUrl = m_homepage;

    if (!m_startingUrl.isEmpty()) {
        startUrl = WebView::guessUrlFromString(m_startingUrl.toString());
        addTab = true;
    }

    if (addTab)
        m_tabWidget->addView(startUrl);

    aboutToShowHistoryMenu();
    aboutToShowBookmarksMenu();

    if (m_tabWidget->count() == 0) //Something went really wrong .. add one tab
        m_tabWidget->addView(m_homepage);

    setUpdatesEnabled(true);
    emit startingCompleted();
}

void QupZilla::setupUi()
{
    setContentsMargins(0,0,0,0);

    m_navigation = new QToolBar(this);
    m_navigation->setWindowTitle(tr("Navigation"));
    m_navigation->setObjectName("Navigation bar");
    addToolBar(m_navigation);
    m_navigation->setMovable(false);
    m_navigation->setStyleSheet("QToolBar{background-image:url(:icons/transp.png); border:none;}");

    m_buttonBack = new QAction(QIcon(":/icons/navigation/zpet.png"),tr("Back"),this);
    m_buttonBack->setEnabled(false);
    m_buttonNext = new QAction(QIcon(":/icons/navigation/vpred.png"),tr("Forward"),this);
    m_buttonNext->setEnabled(false);
    m_buttonStop = new QAction(QIcon(":/icons/navigation/stop.png"),tr("Stop"),this);
    m_buttonReload = new QAction(QIcon(":/icons/navigation/reload.png"),tr("Reload"),this);
    m_buttonReload->setShortcut(QKeySequence("F5"));
    m_buttonHome = new QAction(QIcon(":/icons/navigation/home.png"),tr("Home"),this);

    m_menuBack = new QMenu();
    m_buttonBack->setMenu(m_menuBack);
    connect(m_menuBack, SIGNAL(aboutToShow()),this, SLOT(aboutToShowHistoryBackMenu()));

    m_menuForward = new QMenu();
    m_buttonNext->setMenu(m_menuForward);
    connect(m_menuForward, SIGNAL(aboutToShow()),this, SLOT(aboutToShowHistoryNextMenu()));

    m_supMenu = new QToolButton(this);
    m_supMenu->setPopupMode(QToolButton::InstantPopup);
    m_supMenu->setIcon(QIcon(":/icons/qupzilla.png"));
    m_supMenu->setToolTip(tr("Main Menu"));
    m_superMenu = new QMenu(this);
    m_supMenu->setMenu(m_superMenu);

    m_navigation->addAction(m_buttonBack);
    m_navigation->addAction(m_buttonNext);
    m_navigation->addAction(m_buttonReload);
    m_navigation->addAction(m_buttonStop);
    m_navigation->addAction(m_buttonHome);

    m_locationBar = new LocationBar(this);
    m_searchLine = new WebSearchBar(this);

    m_navigationSplitter = new QSplitter(m_navigation);
    m_navigationSplitter->addWidget(m_locationBar);
    m_navigationSplitter->addWidget(m_searchLine);

    m_navigationSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    m_navigationSplitter->setCollapsible(0, false);

    m_navigation->addWidget(m_navigationSplitter);
    int splitterWidth = m_navigationSplitter->width();
    QList<int> sizes;
    sizes << (int)((double)splitterWidth * .75) << (int)((double)splitterWidth * .25);
    m_navigationSplitter->setSizes(sizes);

    m_actionExitFullscreen = new QAction(tr("Exit Fullscreen"),this);
    m_actionExitFullscreen->setVisible(false);
    QWidget* _spacer = new QWidget();
    _spacer->setMinimumWidth(4);
    m_navigation->addWidget(_spacer); //Elegant spacer -,-
    m_navigation->addAction(m_actionExitFullscreen);
    m_navigation->addWidget(m_supMenu);
    m_navigation->setContextMenuPolicy(Qt::CustomContextMenu);

    m_progressBar = new ProgressBar(statusBar());
    m_privateBrowsing = new QLabel(this);
    m_privateBrowsing->setPixmap(QPixmap(":/icons/locationbar/privatebrowsing.png"));
    m_privateBrowsing->setVisible(false);
    m_privateBrowsing->setToolTip(tr("Private Browsing Enabled"));
    m_adblockIcon = new AdBlockIcon(this);
    m_ipLabel = new QLabel(this);
    m_ipLabel->setStyleSheet("padding-right: 5px;");
    m_ipLabel->setToolTip(tr("IP Address of current page"));

    statusBar()->insertPermanentWidget(0, m_progressBar);
    statusBar()->insertPermanentWidget(1, m_ipLabel);
    statusBar()->insertPermanentWidget(2, m_privateBrowsing);
    statusBar()->insertPermanentWidget(3, m_adblockIcon);

    m_bookmarksToolbar = new BookmarksToolbar(this);
    addToolBar(m_bookmarksToolbar);
    insertToolBarBreak(m_bookmarksToolbar);

    m_tabWidget = new TabWidget(this);
    setCentralWidget(m_tabWidget);
}

void QupZilla::setupMenu()
{
    m_menuTools = new QMenu(tr("Tools"));
    m_menuHelp = new QMenu(tr("Help"));
    m_menuBookmarks = new QMenu(tr("Bookmarks"));
    m_menuHistory = new QMenu(tr("History"));
    connect(m_menuHistory, SIGNAL(aboutToShow()), this, SLOT(aboutToShowHistoryMenu()));
    connect(m_menuBookmarks, SIGNAL(aboutToShow()), this, SLOT(aboutToShowBookmarksMenu()));
    connect(m_menuHelp, SIGNAL(aboutToShow()), this, SLOT(aboutToShowHelpMenu()));
    connect(m_menuTools, SIGNAL(aboutToShow()), this, SLOT(aboutToShowToolsMenu()));

    m_menuFile = new QMenu(tr("File"));
    m_menuFile->addAction(QIcon::fromTheme("window-new"), tr("&New Window"), this, SLOT(newWindow()))->setShortcut(QKeySequence("Ctrl+N"));
    m_menuFile->addAction(QIcon(":/icons/menu/popup.png"), tr("New Tab"), this, SLOT(addTab()))->setShortcut(QKeySequence("Ctrl+T"));
    m_menuFile->addAction(tr("Open Location"), this, SLOT(openLocation()))->setShortcut(QKeySequence("Ctrl+L"));
    m_menuFile->addAction(QIcon::fromTheme("document-open"), tr("Open &File"), this, SLOT(openFile()))->setShortcut(QKeySequence("Ctrl+O"));
    m_menuFile->addAction(tr("Close Tab"), m_tabWidget, SLOT(closeTab()))->setShortcut(QKeySequence("Ctrl+W"));
    m_menuFile->addAction(QIcon::fromTheme("window-close"), tr("Close Window"), this, SLOT(close()))->setShortcut(QKeySequence("Ctrl+Shift+W"));
    m_menuFile->addSeparator();
    m_menuFile->addAction(QIcon::fromTheme("document-save"), tr("&Save Page As..."), this, SLOT(savePage()))->setShortcut(QKeySequence("Ctrl+S"));
    m_menuFile->addAction(tr("Send Link..."), this, SLOT(sendLink()));
    m_menuFile->addAction(QIcon::fromTheme("document-print"), tr("&Print"), this, SLOT(printPage()));
    m_menuFile->addSeparator();
    m_menuFile->addAction(QIcon::fromTheme("application-exit"), tr("Quit"), this, SLOT(quitApp()))->setShortcut(QKeySequence("Ctrl+Q"));
    menuBar()->addMenu(m_menuFile);

    m_menuEdit = new QMenu(tr("Edit"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-undo"), tr("&Undo"))->setShortcut(QKeySequence("Ctrl+Z"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-redo"), tr("&Redo"))->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-cut"), tr("&Cut"))->setShortcut(QKeySequence("Ctrl+X"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-copy"), tr("C&opy"), this, SLOT(copy()))->setShortcut(QKeySequence("Ctrl+C"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-paste"), tr("&Paste"))->setShortcut(QKeySequence("Ctrl+V"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-delete"), tr("&Delete"))->setShortcut(QKeySequence("Del"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-select-all"), tr("Select &All"), this, SLOT(selectAll()))->setShortcut(QKeySequence("Ctrl+A"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-find"), tr("&Find"), this, SLOT(searchOnPage()))->setShortcut(QKeySequence("Ctrl+F"));
    menuBar()->addMenu(m_menuEdit);

    m_menuView = new QMenu(tr("View"));
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
    m_actionStop = new QAction(
#ifdef Q_WS_X11
            style()->standardIcon(QStyle::SP_BrowserStop)
#else
            QIcon(":/icons/faenza/stop.png")
#endif
            , tr("&Stop"), this);
    connect(m_actionStop, SIGNAL(triggered()), this, SLOT(stop()));
    m_actionStop->setShortcut(QKeySequence("Esc"));
    m_actionReload = new QAction(
#ifdef Q_WS_X11
            style()->standardIcon(QStyle::SP_BrowserReload)
#else
            QIcon(":/icons/faenza/reload.png")
#endif
            , tr("&Reload"), this);
    connect(m_actionReload, SIGNAL(triggered()), this, SLOT(reload()));
    m_actionReload->setShortcut(QKeySequence("Ctrl+R"));
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
    toolbarsMenu->addAction(m_actionShowStatusbar);
    connect(toolbarsMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowToolbarsMenu()));
    QMenu* sidebarsMenu = new QMenu(tr("Sidebars"));
    sidebarsMenu->addAction(m_actionShowBookmarksSideBar);
    sidebarsMenu->addAction(m_actionShowHistorySideBar);
//    sidebarsMenu->addAction(m_actionShowRssSideBar);
    connect(sidebarsMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowSidebarsMenu()));

    m_menuView->addMenu(toolbarsMenu);
    m_menuView->addMenu(sidebarsMenu);
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

    menuBar()->addMenu(m_menuHistory);
    menuBar()->addMenu(m_menuBookmarks);
    menuBar()->addMenu(m_menuTools);
    menuBar()->addMenu(m_menuHelp);

    menuBar()->setContextMenuPolicy(Qt::CustomContextMenu);

    aboutToShowToolsMenu();
    aboutToShowHelpMenu();

    connect(m_locationBar, SIGNAL(returnPressed()), this, SLOT(urlEnter()));
    connect(m_buttonBack, SIGNAL(triggered()), this, SLOT(goBack()));
    connect(m_buttonNext, SIGNAL(triggered()), this, SLOT(goNext()));
    connect(m_buttonStop, SIGNAL(triggered()), this, SLOT(stop()));
    connect(m_buttonReload, SIGNAL(triggered()), this, SLOT(reload()));
    connect(m_buttonHome, SIGNAL(triggered()), this, SLOT(goHome()));
    connect(m_actionExitFullscreen, SIGNAL(triggered(bool)), this, SLOT(fullScreen(bool)));

    m_actionRestoreTab = new QAction(QIcon::fromTheme("user-trash"),tr("Restore &Closed Tab"), this);
    m_actionRestoreTab->setShortcut(QKeySequence("Ctrl+Shift+T"));
    connect(m_actionRestoreTab, SIGNAL(triggered()), m_tabWidget, SLOT(restoreClosedTab()));
    addAction(m_actionRestoreTab);

    QAction* reloadByPassCacheAction = new QAction(this);
    reloadByPassCacheAction->setShortcut(QKeySequence("Ctrl+F5"));
    connect(reloadByPassCacheAction, SIGNAL(triggered()), this, SLOT(reloadByPassCache()));
    addAction(reloadByPassCacheAction);

    //Make shortcuts available even in fullscreen (menu hidden)
    QList<QAction*> actions = menuBar()->actions();
    foreach (QAction* action, actions) {
        if (action->menu())
            actions += action->menu()->actions();
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

void QupZilla::setBackground(QColor textColor)
{
    QString color = textColor.name();
    setStyleSheet("QMainWindow { background-image: url("+m_activeProfil+"background.png); background-position: top right; } QToolBar{background-image:url(:icons/transp.png); border:none;}"
                  "QMenuBar{color:"+color+";background-image:url(:icons/transp.png); border:none;} QStatusBar{background-image:url(:icons/transp.png); border:none; color:"+color+";}"
                  "QMenuBar:item{spacing: 5px; padding: 2px 6px;background: transparent;}"
                  "QMenuBar::item:pressed { background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,stop:0 lightgray, stop:1 darkgray); border: 1px solid darkgrey; border-top-left-radius: 4px;border-top-right-radius: 4px; border-bottom: none;}"
                  );
    m_navigation->setStyleSheet("QSplitter::handle{background-color:transparent;}");

}

void QupZilla::loadSettings()
{
    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);

    //Url settings
    settings.beginGroup("Web-URL-Settings");
    m_homepage = settings.value("homepage","http://qupzilla.ic.cz/search/").toUrl();
    m_newtab = settings.value("newTabUrl","").toUrl();
    settings.endGroup();

    QWebSettings* websettings = mApp->webSettings();
    websettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);

    //Browser Window settings
    settings.beginGroup("Browser-View-Settings");
    m_menuTextColor = settings.value("menuTextColor", QColor(Qt::black)).value<QColor>();
    setBackground(m_menuTextColor);
    m_bookmarksToolbar->setColor(m_menuTextColor);
    m_ipLabel->setStyleSheet("QLabel {color: "+m_menuTextColor.name()+";}");
    bool showStatusBar = settings.value("showStatusBar",true).toBool();
    bool showHomeIcon = settings.value("showHomeButton",true).toBool();
    bool showBackForwardIcons = settings.value("showBackForwardButtons",true).toBool();
    bool showBookmarksToolbar = settings.value("showBookmarksToolbar",true).toBool();
    bool showNavigationToolbar = settings.value("showNavigationToolbar",true).toBool();
    bool showMenuBar = settings.value("showMenubar",true).toBool();
    bool makeTransparent = settings.value("useTransparentBackground",false).toBool();
    QString activeSideBar = settings.value("SideBar", "None").toString();
    settings.endGroup();
    bool adBlockEnabled = settings.value("AdBlock/enabled", true).toBool();

    m_adblockIcon->setEnabled(adBlockEnabled);

    statusBar()->setVisible(showStatusBar);
    m_bookmarksToolbar->setVisible(showBookmarksToolbar);
    m_navigation->setVisible(showNavigationToolbar);
    menuBar()->setVisible(showMenuBar);
    m_navigation->actions().at(m_navigation->actions().count() - 1)->setVisible(!showMenuBar);

    m_buttonHome->setVisible(showHomeIcon);
    m_buttonBack->setVisible(showBackForwardIcons);
    m_buttonNext->setVisible(showBackForwardIcons);

    if (activeSideBar != "None") {
        if (activeSideBar == "Bookmarks")
            m_actionShowBookmarksSideBar->trigger();
        else if (activeSideBar == "History")
            m_actionShowHistorySideBar->trigger();
    }

    //Private browsing
    m_actionPrivateBrowsing->setChecked( mApp->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled) );
    m_privateBrowsing->setVisible( mApp->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled) );

    setWindowIcon(QIcon(":/icons/qupzilla.png"));

    if (!makeTransparent)
        return;
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

void QupZilla::receiveMessage(MainApplication::MessageType mes, bool state)
{
    switch (mes) {
    case MainApplication::SetAdBlockIconEnabled:
        m_adblockIcon->setEnabled(state);
        break;

    case MainApplication::CheckPrivateBrowsing:
        m_privateBrowsing->setVisible(state);
        m_actionPrivateBrowsing->setChecked(state);
        break;

    case MainApplication::ReloadSettings:
        loadSettings();
        m_tabWidget->loadSettings();
        m_locationBar->loadSettings();
        break;

    default:
        qWarning("Unresolved message sent!");
        break;
    }
}

void QupZilla::refreshHistory(int index)
{
    QWebHistory* history;
    if (index == -1)
        history = weView()->page()->history();
    else
        history = weView()->page()->history();

    if (history->canGoBack()) {
        m_buttonBack->setEnabled(true);
    } else {
        m_buttonBack->setEnabled(false);
    }

    if (history->canGoForward()) {
        m_buttonNext->setEnabled(true);
    } else {
        m_buttonNext->setEnabled(false);
    }
}

void QupZilla::goAtHistoryIndex()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        weView()->page()->history()->goToItem(weView()->page()->history()->itemAt(action->data().toInt()));
    }
    refreshHistory();
}

void QupZilla::aboutToShowHistoryBackMenu()
{
    if (!m_menuBack || !weView())
        return;
    m_menuBack->clear();
    QWebHistory* history = weView()->history();
    int curindex = history->currentItemIndex();
    for (int i = curindex-1;i>=0;i--) {
        QWebHistoryItem item = history->itemAt(i);
        if (item.isValid()) {
            QString title = item.title();
            if (title.length()>40) {
                title.truncate(40);
                title+="..";
            }
            QAction* action = m_menuBack->addAction(_iconForUrl(item.url()),title, this, SLOT(goAtHistoryIndex()));
            action->setData(i);
        }
    }
}

void QupZilla::aboutToShowHistoryNextMenu()
{
    if (!m_menuForward || !weView())
        return;
    m_menuForward->clear();
    QWebHistory* history = weView()->history();
    int curindex = history->currentItemIndex();
    for (int i = curindex+1;i<history->count();i++) {
        QWebHistoryItem item = history->itemAt(i);
        if (item.isValid()) {
            QString title = item.title();
            if (title.length()>40) {
                title.truncate(40);
                title+="..";
            }
            QAction* action = m_menuForward->addAction(_iconForUrl(item.url()),title, this, SLOT(goAtHistoryIndex()));
            action->setData(i);
        }
    }
}

void QupZilla::aboutToShowBookmarksMenu()
{
    m_menuBookmarks->clear();
    m_menuBookmarks->addAction(tr("Bookmark &This Page"), this, SLOT(bookmarkPage()))->setShortcut(QKeySequence("Ctrl+D"));
    m_menuBookmarks->addAction(tr("Bookmark &All Tabs"), this, SLOT(bookmarkAllTabs()));
    m_menuBookmarks->addAction(QIcon::fromTheme("user-bookmarks"), tr("Organize &Bookmarks"), this, SLOT(showBookmarksManager()))->setShortcut(QKeySequence("Ctrl+Shift+O"));
    m_menuBookmarks->addSeparator();
    if (m_tabWidget->count() == 1)
        m_menuBookmarks->actions().at(1)->setEnabled(false);
    QSqlQuery query;
    query.exec("SELECT title, url FROM bookmarks WHERE folder='bookmarksMenu'");
    while(query.next()) {
        QUrl url = query.value(1).toUrl();
        QString title = query.value(0).toString();
        if (title.length()>40) {
            title.truncate(40);
            title+="..";
        }
        m_menuBookmarks->addAction(_iconForUrl(url), title, this, SLOT(loadActionUrl()))->setData(url);
    }

    QMenu* folderBookmarks = new QMenu(tr("Bookmarks In ToolBar"), m_menuBookmarks);
    folderBookmarks->setIcon(QIcon(style()->standardIcon(QStyle::SP_DirOpenIcon)));

    query.exec("SELECT title, url FROM bookmarks WHERE folder='bookmarksToolbar'");
    while(query.next()) {
        QUrl url = query.value(1).toUrl();
        QString title = query.value(0).toString();
        if (title.length()>40) {
            title.truncate(40);
            title+="..";
        }
        folderBookmarks->addAction(_iconForUrl(url), title, this, SLOT(loadActionUrl()))->setData(url);
    }
    if (folderBookmarks->isEmpty())
        folderBookmarks->addAction(tr("Empty"));
    m_menuBookmarks->addMenu(folderBookmarks);

    query.exec("SELECT name FROM folders");
    while(query.next()) {
        QMenu* tempFolder = new QMenu(query.value(0).toString(), m_menuBookmarks);
        tempFolder->setIcon(QIcon(style()->standardIcon(QStyle::SP_DirOpenIcon)));

        QSqlQuery query2;
        query2.exec("SELECT title, url FROM bookmarks WHERE folder='"+query.value(0).toString()+"'");
        while(query2.next()) {
            QUrl url = query2.value(1).toUrl();
            QString title = query2.value(0).toString();
            if (title.length()>40) {
                title.truncate(40);
                title+="..";
            }
            tempFolder->addAction(_iconForUrl(url), title, this, SLOT(loadActionUrl()))->setData(url);
        }
        if (tempFolder->isEmpty())
            tempFolder->addAction(tr("Empty"));
        m_menuBookmarks->addMenu(tempFolder);
    }

}

void QupZilla::aboutToShowHistoryMenu()
{
    if (!weView())
        return;
    m_menuHistory->clear();
    m_menuHistory->addAction(
#ifdef Q_WS_X11
            style()->standardIcon(QStyle::SP_ArrowBack)
#else
            QIcon(":/icons/faenza/back.png")
#endif
            , tr("&Back"), this, SLOT(goBack()))->setShortcut(QKeySequence("Ctrl+Left"));
    m_menuHistory->addAction(
#ifdef Q_WS_X11
            style()->standardIcon(QStyle::SP_ArrowForward)
#else
            QIcon(":/icons/faenza/forward.png")
#endif
            , tr("&Forward"), this, SLOT(goNext()))->setShortcut(QKeySequence("Ctrl+Right"));
    m_menuHistory->addAction(
#ifdef Q_WS_X11
            QIcon::fromTheme("go-home")
#else
            QIcon(":/icons/faenza/home.png")
#endif
            , tr("&Home"), this, SLOT(goHome()))->setShortcut(QKeySequence("Alt+Home"));

    if (!weView()->history()->canGoBack())
        m_menuHistory->actions().at(0)->setEnabled(false);
    if (!weView()->history()->canGoForward())
        m_menuHistory->actions().at(1)->setEnabled(false);

    m_menuHistory->addAction(QIcon(":/icons/menu/history.png"), tr("Show &All History"), this, SLOT(showHistoryManager()));
    m_menuHistory->addSeparator();

    QSqlQuery query;
    query.exec("SELECT title, url FROM history ORDER BY date DESC LIMIT 10");
    while(query.next()) {
        QUrl url = query.value(1).toUrl();
        QString title = query.value(0).toString();
        if (title.length()>40) {
            title.truncate(40);
            title+="..";
        }
        m_menuHistory->addAction(_iconForUrl(url), title, this, SLOT(loadActionUrl()))->setData(url);
    }
    m_menuHistory->addSeparator();
    QMenu* menuClosedTabs = new QMenu(tr("Closed Tabs"));
    int i = 0;
    foreach (ClosedTabsManager::Tab tab, m_tabWidget->closedTabsManager()->allClosedTabs()) {
        QString title = tab.title;
        if (title.length()>40) {
            title.truncate(40);
            title+="..";
        }
        menuClosedTabs->addAction(_iconForUrl(tab.url), title, m_tabWidget, SLOT(restoreClosedTab()))->setData(i);
        i++;
    }
    menuClosedTabs->addSeparator();
    if (i == 0)
        menuClosedTabs->addAction(tr("Empty"))->setEnabled(false);
    else
        menuClosedTabs->addAction(tr("Restore All Closed Tabs"), m_tabWidget, SLOT(restoreAllClosedTabs()));

    m_menuHistory->addMenu(menuClosedTabs);
}

void QupZilla::aboutToShowHelpMenu()
{
    m_menuHelp->clear();
    m_menuHelp->addAction(tr("Report &Bug"), this, SLOT(reportBug()));
    m_menuHelp->addSeparator();
    mApp->plugins()->populateHelpMenu(m_menuHelp);
    m_menuHelp->addAction(QIcon(":/icons/menu/qt.png"), tr("About &Qt"), qApp, SLOT(aboutQt()));
    m_menuHelp->addAction(QIcon(":/icons/qupzilla.png"), tr("&About QupZilla"), this, SLOT(aboutQupZilla()));
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
    m_actionPrivateBrowsing->setCheckable(true);
    m_actionPrivateBrowsing->setChecked(mApp->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled));
    connect(m_actionPrivateBrowsing, SIGNAL(triggered(bool)), this, SLOT(startPrivate(bool)));
    m_menuTools->addAction(m_actionPrivateBrowsing);
    m_menuTools->addSeparator();
    mApp->plugins()->populateToolsMenu(m_menuTools);
    m_menuTools->addAction(QIcon(":/icons/faenza/settings.png"), tr("Pr&eferences"), this, SLOT(showPreferences()))->setShortcut(QKeySequence("Ctrl+P"));
}

void QupZilla::aboutToShowViewMenu()
{
    if (!weView())
        return;

    if (weView()->isLoading())
        m_actionStop->setEnabled(true);
    else
        m_actionStop->setEnabled(false);
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

    foreach (QByteArray name, available) {
        if (QTextCodec::codecForName(name)->aliases().contains(name))
            continue;
        QAction* action = new QAction(name=="System" ? tr("Default") : name, this);
        action->setData(name);
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), this, SLOT(changeEncoding()));
        if (activeCodec.compare(name, Qt::CaseInsensitive) == 0)
            action->setChecked(true);

        if (name.startsWith("ISO"))
            menuISO->addAction(action);
        else if (name.startsWith("UTF"))
            menuUTF->addAction(action);
        else if (name.startsWith("windows"))
            menuWindows->addAction(action);
        else if (name.startsWith("Iscii"))
            menuIscii->addAction(action);
        else if (name == "System")
            m_menuEncoding->addAction(action);
        else
            menuOther->addAction(action);
    }

    m_menuEncoding->addSeparator();
    if (!menuISO->isEmpty())
        m_menuEncoding->addMenu(menuISO);
    if (!menuUTF->isEmpty())
        m_menuEncoding->addMenu(menuUTF);
    if (!menuWindows->isEmpty())
        m_menuEncoding->addMenu(menuWindows);
    if (!menuIscii->isEmpty())
        m_menuEncoding->addMenu(menuIscii);
    if (!menuOther->isEmpty())
        m_menuEncoding->addMenu(menuOther);
}

void QupZilla::aboutToShowSidebarsMenu()
{
    if (!m_sideBar) {
        m_actionShowBookmarksSideBar->setChecked(false);
        m_actionShowHistorySideBar->setChecked(false);
//        m_actionShowRssSideBar->setChecked(false);
    } else {
        SideBar::SideWidget actWidget = m_sideBar->activeWidget();
        m_actionShowBookmarksSideBar->setChecked(actWidget == SideBar::Bookmarks);
        m_actionShowHistorySideBar->setChecked(actWidget == SideBar::History);
//        m_actionShowRssSideBar->setChecked(actWidget == SideBar::RSS);
    }
}

void QupZilla::aboutToShowToolbarsMenu()
{
    m_actionShowToolbar->setChecked(m_navigation->isVisible());
    m_actionShowMenubar->setChecked(menuBar()->isVisible());
    m_actionShowStatusbar->setChecked(statusBar()->isVisible());
    m_actionShowBookmarksToolbar->setChecked(m_bookmarksToolbar->isVisible());
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
    mApp->bookmarksManager()->addBookmark(weView());
}

void QupZilla::addBookmark(const QUrl &url, const QString &title)
{
    mApp->bookmarksManager()->insertBookmark(url, title);
}

void QupZilla::bookmarkAllTabs()
{
    mApp->bookmarksManager()->insertAllTabs();
}

void QupZilla::loadActionUrl()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        loadAddress(action->data().toUrl());
    }
}

void QupZilla::urlEnter()
{
    if (m_locationBar->text().isEmpty())
        return;
    loadAddress(QUrl(WebView::guessUrlFromString(m_locationBar->text())));
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
    HistoryManager* m = mApp->historyManager();
    m->refreshTable();
    m->setMainWindow(this);
    m->show();
}

void QupZilla::showRSSManager()
{
    RSSManager* m = mApp->rssManager();
    m->refreshTable();
    m->setMainWindow(this);
    m->show();
}

void QupZilla::showBookmarksManager()
{
    BookmarksManager* m = mApp->bookmarksManager();
    m->refreshTable();
    m->setMainWindow(this);
    m->show();
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

void QupZilla::showSource()
{
    SourceViewer* source = new SourceViewer(weView()->page());
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

    QSettings settings(activeProfil()+"settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/showBookmarksToolbar", !status);
}

void QupZilla::showBookmarksSideBar()
{
    bool saveToSettings = false;
    if (!m_sideBar) {
        m_sideBar = new SideBar(this);
        addDockWidget(Qt::LeftDockWidgetArea, m_sideBar);
        m_sideBar->showBookmarks();
        saveToSettings = true;
    } else if (m_actionShowBookmarksSideBar->isChecked()){
        m_sideBar->showBookmarks();
        saveToSettings = true;
    } else {
        m_sideBar->close();
    }
}

void QupZilla::showHistorySideBar()
{
    if (!m_sideBar) {
        m_sideBar = new SideBar(this);
        addDockWidget(Qt::LeftDockWidgetArea, m_sideBar);
        m_sideBar->showHistory();
    } else if (m_actionShowHistorySideBar->isChecked()) {
        m_sideBar->showHistory();
    } else {
        m_sideBar->close();
    }
}

void QupZilla::showNavigationToolbar()
{
    if (!menuBar()->isVisible() && !m_actionShowToolbar->isChecked())
        showMenubar();

    bool status = m_navigation->isVisible();
    m_navigation->setVisible(!status);

    QSettings settings(activeProfil()+"settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/showNavigationToolbar", !status);
}

void QupZilla::showMenubar()
{
    if (!m_navigation->isVisible() && !m_actionShowMenubar->isChecked())
        showNavigationToolbar();

    menuBar()->setVisible(!menuBar()->isVisible());
    m_navigation->actions().at(m_navigation->actions().count() - 1)->setVisible(!menuBar()->isVisible());

    QSettings settings(activeProfil()+"settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/showMenubar", menuBar()->isVisible());
}

void QupZilla::showStatusbar()
{
    bool status = statusBar()->isVisible();
    statusBar()->setVisible(!status);

    QSettings settings(activeProfil()+"settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/showStatusbar", !status);
}

void QupZilla::showInspector()
{
    if (!m_webInspectorDock) {
        m_webInspectorDock = new QDockWidget(this);
        if (m_webInspector)
            delete m_webInspector;
        m_webInspector = new QWebInspector(this);
        m_webInspector->setPage(weView()->page());
        addDockWidget(Qt::BottomDockWidgetArea, m_webInspectorDock);
        m_webInspectorDock->setWindowTitle(tr("Web Inspector"));
        m_webInspectorDock->setTitleBarWidget(new DockTitleBarWidget(tr("Web Inspector"), m_webInspectorDock));
        m_webInspectorDock->setObjectName("WebInspector");
        m_webInspectorDock->setWidget(m_webInspector);
        m_webInspectorDock->setFeatures(0);
        m_webInspectorDock->setContextMenuPolicy(Qt::CustomContextMenu);
    } else if (m_webInspectorDock->isVisible()) { //Next tab
        m_webInspectorDock->show();
        m_webInspector->setPage(weView()->page());
        m_webInspectorDock->setWidget(m_webInspector);
    } else { //Showing hidden dock
        m_webInspectorDock->show();
        if (m_webInspector->page() != weView()->page()) {
            m_webInspector->setPage(weView()->page());
            m_webInspectorDock->setWidget(m_webInspector);
        }
    }
}

void QupZilla::aboutQupZilla()
{
    AboutDialog about(this);
    about.exec();
}

void QupZilla::searchOnPage()
{
    if (!m_webSearchToolbar) {
        m_webSearchToolbar = new SearchToolBar(this);
        addToolBar(Qt::BottomToolBarArea, m_webSearchToolbar);
        m_webSearchToolbar->showBar();
        return;
    }
    if (m_webSearchToolbar->isVisible()) {
        m_webSearchToolbar->hideBar();
        weView()->setFocus();
    }else{
        m_webSearchToolbar->showBar();
    }
}

void QupZilla::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open file..."), QDir::homePath(), "(*.html *.htm *.jpg *.png)");
    if (!filePath.isEmpty())
        loadAddress(QUrl(filePath));
}

void QupZilla::showNavigationWithFullscreen()
{
    bool state;
    if (m_navigationVisible)
        state = !m_navigation->isVisible();
    else
        state = !m_tabWidget->getTabBar()->isVisible();
    if (m_navigationVisible)
        m_navigation->setVisible(state);
    m_tabWidget->getTabBar()->setVisible(state);
    if (m_bookmarksToolBarVisible)
        m_bookmarksToolbar->setVisible(state);
}

void QupZilla::fullScreen(bool make)
{
    if (make) {
        m_menuBarVisible = menuBar()->isVisible();
        m_statusBarVisible = statusBar()->isVisible();
        m_navigationVisible = m_navigation->isVisible();
        m_bookmarksToolBarVisible = m_bookmarksToolbar->isVisible();
        setWindowState(windowState() | Qt::WindowFullScreen);
        menuBar()->hide();
        statusBar()->hide();
        bookmarksToolbar()->hide();
        m_navigation->hide();
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        menuBar()->setVisible(m_menuBarVisible);
        statusBar()->setVisible(m_statusBarVisible);
        m_bookmarksToolbar->setVisible(m_bookmarksToolBarVisible);
        m_navigation->setVisible(m_navigationVisible);
    }
    m_actionShowFullScreen->setChecked(make);
    m_actionExitFullscreen->setVisible(make);
    m_tabWidget->getTabBar()->setVisible(!make);
    emit setWebViewMouseTracking(make);
}

void QupZilla::savePage()
{
    QNetworkRequest request(weView()->url());

    DownloadManager* dManager = mApp->downManager();
    dManager->download(request, false);
}

void QupZilla::printPage()
{
    QPrintPreviewDialog* dialog = new QPrintPreviewDialog(this);
    connect(dialog, SIGNAL(paintRequested(QPrinter*)), weView(), SLOT(print(QPrinter*)));
    dialog->exec();
    delete dialog;
}

void QupZilla::startPrivate(bool state)
{
    if (state) {
        QString title = tr("Are you sure you want to turn on private browsing?");
        QString text1 = tr("When private browsing is turned on, some actions concerning your privacy will be disabled:");

        QStringList actions;
        actions.append(tr("Webpages are not added to the history."));
        actions.append(tr("New cookies are not stored, but current cookies can be accessed."));
        actions.append(tr("Your session won't be stored."));

        QString text2 = tr("Until you close the window, you can still click the Back and Forward "
                                   "buttons to return to the webpages you have opened.");

        QString message = QString(QLatin1String("<b>%1</b><p>%2</p><ul><li>%3</li></ul><p>%4</p>")).arg(title, text1, actions.join(QLatin1String("</li><li>")), text2);

        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Start Private Browsing"),
                                                                   message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (button != QMessageBox::Yes)
            return;
    }
    mApp->webSettings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, state);
    mApp->history()->setSaving(!state);
    mApp->cookieJar()->turnPrivateJar(state);
    emit message(MainApplication::CheckPrivateBrowsing, state);
}

void QupZilla::closeEvent(QCloseEvent* event)
{
    if (mApp->isClosing())
        return;

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
    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-URL-Settings");
    int afterLaunch = settings.value("afterLaunch",0).toInt();
    settings.endGroup();
    bool askOnClose = !settings.value("Browser-View-Settings/DontAskOnClosing", false).toBool();

    if (askOnClose && afterLaunch != 2 && m_tabWidget->count() > 1) {
        QDialog* dialog = new QDialog(this);
        Ui_CloseDialog* ui = new Ui_CloseDialog();
        ui->setupUi(dialog);
        ui->textLabel->setText(tr("There are still %1 open tabs and your session won't be stored. Are you sure to quit?").arg(m_tabWidget->count()));
        ui->iconLabel->setPixmap(style()->standardPixmap(QStyle::SP_MessageBoxWarning));
        if (dialog->exec() != QDialog::Accepted)
            return false;
        if (ui->dontAskAgain->isChecked())
            settings.setValue("Browser-View-Settings/DontAskOnClosing", true);
    }

    mApp->quitApplication();
    return true;
}

QupZilla::~QupZilla()
{
    delete m_tabWidget;
    delete m_privateBrowsing;
    delete m_adblockIcon;
    delete m_menuBack;
    delete m_menuForward;
    delete m_locationBar;
    delete m_searchLine;
    delete m_bookmarksToolbar;
    delete m_webSearchToolbar;
    delete m_buttonBack;
    delete m_buttonNext;
    delete m_buttonHome;
    delete m_buttonStop;
    delete m_buttonReload;
    delete m_actionExitFullscreen;
    delete m_navigationSplitter;
    delete m_navigation;
    delete m_progressBar;

    if (m_webInspectorDock) {
        delete m_webInspector;
        delete m_webInspectorDock;
    }
}
