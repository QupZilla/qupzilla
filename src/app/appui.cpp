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
#include "autofillmodel.h"
#include "bookmarkstoolbar.h"
#include "locationbar.h"

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

        if ( startingAfterCrash || (addTab && afterLaunch == 2) ) {
            mApp->restoreStateSlot(this);
            addTab = false;
        }
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

    QApplication::restoreOverrideCursor();
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
    m_navigation->addAction(m_actionExitFullscreen);
    m_navigation->addWidget(m_supMenu);
    m_navigation->addWidget(new QLabel()); //Elegant spacer -,-
    m_navigation->setContextMenuPolicy(Qt::CustomContextMenu);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setMaximumSize(QSize(150, 16));
    m_privateBrowsing = new QLabel(this);
    m_privateBrowsing->setPixmap(QPixmap(":/icons/locationbar/privatebrowsing.png"));
    m_privateBrowsing->setVisible(false);
    m_privateBrowsing->setToolTip(tr("Private Browsing Enabled"));
    m_allowFlashIcon = new QLabel(this);
    m_allowFlashIcon->setPixmap(QPixmap(":/icons/menu/flash.png"));
    m_allowFlashIcon->setVisible(false);
    m_allowFlashIcon->setToolTip(tr("Flash Plugin Enabled"));
    m_ipLabel = new QLabel(this);
    m_ipLabel->setStyleSheet("padding-right: 5px;");
    m_ipLabel->setToolTip(tr("IP Address of current page"));

    statusBar()->insertPermanentWidget(0, m_progressBar);
    statusBar()->insertPermanentWidget(1, m_ipLabel);
    statusBar()->insertPermanentWidget(2, m_privateBrowsing);
    statusBar()->insertPermanentWidget(3, m_allowFlashIcon);

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
    m_menuFile->addAction(QIcon::fromTheme("window-new"), tr("New Window"), this, SLOT(newWindow()))->setShortcut(QKeySequence("Ctrl+N"));
    m_menuFile->addAction(QIcon(":/icons/menu/popup.png"), tr("New Tab"), this, SLOT(addTab()))->setShortcut(QKeySequence("Ctrl+T"));
    m_menuFile->addAction(tr("Open Location"), this, SLOT(openLocation()))->setShortcut(QKeySequence("Ctrl+L"));
    m_menuFile->addAction(QIcon::fromTheme("document-open"), tr("Open File"), this, SLOT(openFile()))->setShortcut(QKeySequence("Ctrl+O"));
    m_menuFile->addAction(tr("Close Tab"), m_tabWidget, SLOT(closeTab()))->setShortcut(QKeySequence("Ctrl+W"));
    m_menuFile->addAction(QIcon::fromTheme("window-close"), tr("Close Window"), this, SLOT(close()))->setShortcut(QKeySequence("Ctrl+Shift+W"));
    m_menuFile->addSeparator();
    m_menuFile->addAction(QIcon::fromTheme("document-save"), tr("Save Page As..."), this, SLOT(savePage()))->setShortcut(QKeySequence("Ctrl+S"));
    m_menuFile->addAction(tr("Send Link..."), this, SLOT(sendLink()));
    m_menuFile->addAction(QIcon::fromTheme("document-print"), tr("Print"), this, SLOT(printPage()));
    m_menuFile->addSeparator();
    m_menuFile->addAction(QIcon::fromTheme("application-exit"), tr("Quit"), this, SLOT(quitApp()))->setShortcut(QKeySequence("Ctrl+Q"));
    menuBar()->addMenu(m_menuFile);

    m_menuEdit = new QMenu(tr("Edit"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-undo"), tr("Undo"))->setShortcut(QKeySequence("Ctrl+Z"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-redo"), tr("Redo"))->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-cut"), tr("Cut"))->setShortcut(QKeySequence("Ctrl+X"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this, SLOT(copy()))->setShortcut(QKeySequence("Ctrl+C"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-paste"), tr("Paste"))->setShortcut(QKeySequence("Ctrl+V"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"))->setShortcut(QKeySequence("Del"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-select-all"), tr("Select All"), this, SLOT(selectAll()))->setShortcut(QKeySequence("Ctrl+A"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-find"), tr("Find"), this, SLOT(searchOnPage()))->setShortcut(QKeySequence("Ctrl+F"));
    menuBar()->addMenu(m_menuEdit);

    m_menuView = new QMenu(tr("View"));
    m_actionShowToolbar = new QAction(tr("Navigation Toolbar"), this);
    m_actionShowToolbar->setCheckable(true);
    connect(m_actionShowToolbar, SIGNAL(triggered(bool)), this, SLOT(showNavigationToolbar()));
    m_actionShowBookmarksToolbar = new QAction(tr("Bookmarks Toolbar"), this);
    m_actionShowBookmarksToolbar->setCheckable(true);
    connect(m_actionShowBookmarksToolbar, SIGNAL(triggered(bool)), this, SLOT(showBookmarksToolbar()));
    m_actionShowStatusbar = new QAction(tr("Status Bar"), this);
    m_actionShowStatusbar->setCheckable(true);
    connect(m_actionShowStatusbar, SIGNAL(triggered(bool)), this, SLOT(showStatusbar()));
    m_actionShowMenubar = new QAction(tr("Menu Bar"), this);
    m_actionShowMenubar->setCheckable(true);
    connect(m_actionShowMenubar, SIGNAL(triggered(bool)), this, SLOT(showMenubar()));
    m_actionShowFullScreen = new QAction(tr("Fullscreen"), this);
    m_actionShowFullScreen->setCheckable(true);
    m_actionShowFullScreen->setShortcut(QKeySequence("F11"));
    connect(m_actionShowFullScreen, SIGNAL(triggered(bool)), this, SLOT(fullScreen(bool)));
    m_actionStop = new QAction(
#ifdef Q_WS_X11
            style()->standardIcon(QStyle::SP_BrowserStop)
#else
            QIcon(":/icons/faenza/stop.png")
#endif
            , tr("Stop"), this);
    connect(m_actionStop, SIGNAL(triggered()), this, SLOT(stop()));
    m_actionStop->setShortcut(QKeySequence("Esc"));
    m_actionReload = new QAction(
#ifdef Q_WS_X11
            style()->standardIcon(QStyle::SP_BrowserReload)
#else
            QIcon(":/icons/faenza/reload.png")
#endif
            , tr("Reload"), this);
    connect(m_actionReload, SIGNAL(triggered()), this, SLOT(reload()));
    m_actionReload->setShortcut(QKeySequence("Ctrl+R"));
    QAction* actionEncoding = new QAction(tr("Character Encoding"), this);
    m_menuEncoding = new QMenu(this);
    actionEncoding->setMenu(m_menuEncoding);
    connect(m_menuEncoding, SIGNAL(aboutToShow()), this, SLOT(aboutToShowEncodingMenu()));

    m_menuView->addAction(m_actionShowMenubar);
    m_menuView->addAction(m_actionShowToolbar);
    m_menuView->addAction(m_actionShowBookmarksToolbar);
    m_menuView->addAction(m_actionShowStatusbar);
    m_menuView->addSeparator();
    m_menuView->addAction(m_actionStop);
    m_menuView->addAction(m_actionReload);
    m_menuView->addSeparator();
    m_menuView->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom In"), this, SLOT(zoomIn()))->setShortcut(QKeySequence("Ctrl++"));
    m_menuView->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom Out"), this, SLOT(zoomOut()))->setShortcut(QKeySequence("Ctrl+-"));
    m_menuView->addAction(QIcon::fromTheme("zoom-original"), tr("Reset"), this, SLOT(zoomReset()))->setShortcut(QKeySequence("Ctrl+0"));
    m_menuView->addSeparator();
    m_menuView->addAction(actionEncoding);
    m_menuView->addSeparator();
    m_menuView->addAction(QIcon::fromTheme("text-html"), tr("Page Source"), this, SLOT(showSource()))->setShortcut(QKeySequence("Ctrl+U"));
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
                  "QSplitter::handle{background-color:transparent;}"
                  );

}

