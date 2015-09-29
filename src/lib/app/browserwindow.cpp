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
#include "browserwindow.h"
#include "tabwidget.h"
#include "tabbar.h"
#include "webpage.h"
#include "tabbedwebview.h"
#include "lineedit.h"
#include "history.h"
#include "locationbar.h"
#include "searchtoolbar.h"
#include "websearchbar.h"
#include "pluginproxy.h"
#include "sidebar.h"
#include "downloadmanager.h"
#include "cookiejar.h"
#include "cookiemanager.h"
#include "bookmarkstoolbar.h"
#include "clearprivatedata.h"
#include "sourceviewer.h"
#include "networkmanager.h"
#include "autofill.h"
#include "networkmanagerproxy.h"
#include "rssmanager.h"
#include "mainapplication.h"
#include "checkboxdialog.h"
#include "adblockmanager.h"
#include "clickablelabel.h"
#include "docktitlebarwidget.h"
#include "iconprovider.h"
#include "progressbar.h"
#include "adblockicon.h"
#include "closedtabsmanager.h"
#include "statusbarmessage.h"
#include "browsinglibrary.h"
#include "navigationbar.h"
#include "pagescreen.h"
#include "bookmarksimport/bookmarksimportdialog.h"
#include "qztools.h"
#include "reloadstopbutton.h"
#include "enhancedmenu.h"
#include "navigationcontainer.h"
#include "settings.h"
#include "qzsettings.h"
#include "webtab.h"
#include "speeddial.h"
#include "menubar.h"
#include "qtwin.h"
#include "bookmarkstools.h"
#include "bookmarksmenu.h"
#include "historymenu.h"
#include "mainmenu.h"

#include <algorithm>

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
#include <QPrinter>
#include <QWebFrame>
#include <QWebHistory>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QToolTip>
#include <QScrollArea>

#if QT_VERSION < 0x050000
#include "qwebkitversion.h"
#endif

#ifdef QZ_WS_X11
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif
#endif /* Q_OS_WIN */

const QString BrowserWindow::WEBKITVERSION = qWebKitVersion();

static QKeySequence actionShortcut(QKeySequence shortcut, QKeySequence fallBack, QKeySequence shortcutRTL = QKeySequence(), QKeySequence fallbackRTL = QKeySequence())
{
    if (QApplication::isRightToLeft() && (!shortcutRTL.isEmpty() || !fallbackRTL.isEmpty())) {
        return (shortcutRTL.isEmpty() ? fallbackRTL : shortcutRTL);
    }
    else {
        return (shortcut.isEmpty() ? fallBack : shortcut);
    }
}

BrowserWindow::BrowserWindow(Qz::BrowserWindowType type, const QUrl &startUrl)
    : QMainWindow(0)
    , m_startUrl(startUrl)
    , m_windowType(type)
    , m_startTab(0)
    , m_sideBarManager(new SideBarManager(this))
    , m_statusBarMessage(new StatusBarMessage(this))
    , m_useTransparentBackground(false)
    , m_hideNavigationTimer(0)
{
    setObjectName("mainwindow");
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("QupZilla"));
    setProperty("private", mApp->isPrivate());

    setupUi();
    setupMenu();

    m_hideNavigationTimer = new QTimer(this);
    m_hideNavigationTimer->setInterval(1000);
    m_hideNavigationTimer->setSingleShot(true);
    connect(m_hideNavigationTimer, SIGNAL(timeout()), this, SLOT(hideNavigationSlot()));

    connect(mApp, SIGNAL(settingsReloaded()), this, SLOT(loadSettings()));

    QTimer::singleShot(0, this, SLOT(postLaunch()));

    if (mApp->isPrivate()) {
        QzTools::setWmClass("QupZilla Browser (Private Window)", this);
    }
    else {
        QzTools::setWmClass("QupZilla Browser", this);
    }
}

BrowserWindow::~BrowserWindow()
{
    mApp->plugins()->emitMainWindowDeleted(this);

    foreach (const QPointer<QWidget> &pointer, m_deleteOnCloseWidgets) {
        if (pointer) {
            pointer->deleteLater();
        }
    }
}

void BrowserWindow::setStartTab(WebTab* tab)
{
    m_startTab = tab;
}

void BrowserWindow::postLaunch()
{
    loadSettings();

    Settings settings;
    int afterLaunch = settings.value("Web-URL-Settings/afterLaunch", 3).toInt();
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

    switch (m_windowType) {
    case Qz::BW_FirstAppWindow:
        if (mApp->isStartingAfterCrash()) {
            addTab = false;
            startUrl.clear();
            m_tabWidget->addView(QUrl("qupzilla:restore"), Qz::NT_CleanSelectedTabAtTheEnd);
        }
        else if (afterLaunch == 3 && mApp->restoreManager()) {
            addTab = !mApp->restoreSession(this, mApp->restoreManager()->restoreData());
        }
        else {
            // Restore pinned tabs also when not restoring session
            m_tabWidget->restorePinnedTabs();
        }
        break;

    case Qz::BW_MacFirstWindow:
        m_tabWidget->restorePinnedTabs();
        // fallthrough

    case Qz::BW_NewWindow:
        addTab = true;
        break;

    case Qz::BW_OtherRestoredWindow:
        addTab = false;
        break;
    }

    show();

    if (!m_startUrl.isEmpty()) {
        startUrl = m_startUrl;
        addTab = true;
    }

    if (m_startTab) {
        addTab = false;
        m_tabWidget->addView(m_startTab);
    }

    if (addTab) {
        QNetworkRequest request(startUrl);
        request.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));

        m_tabWidget->addView(request, Qz::NT_CleanSelectedTabAtTheEnd);

        if (startUrl.isEmpty() || startUrl.toString() == QLatin1String("qupzilla:speeddial")) {
            locationBar()->setFocus();
        }
    }

    // Something went really wrong .. add one tab
    if (m_tabWidget->tabBar()->normalTabsCount() <= 0) {
        QNetworkRequest request(m_homepage);
        request.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));

        m_tabWidget->addView(request, Qz::NT_SelectedTabAtTheEnd);
    }

    mApp->plugins()->emitMainWindowCreated(this);
    emit startingCompleted();

    raise();
    activateWindow();

    QTimer::singleShot(0, tabWidget()->tabBar(), SLOT(ensureVisible()));
}

void BrowserWindow::setupUi()
{
    int locationBarWidth;
    int websearchBarWidth;

    QDesktopWidget* desktop = mApp->desktop();
    int windowWidth = desktop->availableGeometry().width() / 1.3;
    int windowHeight = desktop->availableGeometry().height() / 1.3;

    Settings settings;
    settings.beginGroup("Browser-View-Settings");
    if (settings.value("WindowMaximised", false).toBool()) {
        resize(windowWidth, windowHeight);
        setWindowState(Qt::WindowMaximized);
    }
    else {
        // Let the WM decides where to put new browser window
        if ((m_windowType != Qz::BW_FirstAppWindow && m_windowType != Qz::BW_MacFirstWindow) && mApp->getWindow()) {
#ifdef Q_WS_WIN
            // Windows WM places every new window in the middle of screen .. for some reason
            QPoint p = mApp->getWindow()->geometry().topLeft();
            p.setX(p.x() + 30);
            p.setY(p.y() + 30);

            if (!desktop->availableGeometry(mApp->getWindow()).contains(p)) {
                p.setX(desktop->availableGeometry(mApp->getWindow()).x() + 30);
                p.setY(desktop->availableGeometry(mApp->getWindow()).y() + 30);
            }

            setGeometry(QRect(p, mApp->getWindow()->size()));
#else
            resize(mApp->getWindow()->size());
#endif
        }
        else if (!restoreGeometry(settings.value("WindowGeometry").toByteArray())) {
#ifdef Q_WS_WIN
            setGeometry(QRect(desktop->availableGeometry(mApp->getWindow()).x() + 30,
                              desktop->availableGeometry(mApp->getWindow()).y() + 30, windowWidth, windowHeight));
#else
            resize(windowWidth, windowHeight);
#endif
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
    m_tabWidget = new TabWidget(this);
    m_superMenu = new QMenu(this);
    m_navigationToolbar = new NavigationBar(this);
    m_navigationToolbar->setSplitterSizes(locationBarWidth, websearchBarWidth);
    m_bookmarksToolbar = new BookmarksToolbar(this);

    m_navigationContainer = new NavigationContainer(this);
    m_navigationContainer->addWidget(m_navigationToolbar);
    m_navigationContainer->addWidget(m_bookmarksToolbar);
    m_navigationContainer->setTabBar(m_tabWidget->tabBar());

    m_mainSplitter->addWidget(m_tabWidget);
    m_mainSplitter->setCollapsible(0, false);

    m_mainLayout->addWidget(m_navigationContainer);
    m_mainLayout->addWidget(m_mainSplitter);

    statusBar()->setObjectName("mainwindow-statusbar");
    statusBar()->setCursor(Qt::ArrowCursor);
    m_progressBar = new ProgressBar(statusBar());
    m_adblockIcon = new AdBlockIcon(this);
    m_ipLabel = new QLabel(this);
    m_ipLabel->setObjectName("statusbar-ip-label");
    m_ipLabel->setToolTip(tr("IP Address of current page"));

    statusBar()->addPermanentWidget(m_progressBar);
    statusBar()->addPermanentWidget(m_ipLabel);
    statusBar()->addPermanentWidget(m_adblockIcon);

    // Workaround for Oxygen tooltips not having transparent background
    QPalette pal = QToolTip::palette();
    QColor col = pal.window().color();
    col.setAlpha(0);
    pal.setColor(QPalette::Window, col);
    QToolTip::setPalette(pal);

    // Set some sane minimum width
    setMinimumWidth(300);
}

void BrowserWindow::setupMenu()
{
#ifdef Q_OS_MAC
    static MainMenu* macMainMenu = 0;

    if (!macMainMenu) {
        macMainMenu = new MainMenu(this, 0);
        macMainMenu->initMenuBar(new QMenuBar(0));
    }
    else {
        macMainMenu->setWindow(this);
    }

    m_mainMenu = macMainMenu;
#else
    setMenuBar(new MenuBar(this));

    m_mainMenu = new MainMenu(this, this);
    m_mainMenu->initMenuBar(menuBar());
    m_mainMenu->initSuperMenu(m_superMenu);
#endif

    // Setup other shortcuts
    QShortcut* reloadBypassCacheAction = new QShortcut(QKeySequence(QSL("Ctrl+F5")), this);
    QShortcut* reloadBypassCacheAction2 = new QShortcut(QKeySequence(QSL("Ctrl+Shift+R")), this);
    connect(reloadBypassCacheAction, SIGNAL(activated()), this, SLOT(reloadBypassCache()));
    connect(reloadBypassCacheAction2, SIGNAL(activated()), this, SLOT(reloadBypassCache()));

    QShortcut* closeTabAction = new QShortcut(QKeySequence(QSL("Ctrl+W")), this);
    QShortcut* closeTabAction2 = new QShortcut(QKeySequence(QSL("Ctrl+F4")), this);
    connect(closeTabAction, SIGNAL(activated()), this, SLOT(closeTab()));
    connect(closeTabAction2, SIGNAL(activated()), this, SLOT(closeTab()));

    QShortcut* reloadAction = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(reloadAction, SIGNAL(activated()), this, SLOT(reload()));

    QShortcut* openLocationAction = new QShortcut(QKeySequence("Alt+D"), this);
    connect(openLocationAction, SIGNAL(activated()), this, SLOT(openLocation()));

    QShortcut* inspectorAction = new QShortcut(QKeySequence(QSL("F12")), this);
    connect(inspectorAction, SIGNAL(activated()), this, SLOT(toggleWebInspector()));
}

QAction* BrowserWindow::createEncodingAction(const QString &codecName,
                                             const QString &activeCodecName, QMenu* menu)
{
    QAction* action = new QAction(codecName, menu);
    action->setData(codecName);
    action->setCheckable(true);
    connect(action, SIGNAL(triggered()), this, SLOT(changeEncoding()));
    if (activeCodecName.compare(codecName, Qt::CaseInsensitive) == 0) {
        action->setChecked(true);
    }
    return action;
}

void BrowserWindow::createEncodingSubMenu(const QString &name, QStringList &codecNames, QMenu* menu)
{
    if (codecNames.isEmpty()) {
        return;
    }

    // TODO: Alphanumeric sorting: QCollator (5.2+) or http://www.davekoelle.com/alphanum.html
    std::sort(codecNames.begin(), codecNames.end());

    QMenu* subMenu = new QMenu(name, menu);
    const QString activeCodecName = QWebSettings::globalSettings()->defaultTextEncoding();

    foreach (const QString &codecName, codecNames) {
        subMenu->addAction(createEncodingAction(codecName, activeCodecName, subMenu));
    }

    menu->addMenu(subMenu);
}

void BrowserWindow::loadSettings()
{
    Settings settings;

    //Url settings
    settings.beginGroup("Web-URL-Settings");
    m_homepage = settings.value("homepage", "qupzilla:start").toUrl();
    settings.endGroup();

    //Browser Window settings
    settings.beginGroup("Browser-View-Settings");
    bool showStatusBar = settings.value("showStatusBar", true).toBool();
    bool showReloadButton = settings.value("showReloadButton", true).toBool();
    bool showHomeButton = settings.value("showHomeButton", true).toBool();
    bool showBackForwardButtons = settings.value("showBackForwardButtons", true).toBool();
    bool showAddTabButton = settings.value("showAddTabButton", false).toBool();
    bool showWebSearchBar = settings.value("showWebSearchBar", true).toBool();
    bool showBookmarksToolbar = settings.value("showBookmarksToolbar", true).toBool();
    bool showNavigationToolbar = settings.value("showNavigationToolbar", true).toBool();
    bool showMenuBar = settings.value("showMenubar", true).toBool();
    bool makeTransparent = settings.value("useTransparentBackground", false).toBool();
    m_sideBarWidth = settings.value("SideBarWidth", 250).toInt();
    m_webViewWidth = settings.value("WebViewWidth", 2000).toInt();
    const QString activeSideBar = settings.value("SideBar", "None").toString();

    // Make sure both menubar and navigationbar are not hidden
    // Fixes #781
    if (!showNavigationToolbar) {
        showMenuBar = true;
        settings.setValue("showMenubar", true);
    }

    settings.endGroup();

    settings.beginGroup("Shortcuts");
    m_useTabNumberShortcuts = settings.value("useTabNumberShortcuts", true).toBool();
    m_useSpeedDialNumberShortcuts = settings.value("useSpeedDialNumberShortcuts", true).toBool();
    m_useSingleKeyShortcuts = settings.value("useSingleKeyShortcuts", false).toBool();
    settings.endGroup();

    settings.beginGroup("Web-Browser-Settings");
    QAction *quitAction = m_mainMenu->action(QSL("Standard/Quit"));
    if (settings.value("closeAppWithCtrlQ", true).toBool()) {
        quitAction->setShortcut(actionShortcut(QKeySequence::Quit, QKeySequence(QSL("Ctrl+Q"))));
    } else {
        quitAction->setShortcut(QKeySequence());
    }
    settings.endGroup();

    m_adblockIcon->setEnabled(settings.value("AdBlock/enabled", true).toBool());

    statusBar()->setVisible(!isFullScreen() && showStatusBar);
    m_bookmarksToolbar->setVisible(showBookmarksToolbar);
    m_navigationToolbar->setVisible(showNavigationToolbar);

#ifndef Q_OS_MAC
    menuBar()->setVisible(!isFullScreen() && showMenuBar);
#endif

    m_navigationToolbar->setSuperMenuVisible(!showMenuBar);
    m_navigationToolbar->buttonReloadStop()->setVisible(showReloadButton);
    m_navigationToolbar->buttonHome()->setVisible(showHomeButton);
    m_navigationToolbar->buttonBack()->setVisible(showBackForwardButtons);
    m_navigationToolbar->buttonForward()->setVisible(showBackForwardButtons);
    m_navigationToolbar->webSearchBar()->setVisible(showWebSearchBar);
    m_navigationToolbar->buttonAddTab()->setVisible(showAddTabButton);

    m_sideBarManager->showSideBar(activeSideBar, false);

#ifdef Q_OS_WIN
    if (m_useTransparentBackground && !makeTransparent) {
        QtWin::extendFrameIntoClientArea(this, 0, 0, 0, 0);
        QtWin::enableBlurBehindWindow(this, false);
        m_tabWidget->tabBar()->enableBluredBackground(false);
        m_useTransparentBackground = false;
    }
#endif

    if (!makeTransparent) {
        return;
    }

    // Transparency on X11 (no blur like on Windows)
#ifdef QZ_WS_X11
    if (QzTools::isPlatformX11()) {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground, false);
        QPalette pal = palette();
        QColor bg = pal.window().color();
        bg.setAlpha(180);
        pal.setColor(QPalette::Window, bg);
        setPalette(pal);
        ensurePolished(); // workaround Oxygen filling the background
        setAttribute(Qt::WA_StyledBackground, false);
    }
#endif

#ifdef Q_OS_WIN
    if (QtWin::isCompositionEnabled()) {
        setContentsMargins(0, 0, 0, 0);

        m_useTransparentBackground = true;

        if (!isFullScreen()) {
            m_tabWidget->tabBar()->enableBluredBackground(true);
            QtWin::extendFrameIntoClientArea(this);
        }

        // Install event filters
        menuBar()->installEventFilter(this);
        m_tabWidget->tabBar()->installEventFilter(this);
        m_navigationToolbar->installEventFilter(this);
        m_bookmarksToolbar->installEventFilter(this);
        statusBar()->installEventFilter(this);
        m_navigationContainer->installEventFilter(this);
    }
#endif
}

void BrowserWindow::goForward()
{
    weView()->forward();
}

void BrowserWindow::reload()
{
    weView()->reload();
}

void BrowserWindow::reloadBypassCache()
{
    weView()->reloadBypassCache();
}

void BrowserWindow::goBack()
{
    weView()->back();
}

TabbedWebView* BrowserWindow::weView() const
{
    return weView(m_tabWidget->currentIndex());
}

TabbedWebView* BrowserWindow::weView(int index) const
{
    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    if (!webTab) {
        return 0;
    }

    return webTab->webView();
}

LocationBar* BrowserWindow::locationBar() const
{
    return qobject_cast<LocationBar*>(m_tabWidget->locationBars()->currentWidget());
}

TabWidget* BrowserWindow::tabWidget() const
{
    return m_tabWidget;
}

BookmarksToolbar* BrowserWindow::bookmarksToolbar() const
{
    return m_bookmarksToolbar;
}

StatusBarMessage* BrowserWindow::statusBarMessage() const
{
    return m_statusBarMessage;
}

NavigationBar* BrowserWindow::navigationBar() const
{
    return m_navigationToolbar;
}

SideBarManager* BrowserWindow::sideBarManager() const
{
    return m_sideBarManager;
}

QLabel* BrowserWindow::ipLabel() const
{
    return m_ipLabel;
}

AdBlockIcon* BrowserWindow::adBlockIcon() const
{
    return m_adblockIcon;
}

QMenu* BrowserWindow::superMenu() const
{
    return m_superMenu;
}

QUrl BrowserWindow::homepageUrl() const
{
    return m_homepage;
}

Qz::BrowserWindowType BrowserWindow::windowType() const
{
    return m_windowType;
}

bool BrowserWindow::isTransparentBackgroundAllowed() const
{
    return m_useTransparentBackground && !isFullScreen();
}

QAction* BrowserWindow::action(const QString &name) const
{
    return m_mainMenu->action(name);
}

void BrowserWindow::setWindowTitle(const QString &t)
{
    QString title = t;

    if (mApp->isPrivate()) {
        title.append(tr(" (Private Browsing)"));
    }

    QMainWindow::setWindowTitle(title);
}

void BrowserWindow::changeEncoding()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        const QString encoding = action->data().toString();
        QWebSettings::globalSettings()->setDefaultTextEncoding(encoding);

        Settings settings;
        settings.setValue("Web-Browser-Settings/DefaultEncoding", encoding);

        weView()->reload();
    }
}

void BrowserWindow::bookmarkPage()
{
    TabbedWebView* view = weView();
    BookmarksTools::addBookmarkDialog(this, view->url(), view->title());
}

void BrowserWindow::bookmarkAllTabs()
{
    BookmarksTools::bookmarkAllTabsDialog(this, m_tabWidget);
}

void BrowserWindow::addBookmark(const QUrl &url, const QString &title)
{
    BookmarksTools::addBookmarkDialog(this, url, title);
}

void BrowserWindow::goHome()
{
    loadAddress(m_homepage);
}

void BrowserWindow::goHomeInNewTab()
{
    m_tabWidget->addView(m_homepage, Qz::NT_SelectedTab);
}

void BrowserWindow::loadActionUrl(QObject* obj)
{
    if (!obj) {
        obj = sender();
    }

    if (QAction* action = qobject_cast<QAction*>(obj)) {
        loadAddress(action->data().toUrl());
    }
}

void BrowserWindow::loadActionUrlInNewTab(QObject* obj)
{
    if (!obj) {
        obj = sender();
    }

    if (QAction* action = qobject_cast<QAction*>(obj)) {
        m_tabWidget->addView(action->data().toUrl(), Qz::NT_SelectedTabAtTheEnd);
    }
}

void BrowserWindow::loadAddress(const QUrl &url)
{
    if (weView()->webTab()->isPinned()) {
        int index = m_tabWidget->addView(url, qzSettings->newTabPosition);
        weView(index)->setFocus();
    }
    else {
        weView()->setFocus();
        weView()->load(url);
    }
}

void BrowserWindow::showHistoryManager()
{
    mApp->browsingLibrary()->showHistory(this);
}

void BrowserWindow::showSource(QWebFrame* frame, const QString &selectedHtml)
{
    if (!frame) {
        frame = weView()->page()->mainFrame();
    }

    SourceViewer* source = new SourceViewer(frame, selectedHtml);
    QzTools::centerWidgetToParent(source, this);
    source->show();
}

SideBar* BrowserWindow::addSideBar()
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

void BrowserWindow::saveSideBarWidth()
{
    // That +1 is important here, without it, the sidebar width would
    // decrease by 1 pixel every close

    m_sideBarWidth = m_mainSplitter->sizes().at(0) + 1;
    m_webViewWidth = width() - m_sideBarWidth;
}

void BrowserWindow::toggleShowMenubar()
{
#ifdef Q_OS_MAC
    // We use one shared global menubar on Mac that can't be hidden
    return;
#endif

    setUpdatesEnabled(false);

    menuBar()->setVisible(!menuBar()->isVisible());
    m_navigationToolbar->setSuperMenuVisible(!menuBar()->isVisible());

    setUpdatesEnabled(true);

    Settings().setValue("Browser-View-Settings/showMenubar", menuBar()->isVisible());

    // Make sure we show Navigation Toolbar when Menu Bar is hidden
    if (!m_navigationToolbar->isVisible() && !menuBar()->isVisible()) {
        toggleShowNavigationToolbar();
    }
}

void BrowserWindow::toggleShowStatusBar()
{
    setUpdatesEnabled(false);

    statusBar()->setVisible(!statusBar()->isVisible());

    setUpdatesEnabled(true);

    Settings().setValue("Browser-View-Settings/showStatusBar", statusBar()->isVisible());

}

void BrowserWindow::toggleShowBookmarksToolbar()
{
    setUpdatesEnabled(false);

    m_bookmarksToolbar->setVisible(!m_bookmarksToolbar->isVisible());

    setUpdatesEnabled(true);

    Settings().setValue("Browser-View-Settings/showBookmarksToolbar", m_bookmarksToolbar->isVisible());
}

void BrowserWindow::toggleShowNavigationToolbar()
{
    setUpdatesEnabled(false);

    m_navigationToolbar->setVisible(!m_navigationToolbar->isVisible());

    setUpdatesEnabled(true);

    Settings().setValue("Browser-View-Settings/showNavigationToolbar", m_navigationToolbar->isVisible());

#ifndef Q_OS_MAC
    // Make sure we show Menu Bar when Navigation Toolbar is hidden
    if (!m_navigationToolbar->isVisible() && !menuBar()->isVisible()) {
        toggleShowMenubar();
    }
#endif
}

void BrowserWindow::toggleTabsOnTop(bool enable)
{
    qzSettings->tabsOnTop = enable;
    m_navigationContainer->toggleTabsOnTop(enable);

#ifdef Q_OS_WIN
    // workaround for changing TabsOnTop state when sidebar is visible
    // TODO: we need a solution that changing TabsOnTop state
    //       doesn't call applyBlurToMainWindow() from eventFilter()
    QTimer::singleShot(0, this, SLOT(applyBlurToMainWindow()));
#endif
}

void BrowserWindow::toggleCaretBrowsing()
{
#if QTWEBKIT_FROM_2_3
    bool enable = !QWebSettings::globalSettings()->testAttribute(QWebSettings::CaretBrowsingEnabled);

    Settings().setValue("Web-Browser-Settings/CaretBrowsing", enable);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::CaretBrowsingEnabled, enable);
#endif
}

void BrowserWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        showNormal();
    }
    else {
        showFullScreen();
    }
}

void BrowserWindow::toggleOfflineMode()
{
    bool enable = !qzSettings->workOffline;

    Settings().setValue("Web-Browser-Settings/WorkOffline", enable);

    qzSettings->workOffline = enable;
}

void BrowserWindow::showWebInspector()
{
    if (weView() && weView()->webTab()) {
        weView()->webTab()->showWebInspector();
    }
}

void BrowserWindow::toggleWebInspector()
{
    if (weView() && weView()->webTab()) {
        weView()->webTab()->toggleWebInspector();
    }
}

void BrowserWindow::refreshHistory()
{
    m_navigationToolbar->refreshHistory();
}

void BrowserWindow::currentTabChanged()
{
    TabbedWebView* view = weView();
    if (!view) {
        return;
    }

    setWindowTitle(tr("%1 - QupZilla").arg(view->webTab()->title()));
    m_ipLabel->setText(view->getIp());
    view->setFocus();

    SearchToolBar* search = searchToolBar();
    if (search) {
        search->setWebView(view);
    }

    updateLoadingActions();

    // Setting correct tab order (LocationBar -> WebSearchBar -> WebView)
    setTabOrder(locationBar(), m_navigationToolbar->webSearchBar());
    setTabOrder(m_navigationToolbar->webSearchBar(), view);
}

void BrowserWindow::updateLoadingActions()
{
    TabbedWebView* view = weView();
    if (!view) {
        return;
    }

    bool isLoading = view->isLoading();

    m_ipLabel->setVisible(!isLoading);
    m_progressBar->setVisible(isLoading);

    action(QSL("View/Stop"))->setEnabled(isLoading);
    action(QSL("View/Reload"))->setEnabled(!isLoading);

    if (isLoading) {
        m_progressBar->setValue(view->loadingProgress());
        m_navigationToolbar->showStopButton();
    }
    else {
        m_navigationToolbar->showReloadButton();
    }
}

void BrowserWindow::addDeleteOnCloseWidget(QWidget* widget)
{
    if (!m_deleteOnCloseWidgets.contains(widget)) {
        m_deleteOnCloseWidgets.append(widget);
    }
}

void BrowserWindow::restoreWindowState(const RestoreManager::WindowData &d)
{
    restoreState(d.windowState);
    m_tabWidget->restoreState(d.tabsState, d.currentTab);
}

void BrowserWindow::createToolbarsMenu(QMenu* menu)
{
    removeActions(menu->actions());
    menu->clear();

    QAction* action;

#ifndef Q_OS_MAC
    action = menu->addAction(tr("&Menu Bar"), this, SLOT(toggleShowMenubar()));
    action->setCheckable(true);
    action->setChecked(menuBar()->isVisible());
#endif

    action = menu->addAction(tr("&Navigation Toolbar"), this, SLOT(toggleShowNavigationToolbar()));
    action->setCheckable(true);
    action->setChecked(m_navigationToolbar->isVisible());

    action = menu->addAction(tr("&Bookmarks Toolbar"), this, SLOT(toggleShowBookmarksToolbar()));
    action->setCheckable(true);
    action->setChecked(m_bookmarksToolbar->isVisible());

    menu->addSeparator();

    action = menu->addAction(tr("&Tabs on Top"), this, SLOT(toggleTabsOnTop(bool)));
    action->setCheckable(true);
    action->setChecked(qzSettings->tabsOnTop);

    addActions(menu->actions());
}

void BrowserWindow::createSidebarsMenu(QMenu* menu)
{
    m_sideBarManager->createMenu(menu);
}

void BrowserWindow::createEncodingMenu(QMenu* menu)
{
    const QString activeCodecName = QWebSettings::globalSettings()->defaultTextEncoding();

    QStringList isoCodecs, utfCodecs, windowsCodecs, isciiCodecs, otherCodecs;

    foreach (const int mib, QTextCodec::availableMibs()) {
        const QString codecName = QString::fromUtf8(QTextCodec::codecForMib(mib)->name());

        if (codecName.startsWith(QLatin1String("ISO")) && !isoCodecs.contains(codecName)) {
            isoCodecs << codecName;
        }
        else if (codecName.startsWith(QLatin1String("UTF")) && !utfCodecs.contains(codecName)) {
            utfCodecs << codecName;
        }
        else if (codecName.startsWith(QLatin1String("windows"))
                 && !windowsCodecs.contains(codecName)) {
            windowsCodecs << codecName;
        }
        else if (codecName.startsWith(QLatin1String("Iscii")) && !isciiCodecs.contains(codecName)) {
            isciiCodecs << codecName;
        }
        else if (codecName == QLatin1String("System")) {
            menu->addAction(createEncodingAction(codecName, activeCodecName, menu));
        }
        else if (!otherCodecs.contains(codecName)) {
            otherCodecs << codecName;
        }
    }

    if (!menu->isEmpty()) {
        menu->addSeparator();
    }
    createEncodingSubMenu("ISO", isoCodecs, menu);
    createEncodingSubMenu("UTF", utfCodecs, menu);
    createEncodingSubMenu("Windows", windowsCodecs, menu);
    createEncodingSubMenu("Iscii", isciiCodecs, menu);
    createEncodingSubMenu(tr("Other"), otherCodecs, menu);
}

void BrowserWindow::removeActions(const QList<QAction *> &actions)
{
    foreach (QAction *action, actions) {
        removeAction(action);
    }
}

void BrowserWindow::addTab()
{
    m_tabWidget->addView(QUrl(), Qz::NT_SelectedNewEmptyTab, true);
}

void BrowserWindow::webSearch()
{
    m_navigationToolbar->webSearchBar()->setFocus();
    m_navigationToolbar->webSearchBar()->selectAll();
}

void BrowserWindow::searchOnPage()
{
    SearchToolBar* toolBar = searchToolBar();

    if (!toolBar) {
        const int searchPos = 2;

        toolBar = new SearchToolBar(weView(), this);
        m_mainLayout->insertWidget(searchPos, toolBar);
    }

    toolBar->focusSearchLine();

#ifdef Q_OS_WIN
    if (QtWin::isCompositionEnabled()) {
        applyBlurToMainWindow();
        toolBar->installEventFilter(this);
    }
#endif
}

void BrowserWindow::openFile()
{
    const QString fileTypes = QString("%1(*.html *.htm *.shtml *.shtm *.xhtml);;"
                                      "%2(*.png *.jpg *.jpeg *.bmp *.gif *.svg *.tiff);;"
                                      "%3(*.txt);;"
                                      "%4(*.*)").arg(tr("HTML files"), tr("Image files"), tr("Text files"), tr("All files"));

    const QString filePath = QzTools::getOpenFileName("MainWindow-openFile", this, tr("Open file..."), QDir::homePath(), fileTypes);

    if (!filePath.isEmpty()) {
        loadAddress(QUrl::fromLocalFile(filePath));
    }
}

void BrowserWindow::openLocation()
{
    if (isFullScreen()) {
        showNavigationWithFullScreen();
    }

    locationBar()->setFocus();
    locationBar()->selectAll();
}

bool BrowserWindow::fullScreenNavigationVisible() const
{
    return m_navigationContainer->isVisible();
}

void BrowserWindow::showNavigationWithFullScreen()
{
    if (m_hideNavigationTimer->isActive()) {
        m_hideNavigationTimer->stop();
    }

    m_navigationContainer->show();
}

void BrowserWindow::hideNavigationWithFullScreen()
{
    if (!m_hideNavigationTimer->isActive()) {
        m_hideNavigationTimer->start();
    }
}

void BrowserWindow::hideNavigationSlot()
{
    TabbedWebView* view = weView();
    bool mouseInView = view && view->underMouse();

    if (isFullScreen() && mouseInView) {
        m_navigationContainer->hide();
    }
}

bool BrowserWindow::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::WindowStateChange: {
        QWindowStateChangeEvent* ev = static_cast<QWindowStateChangeEvent*>(event);

        if (!(ev->oldState() & Qt::WindowFullScreen) && windowState() & Qt::WindowFullScreen) {
            // Enter fullscreen
            m_windowStates = ev->oldState();

            m_statusBarVisible = statusBar()->isVisible();
#ifndef Q_OS_MAC
            m_menuBarVisible = menuBar()->isVisible();
            menuBar()->hide();
#endif
            statusBar()->hide();

            m_navigationContainer->hide();
            m_navigationToolbar->setSuperMenuVisible(false);
            m_navigationToolbar->buttonExitFullscreen()->setVisible(true);
#ifdef Q_OS_WIN
            if (m_useTransparentBackground) {
                m_tabWidget->tabBar()->enableBluredBackground(false);
                QtWin::extendFrameIntoClientArea(this, 0, 0, 0 , 0);
                QtWin::enableBlurBehindWindow(this, false);
            }
#endif
        }
        else if (ev->oldState() & Qt::WindowFullScreen && !(windowState() & Qt::WindowFullScreen)) {
            // Leave fullscreen
            setWindowState(m_windowStates);

            statusBar()->setVisible(m_statusBarVisible);
#ifndef Q_OS_MAC
            menuBar()->setVisible(m_menuBarVisible);
#endif

            m_navigationContainer->show();
            m_navigationToolbar->setSuperMenuVisible(!m_menuBarVisible);
            m_navigationToolbar->buttonExitFullscreen()->setVisible(false);
#ifdef Q_OS_WIN
            if (m_useTransparentBackground) {
                m_tabWidget->tabBar()->enableBluredBackground(true);
                applyBlurToMainWindow(true);
            }
#endif
        }

        if (m_hideNavigationTimer) {
            m_hideNavigationTimer->stop();
        }
        break;
    }

    default:
        break;
    }

    return QMainWindow::event(event);
}

void BrowserWindow::printPage(QWebFrame* frame)
{
    QPrintPreviewDialog* dialog = new QPrintPreviewDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->resize(800, 750);
    dialog->printer()->setCreator(tr("QupZilla %1 (%2)").arg(Qz::VERSION, Qz::WWWADDRESS));

    if (!frame) {
        dialog->printer()->setDocName(QzTools::getFileNameFromUrl(weView()->url()));

        connect(dialog, SIGNAL(paintRequested(QPrinter*)), weView(), SLOT(print(QPrinter*)));
    }
    else {
        dialog->printer()->setDocName(QzTools::getFileNameFromUrl(QzTools::frameUrl(frame)));

        connect(dialog, SIGNAL(paintRequested(QPrinter*)), frame, SLOT(print(QPrinter*)));
    }

    dialog->open();
}

void BrowserWindow::savePageScreen()
{
    PageScreen* dialog = new PageScreen(weView(), this);
    dialog->show();
}

void BrowserWindow::resizeEvent(QResizeEvent* event)
{
    m_bookmarksToolbar->setMaximumWidth(width());

    QMainWindow::resizeEvent(event);
}

void BrowserWindow::keyPressEvent(QKeyEvent* event)
{
    if (mApp->plugins()->processKeyPress(Qz::ON_BrowserWindow, this, event)) {
        return;
    }

    int number = -1;
    TabbedWebView* view = weView();

    switch (event->key()) {
    case Qt::Key_Back:
        if (view) {
            view->back();
            event->accept();
        }
        break;

    case Qt::Key_Forward:
        if (view) {
            view->forward();
            event->accept();
        }
        break;

    case Qt::Key_Stop:
        if (view) {
            view->stop();
            event->accept();
        }
        break;

    case Qt::Key_Refresh:
        if (view) {
            view->reload();
            event->accept();
        }
        break;

    case Qt::Key_HomePage:
        goHome();
        event->accept();
        break;

    case Qt::Key_Favorites:
        mApp->browsingLibrary()->showBookmarks(this);
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
        action(QSL("Tools/RssReader"))->trigger();
        event->accept();
        break;

    case Qt::Key_Tools:
        action(QSL("Standard/Preferences"))->trigger();
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
        if (view && event->modifiers() == Qt::ControlModifier) {
            view->zoomIn();
            event->accept();
        }
        break;

    case Qt::Key_I:
        if (event->modifiers() == Qt::ControlModifier) {
            action(QSL("Tools/SiteInfo"))->trigger();
            event->accept();
        }
        break;

    case Qt::Key_U:
        if (event->modifiers() == Qt::ControlModifier) {
            action(QSL("View/PageSource"))->trigger();
            event->accept();
        }
        break;

    case Qt::Key_F:
        if (event->modifiers() == Qt::ControlModifier) {
            action(QSL("Edit/Find"))->trigger();
            event->accept();
        }
        break;

    case Qt::Key_Slash:
        if (m_useSingleKeyShortcuts) {
            action(QSL("Edit/Find"))->trigger();
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
        if (event->modifiers() & Qt::AltModifier && m_useTabNumberShortcuts) {
            if (number == 9) {
                number = m_tabWidget->count();
            }
            m_tabWidget->setCurrentIndex(number - 1);
            return;
        }
        if (event->modifiers() & Qt::ControlModifier && m_useSpeedDialNumberShortcuts) {
            const QUrl url = mApp->plugins()->speedDial()->urlForShortcut(number - 1);
            if (url.isValid()) {
                loadAddress(url);
                return;
            }
        }
        if (event->modifiers() == Qt::NoModifier && m_useSingleKeyShortcuts) {
            if (number == 1)
                m_tabWidget->previousTab();
            if (number == 2)
                m_tabWidget->nextTab();
        }
    }

    QMainWindow::keyPressEvent(event);
}

void BrowserWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (mApp->plugins()->processKeyRelease(Qz::ON_BrowserWindow, this, event)) {
        return;
    }

    QMainWindow::keyReleaseEvent(event);
}

void BrowserWindow::closeEvent(QCloseEvent* event)
{
    if (mApp->isClosing()) {
        return;
    }

    Settings settings;
    int afterLaunch = settings.value("Web-URL-Settings/afterLaunch", 3).toInt();
    bool askOnClose = settings.value("Browser-Tabs-Settings/AskOnClosing", true).toBool();

    if (afterLaunch == 3 && mApp->windowCount() == 1) {
        askOnClose = false;
    }

    if (askOnClose && m_tabWidget->normalTabsCount() > 1) {
        CheckBoxDialog dialog(QDialogButtonBox::Yes | QDialogButtonBox::No, this);
        dialog.setText(tr("There are still %n open tabs and your session won't be stored. \nAre you sure to close this window?", "", m_tabWidget->count()));
        dialog.setCheckBoxText(tr("Don't ask again"));
        dialog.setWindowTitle(tr("There are still open tabs"));
        dialog.setIcon(IconProvider::standardIcon(QStyle::SP_MessageBoxWarning));

        if (dialog.exec() != QDialog::Accepted) {
            event->ignore();
            return;
        }

        if (dialog.isChecked()) {
            settings.setValue("Browser-Tabs-Settings/AskOnClosing", false);
        }
    }

#ifndef Q_OS_MAC
    if (mApp->windowCount() == 1) {
        if (quitApp()) {
            event->accept();
        }
        else {
            event->ignore();
        }

        return;
    }
#endif

    event->accept();
}

SearchToolBar* BrowserWindow::searchToolBar() const
{
    SearchToolBar* toolBar = 0;
    const int searchPos = 2;

    if (m_mainLayout->count() == searchPos + 1) {
        toolBar = qobject_cast<SearchToolBar*>(m_mainLayout->itemAt(searchPos)->widget());
    }

    return toolBar;
}

void BrowserWindow::closeWindow()
{
#ifdef Q_OS_MAC
    close();
    return;
#endif

    if (mApp->windowCount() > 1) {
        close();
    }
}

bool BrowserWindow::quitApp()
{
    if (m_sideBar) {
        saveSideBarWidth();
    }

    if (!mApp->isPrivate()) {
        Settings settings;
        settings.beginGroup("Browser-View-Settings");
        settings.setValue("WindowMaximised", windowState().testFlag(Qt::WindowMaximized));
        settings.setValue("LocationBarWidth", m_navigationToolbar->splitter()->sizes().at(0));
        settings.setValue("WebSearchBarWidth", m_navigationToolbar->splitter()->sizes().at(1));
        settings.setValue("SideBarWidth", m_sideBarWidth);
        settings.setValue("WebViewWidth", m_webViewWidth);

        if (!isFullScreen()) {
            settings.setValue("WindowGeometry", saveGeometry());
        }
        settings.endGroup();
    }

    mApp->quitApplication();
    return true;
}

void BrowserWindow::closeTab()
{
    // Don't close pinned tabs with keyboard shortcuts (Ctrl+W, Ctrl+F4)
    if (weView() && !weView()->webTab()->isPinned()) {
        m_tabWidget->closeTab();
    }
}

QByteArray BrowserWindow::saveState(int version) const
{
#ifdef QZ_WS_X11
    if (QzTools::isPlatformX11()) {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << QMainWindow::saveState(version);
        stream << getCurrentVirtualDesktop();

        return data;
    }
    else
        return QMainWindow::saveState(version);
#else
    return QMainWindow::saveState(version);
#endif
}

bool BrowserWindow::restoreState(const QByteArray &state, int version)
{
#ifdef QZ_WS_X11
    if (QzTools::isPlatformX11()) {
        QByteArray windowState;
        int desktopId = -1;

        QDataStream stream(state);
        stream >> windowState;
        stream >> desktopId;

        moveToVirtualDesktop(desktopId);

        return QMainWindow::restoreState(windowState, version);
    }
    else
        return QMainWindow::restoreState(state, version);

#else
    return QMainWindow::restoreState(state, version);
#endif
}

#ifdef QZ_WS_X11
int BrowserWindow::getCurrentVirtualDesktop() const
{
    if (!QzTools::isPlatformX11())
        return 0;

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

void BrowserWindow::moveToVirtualDesktop(int desktopId)
{
    if (!QzTools::isPlatformX11())
        return;

    // Don't move when window is already visible or it is first app window
    if (desktopId < 0 || isVisible() || m_windowType == Qz::BW_FirstAppWindow) {
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
void BrowserWindow::applyBlurToMainWindow(bool force)
{
    if (mApp->isClosing() || (!force && !isTransparentBackgroundAllowed())) {
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
    topMargin += m_navigationToolbar->isVisible() ? m_navigationToolbar->height() : 0;
    topMargin += m_bookmarksToolbar->isVisible() ? m_bookmarksToolbar->height() : 0;
    topMargin += m_tabWidget->tabBar()->height();

    SearchToolBar* search = searchToolBar();
    if (search) {
        bottomMargin += search->height();
    }

    bottomMargin += statusBar()->isVisible() ? statusBar()->height() : 0;

    QtWin::extendFrameIntoClientArea(this, leftMargin, topMargin, rightMargin, bottomMargin);
}

#if (QT_VERSION < 0x050000)
bool BrowserWindow::winEvent(MSG* message, long* result)
{
#else
bool BrowserWindow::nativeEvent(const QByteArray &eventType, void* _message, long* result)
{
    Q_UNUSED(eventType)
    MSG* message = static_cast<MSG*>(_message);
#endif
    if (message && message->message == WM_DWMCOMPOSITIONCHANGED) {
        Settings settings;
        settings.beginGroup("Browser-View-Settings");
        m_useTransparentBackground = settings.value("useTransparentBackground", false).toBool();
        settings.endGroup();
        if (m_useTransparentBackground && QtWin::isCompositionEnabled()) {
            setUpdatesEnabled(false);

            QtWin::extendFrameIntoClientArea(this, 0, 0, 0, 0);
            QTimer::singleShot(0, this, SLOT(applyBlurToMainWindow()));

            //install event filter
            menuBar()->installEventFilter(this);
            m_navigationToolbar->installEventFilter(this);
            m_bookmarksToolbar->installEventFilter(this);
            statusBar()->installEventFilter(this);

            if (m_sideBar) {
                m_sideBar.data()->installEventFilter(this);
            }

            SearchToolBar* search = searchToolBar();
            if (search) {
                search->installEventFilter(this);
            }

            if (isVisible()) {
                hide();
                show();
            }
            setUpdatesEnabled(true);
        }
        else {
            m_useTransparentBackground = false;
        }
    }
#if (QT_VERSION < 0x050000)
    return QMainWindow::winEvent(message, result);
#else
    return QMainWindow::nativeEvent(eventType, _message, result);
#endif
}

void BrowserWindow::paintEvent(QPaintEvent* event)
{
    if (isTransparentBackgroundAllowed()) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.fillRect(event->rect(), QColor(0, 0, 0, 0));
    }

    QMainWindow::paintEvent(event);
}

bool BrowserWindow::eventFilter(QObject* object, QEvent* event)
{
    switch (event->type()) {
    case QEvent::Hide:
        if (object == m_navigationContainer) {
            m_navigationToolbar->removeEventFilter(this);
            m_bookmarksToolbar->removeEventFilter(this);
            break;
        }
    case QEvent::Show:
        if (object == m_navigationContainer) {
            m_navigationToolbar->installEventFilter(this);
            m_bookmarksToolbar->installEventFilter(this);
            break;
        }
    case QEvent::Resize:
    case QEvent::DeferredDelete:
        if (object == m_navigationContainer) {
            break;
        }
        applyBlurToMainWindow();
        break;
    default:
        break;
    }

    return QMainWindow::eventFilter(object, event);
}
#endif
