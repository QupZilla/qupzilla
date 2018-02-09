/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2018 David Rosca <nowrep@gmail.com>
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
#include "mainmenu.h"
#include "siteinfo.h"
#include "tabwidget.h"
#include "historymenu.h"
#include "aboutdialog.h"
#include "preferences.h"
#include "iconprovider.h"
#include "cookiemanager.h"
#include "bookmarksmenu.h"
#include "tabbedwebview.h"
#include "browserwindow.h"
#include "downloadmanager.h"
#include "mainapplication.h"
#include "clearprivatedata.h"
#include "qzsettings.h"
#include "pluginproxy.h"
#include "webinspector.h"
#include "sessionmanager.h"
#include "statusbar.h"

#include <QApplication>
#include <QMetaObject>
#include <QWebEnginePage>
#include <QMenuBar>
#include <QDesktopServices>

#ifdef Q_OS_MACOS
extern void qt_mac_set_dock_menu(QMenu* menu);
#endif

MainMenu::MainMenu(BrowserWindow* window, QWidget* parent)
    : QMenu(parent)
    , m_window(window)
{
    Q_ASSERT(m_window);

    init();
}

void MainMenu::setWindow(BrowserWindow* window)
{
    Q_ASSERT(window);

    m_window = window;
    addActionsToWindow();
}

void MainMenu::initMenuBar(QMenuBar* menuBar) const
{
    menuBar->addMenu(m_menuFile);
    menuBar->addMenu(m_menuEdit);
    menuBar->addMenu(m_menuView);
    menuBar->addMenu(m_menuHistory);
    menuBar->addMenu(m_menuBookmarks);
    menuBar->addMenu(m_menuTools);
    menuBar->addMenu(m_menuHelp);
}

void MainMenu::initSuperMenu(QMenu* superMenu) const
{
    superMenu->addAction(m_actions[QSL("File/NewTab")]);
    superMenu->addAction(m_actions[QSL("File/NewWindow")]);
    superMenu->addAction(m_actions[QSL("File/NewPrivateWindow")]);
    superMenu->addAction(m_actions[QSL("File/OpenFile")]);
    if (mApp->sessionManager()) {
        superMenu->addSeparator();
        QMenu* sessionsSubmenu = new QMenu(tr("Sessions"));
        connect(sessionsSubmenu, SIGNAL(aboutToShow()), mApp->sessionManager(), SLOT(aboutToShowSessionsMenu()));
        superMenu->addMenu(sessionsSubmenu);
        superMenu->addAction(m_actions[QSL("File/SessionManager")]);
    }
    superMenu->addSeparator();
    superMenu->addAction(m_actions[QSL("File/SendLink")]);
    superMenu->addAction(m_actions[QSL("File/Print")]);
    superMenu->addSeparator();
    superMenu->addAction(m_actions[QSL("Edit/SelectAll")]);
    superMenu->addAction(m_actions[QSL("Edit/Find")]);
    superMenu->addSeparator();
    superMenu->addAction(m_menuHistory->actions().at(3));
    superMenu->addAction(m_menuBookmarks->actions().at(2));
    superMenu->addSeparator();
    superMenu->addMenu(m_menuView);
    superMenu->addMenu(m_menuHistory);
    superMenu->addMenu(m_menuBookmarks);
    superMenu->addMenu(m_menuTools);
    superMenu->addMenu(m_menuHelp);
    superMenu->addSeparator();
    superMenu->addAction(m_actions[QSL("Standard/Preferences")]);
    superMenu->addAction(m_actions[QSL("Standard/About")]);
    superMenu->addSeparator();
    superMenu->addAction(m_actions[QSL("Standard/Quit")]);

    connect(superMenu, &QMenu::aboutToShow, this, &MainMenu::aboutToShowSuperMenu);
}

QAction* MainMenu::action(const QString &name) const
{
    Q_ASSERT(m_actions.value(name));

    return m_actions.value(name);
}

void MainMenu::showAboutDialog()
{
    AboutDialog* dialog = new AboutDialog(m_window);
    dialog->open();
}

void MainMenu::showPreferences()
{
    if (!m_preferences)
        m_preferences = new Preferences(m_window);

    m_preferences->show();
    m_preferences->raise();
    m_preferences->activateWindow();
}

void MainMenu::quitApplication()
{
    mApp->quitApplication();
}

void MainMenu::newTab()
{
    callSlot("addTab");
}

void MainMenu::newWindow()
{
    mApp->createWindow(Qz::BW_NewWindow);
}

void MainMenu::newPrivateWindow()
{
    mApp->startPrivateBrowsing();
}

void MainMenu::openLocation()
{
    callSlot("openLocation");
}

void MainMenu::openFile()
{
    callSlot("openFile");
}

void MainMenu::closeWindow()
{
    callSlot("closeWindow");
}

void MainMenu::savePageAs()
{
    if (m_window) {
        QMetaObject::invokeMethod(m_window->weView(), "savePageAs");
    }
}

void MainMenu::sendLink()
{
    const QUrl mailUrl = QUrl::fromEncoded("mailto:%20?body=" + QUrl::toPercentEncoding(m_window->weView()->url().toEncoded()) + "&subject=" + QUrl::toPercentEncoding(m_window->weView()->title()));
    QDesktopServices::openUrl(mailUrl);
}

void MainMenu::printPage()
{
    callSlot("printPage");
}

void MainMenu::editUndo()
{
    if (m_window) {
        m_window->weView()->editUndo();
    }
}

void MainMenu::editRedo()
{
    if (m_window) {
        m_window->weView()->editRedo();
    }
}

void MainMenu::editCut()
{
    if (m_window) {
        m_window->weView()->editCut();
    }
}

void MainMenu::editCopy()
{
    if (m_window) {
        m_window->weView()->editCopy();
    }
}

void MainMenu::editPaste()
{
    if (m_window) {
        m_window->weView()->editPaste();
    }
}

void MainMenu::editSelectAll()
{
    if (m_window) {
        m_window->weView()->editSelectAll();
    }
}

void MainMenu::editFind()
{
    callSlot("searchOnPage");
}

void MainMenu::showStatusBar()
{
    if (m_window) {
        m_window->toggleShowStatusBar();
    }
}

void MainMenu::stop()
{
    if (m_window) {
        m_window->weView()->stop();
    }
}

void MainMenu::reload()
{
    if (m_window) {
        m_window->weView()->reload();
    }
}

void MainMenu::zoomIn()
{
    if (m_window) {
        m_window->weView()->zoomIn();
    }
}

void MainMenu::zoomOut()
{
    if (m_window) {
        m_window->weView()->zoomOut();
    }
}

void MainMenu::zoomReset()
{
    if (m_window) {
        m_window->weView()->zoomReset();
    }
}

void MainMenu::showPageSource()
{
    callSlot("showSource");
}

void MainMenu::showFullScreen()
{
    if (m_window) {
        m_window->toggleFullScreen();
    }
}

void MainMenu::webSearch()
{
    callSlot("webSearch");
}

void MainMenu::showSiteInfo()
{
    if (m_window && SiteInfo::canShowSiteInfo(m_window->weView()->url())) {
        SiteInfo* info = new SiteInfo(m_window->weView());
        info->show();
    }
}

void MainMenu::showDownloadManager()
{
    DownloadManager* m = mApp->downloadManager();
    m->show();
    m->raise();
}

void MainMenu::showCookieManager()
{
    CookieManager* m = new CookieManager(m_window);
    m->show();
    m->raise();
}

void MainMenu::toggleWebInspector()
{
    callSlot("toggleWebInspector");
}

void MainMenu::showClearRecentHistoryDialog()
{
    ClearPrivateData* dialog = new ClearPrivateData(m_window);
    dialog->open();
}

void MainMenu::aboutQt()
{
    QApplication::aboutQt();
}

void MainMenu::showInfoAboutApp()
{
    if (m_window) {
        m_window->tabWidget()->addView(QUrl(QSL("qupzilla:about")), Qz::NT_CleanSelectedTab);
    }
}

void MainMenu::showConfigInfo()
{
    if (m_window) {
        m_window->tabWidget()->addView(QUrl(QSL("qupzilla:config")), Qz::NT_CleanSelectedTab);
    }
}

void MainMenu::reportIssue()
{
    if (m_window) {
        m_window->tabWidget()->addView(QUrl(QSL("qupzilla:reportbug")), Qz::NT_CleanSelectedTab);
    }
}

void MainMenu::restoreClosedTab()
{
    if (m_window) {
        m_window->tabWidget()->restoreClosedTab();
    }
}

void MainMenu::aboutToShowFileMenu()
{
#ifndef Q_OS_MACOS
    m_actions[QSL("File/CloseWindow")]->setEnabled(mApp->windowCount() > 1);
#endif
}

void MainMenu::aboutToShowViewMenu()
{
    if (!m_window) {
        return;
    }

    m_actions[QSL("View/ShowStatusBar")]->setChecked(m_window->statusBar()->isVisible());
    m_actions[QSL("View/FullScreen")]->setChecked(m_window->isFullScreen());
}

void MainMenu::aboutToShowEditMenu()
{
    if (!m_window) {
        return;
    }

    WebView* view = m_window->weView();

    m_actions[QSL("Edit/Undo")]->setEnabled(view->pageAction(QWebEnginePage::Undo)->isEnabled());
    m_actions[QSL("Edit/Redo")]->setEnabled(view->pageAction(QWebEnginePage::Redo)->isEnabled());
    m_actions[QSL("Edit/Cut")]->setEnabled(view->pageAction(QWebEnginePage::Cut)->isEnabled());
    m_actions[QSL("Edit/Copy")]->setEnabled(view->pageAction(QWebEnginePage::Copy)->isEnabled());
    m_actions[QSL("Edit/Paste")]->setEnabled(view->pageAction(QWebEnginePage::Paste)->isEnabled());
    m_actions[QSL("Edit/SelectAll")]->setEnabled(view->pageAction(QWebEnginePage::SelectAll)->isEnabled());
}

void MainMenu::aboutToShowToolsMenu()
{
    if (!m_window)
        return;

    m_actions[QSL("Tools/SiteInfo")]->setEnabled(SiteInfo::canShowSiteInfo(m_window->weView()->url()));

    m_submenuExtensions->clear();
    mApp->plugins()->populateExtensionsMenu(m_submenuExtensions);

    m_submenuExtensions->menuAction()->setVisible(!m_submenuExtensions->actions().isEmpty());
}

void MainMenu::aboutToShowSuperMenu()
{
    if (!m_window) {
        return;
    }

    WebView* view = m_window->weView();

    m_actions[QSL("Edit/Find")]->setEnabled(true);
    m_actions[QSL("Edit/SelectAll")]->setEnabled(view->pageAction(QWebEnginePage::SelectAll)->isEnabled());
}

void MainMenu::aboutToShowToolbarsMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());
    Q_ASSERT(menu);

    if (m_window) {
        menu->clear();
        m_window->createToolbarsMenu(menu);
    }
}

void MainMenu::aboutToShowSidebarsMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());
    Q_ASSERT(menu);

    if (m_window) {
        m_window->createSidebarsMenu(menu);
    }
}

void MainMenu::aboutToShowEncodingMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());
    Q_ASSERT(menu);

    if (m_window) {
        menu->clear();
        m_window->createEncodingMenu(menu);
    }
}

void MainMenu::init()
{
#define ADD_ACTION(name, menu, icon, trName, slot, shortcut) \
    action = menu->addAction(icon, trName); \
    action->setShortcut(QKeySequence(QSL(shortcut))); \
    connect(action, SIGNAL(triggered()), this, slot); \
    m_actions[QSL(name)] = action

#define ADD_CHECKABLE_ACTION(name, menu, icon, trName, slot, shortcut) \
    action = menu->addAction(icon, trName); \
    action->setShortcut(QKeySequence(QSL(shortcut))); \
    action->setCheckable(true); \
    connect(action, SIGNAL(triggered(bool)), this, slot); \
    m_actions[QSL(name)] = action

    // Standard actions - needed on Mac to be placed correctly in "application" menu
    QAction* action = new QAction(QIcon::fromTheme(QSL("help-about")), tr("&About QupZilla"), this);
    action->setMenuRole(QAction::AboutRole);
    connect(action, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    m_actions[QSL("Standard/About")] = action;

    action = new QAction(IconProvider::settingsIcon(), tr("Pr&eferences"), this);
    action->setMenuRole(QAction::PreferencesRole);
    action->setShortcut(QKeySequence(QKeySequence::Preferences));
    connect(action, SIGNAL(triggered()), this, SLOT(showPreferences()));
    m_actions[QSL("Standard/Preferences")] = action;

    action = new QAction(QIcon::fromTheme(QSL("application-exit")), tr("Quit"), this);
    action->setMenuRole(QAction::QuitRole);
    // shortcut set from browserwindow
    connect(action, SIGNAL(triggered()), this, SLOT(quitApplication()));
    m_actions[QSL("Standard/Quit")] = action;

    // File menu
    m_menuFile = new QMenu(tr("&File"));
    connect(m_menuFile, SIGNAL(aboutToShow()), this, SLOT(aboutToShowFileMenu()));

    ADD_ACTION("File/NewTab", m_menuFile, IconProvider::newTabIcon(), tr("New Tab"), SLOT(newTab()), "Ctrl+T");
    ADD_ACTION("File/NewWindow", m_menuFile, IconProvider::newWindowIcon(), tr("&New Window"), SLOT(newWindow()), "Ctrl+N");
    ADD_ACTION("File/NewPrivateWindow", m_menuFile, IconProvider::privateBrowsingIcon(), tr("New &Private Window"), SLOT(newPrivateWindow()), "Ctrl+Shift+P");
    ADD_ACTION("File/OpenLocation", m_menuFile, QIcon::fromTheme(QSL("document-open-remote")), tr("Open Location"), SLOT(openLocation()), "Ctrl+L");
    ADD_ACTION("File/OpenFile", m_menuFile, QIcon::fromTheme(QSL("document-open")), tr("Open &File..."), SLOT(openFile()), "Ctrl+O");
    ADD_ACTION("File/CloseWindow", m_menuFile, QIcon::fromTheme(QSL("window-close")), tr("Close Window"), SLOT(closeWindow()), "Ctrl+Shift+W");
    m_menuFile->addSeparator();

    if (mApp->sessionManager()) {
        QMenu* sessionsSubmenu = new QMenu(tr("Sessions"));
        connect(sessionsSubmenu, SIGNAL(aboutToShow()), mApp->sessionManager(), SLOT(aboutToShowSessionsMenu()));
        m_menuFile->addMenu(sessionsSubmenu);
        action = new QAction(tr("Session Manager"), this);
        connect(action, SIGNAL(triggered()), mApp->sessionManager(), SLOT(openSessionManagerDialog()));
        m_actions[QSL("File/SessionManager")] = action;
        m_menuFile->addAction(action);
        m_menuFile->addSeparator();
    }

    ADD_ACTION("File/SavePageAs", m_menuFile, QIcon::fromTheme(QSL("document-save")), tr("&Save Page As..."), SLOT(savePageAs()), "Ctrl+S");
    ADD_ACTION("File/SendLink", m_menuFile, QIcon::fromTheme(QSL("mail-message-new")), tr("Send Link..."), SLOT(sendLink()), "");
    ADD_ACTION("File/Print", m_menuFile, QIcon::fromTheme(QSL("document-print")), tr("&Print..."), SLOT(printPage()), "Ctrl+P");
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actions[QSL("Standard/Quit")]);

    // Edit menu
    m_menuEdit = new QMenu(tr("&Edit"));
    connect(m_menuEdit, SIGNAL(aboutToShow()), this, SLOT(aboutToShowEditMenu()));

    ADD_ACTION("Edit/Undo", m_menuEdit, QIcon::fromTheme(QSL("edit-undo")), tr("&Undo"), SLOT(editUndo()), "Ctrl+Z");
    action->setShortcutContext(Qt::WidgetShortcut);
    ADD_ACTION("Edit/Redo", m_menuEdit, QIcon::fromTheme(QSL("edit-redo")), tr("&Redo"), SLOT(editRedo()), "Ctrl+Shift+Z");
    action->setShortcutContext(Qt::WidgetShortcut);
    m_menuEdit->addSeparator();
    ADD_ACTION("Edit/Cut", m_menuEdit, QIcon::fromTheme(QSL("edit-cut")), tr("&Cut"), SLOT(editCut()), "Ctrl+X");
    action->setShortcutContext(Qt::WidgetShortcut);
    ADD_ACTION("Edit/Copy", m_menuEdit, QIcon::fromTheme(QSL("edit-copy")), tr("C&opy"), SLOT(editCopy()), "Ctrl+C");
    action->setShortcutContext(Qt::WidgetShortcut);
    ADD_ACTION("Edit/Paste", m_menuEdit, QIcon::fromTheme(QSL("edit-paste")), tr("&Paste"), SLOT(editPaste()), "Ctrl+V");
    action->setShortcutContext(Qt::WidgetShortcut);
    m_menuEdit->addSeparator();
    ADD_ACTION("Edit/SelectAll", m_menuEdit, QIcon::fromTheme(QSL("edit-select-all")), tr("Select &All"), SLOT(editSelectAll()), "Ctrl+A");
    action->setShortcutContext(Qt::WidgetShortcut);
    ADD_ACTION("Edit/Find", m_menuEdit, QIcon::fromTheme(QSL("edit-find")), tr("&Find"), SLOT(editFind()), "Ctrl+F");
    action->setShortcutContext(Qt::WidgetShortcut);
    m_menuEdit->addSeparator();

    // View menu
    m_menuView = new QMenu(tr("&View"));
    connect(m_menuView, SIGNAL(aboutToShow()), this, SLOT(aboutToShowViewMenu()));

    QMenu* toolbarsMenu = new QMenu(tr("Toolbars"));
    connect(toolbarsMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowToolbarsMenu()));
    QMenu* sidebarMenu = new QMenu(tr("Sidebar"));
    connect(sidebarMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowSidebarsMenu()));
    QMenu* encodingMenu = new QMenu(tr("Character &Encoding"));
    connect(encodingMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowEncodingMenu()));

    // Create menus to make shortcuts available even before first showing the menu
    m_window->createToolbarsMenu(toolbarsMenu);
    m_window->createSidebarsMenu(sidebarMenu);

    m_menuView->addMenu(toolbarsMenu);
    m_menuView->addMenu(sidebarMenu);
    ADD_CHECKABLE_ACTION("View/ShowStatusBar", m_menuView, QIcon(), tr("Sta&tus Bar"), SLOT(showStatusBar()), "");
    m_menuView->addSeparator();
    ADD_ACTION("View/Stop", m_menuView, QIcon::fromTheme(QSL("process-stop")), tr("&Stop"), SLOT(stop()), "Esc");
    ADD_ACTION("View/Reload", m_menuView, QIcon::fromTheme(QSL("view-refresh")), tr("&Reload"), SLOT(reload()), "F5");
    m_menuView->addSeparator();
    ADD_ACTION("View/ZoomIn", m_menuView, QIcon::fromTheme(QSL("zoom-in")), tr("Zoom &In"), SLOT(zoomIn()), "Ctrl++");
    ADD_ACTION("View/ZoomOut", m_menuView, QIcon::fromTheme(QSL("zoom-out")), tr("Zoom &Out"), SLOT(zoomOut()), "Ctrl+-");
    ADD_ACTION("View/ZoomReset", m_menuView, QIcon::fromTheme(QSL("zoom-original")), tr("Reset"), SLOT(zoomReset()), "Ctrl+0");
    m_menuView->addSeparator();
    m_menuView->addMenu(encodingMenu);
    m_menuView->addSeparator();
    ADD_ACTION("View/PageSource", m_menuView, QIcon::fromTheme(QSL("text-html")), tr("&Page Source"), SLOT(showPageSource()), "Ctrl+U");
    action->setShortcutContext(Qt::WidgetShortcut);
    ADD_CHECKABLE_ACTION("View/FullScreen", m_menuView, QIcon(), tr("&FullScreen"), SLOT(showFullScreen()), "F11");

    // Tools menu
    m_menuTools = new QMenu(tr("&Tools"));
    connect(m_menuTools, SIGNAL(aboutToShow()), this, SLOT(aboutToShowToolsMenu()));

    ADD_ACTION("Tools/WebSearch", m_menuTools, QIcon(), tr("&Web Search"), SLOT(webSearch()), "Ctrl+K");
    ADD_ACTION("Tools/SiteInfo", m_menuTools, QIcon::fromTheme(QSL("dialog-information")), tr("Site &Info"), SLOT(showSiteInfo()), "Ctrl+I");
    action->setShortcutContext(Qt::WidgetShortcut);
    m_menuTools->addSeparator();
    ADD_ACTION("Tools/DownloadManager", m_menuTools, QIcon(), tr("&Download Manager"), SLOT(showDownloadManager()), "Ctrl+Y");
    ADD_ACTION("Tools/CookiesManager", m_menuTools, QIcon(), tr("&Cookies Manager"), SLOT(showCookieManager()), "");
    ADD_ACTION("Tools/WebInspector", m_menuTools, QIcon(), tr("Web In&spector"), SLOT(toggleWebInspector()), "Ctrl+Shift+I");
    ADD_ACTION("Tools/ClearRecentHistory", m_menuTools, QIcon::fromTheme(QSL("edit-clear")), tr("Clear Recent &History"), SLOT(showClearRecentHistoryDialog()), "Ctrl+Shift+Del");

    if (!WebInspector::isEnabled())
        m_actions.value(QSL("Tools/WebInspector"))->setVisible(false);

    m_submenuExtensions = new QMenu(tr("&Extensions"));
    m_submenuExtensions->menuAction()->setVisible(false);
    m_menuTools->addMenu(m_submenuExtensions);
    m_menuTools->addSeparator();

    // Help menu
    m_menuHelp = new QMenu(tr("&Help"));

#ifndef Q_OS_MACOS
    ADD_ACTION("Help/AboutQt", m_menuHelp, QIcon(), tr("About &Qt"), SLOT(aboutQt()), "");
    m_menuHelp->addAction(m_actions[QSL("Standard/About")]);
    m_menuHelp->addSeparator();
#endif

    ADD_ACTION("Help/InfoAboutApp", m_menuHelp, QIcon::fromTheme(QSL("help-contents")), tr("Information about application"), SLOT(showInfoAboutApp()), "");
    ADD_ACTION("Help/ConfigInfo", m_menuHelp, QIcon(), tr("Configuration Information"), SLOT(showConfigInfo()), "");
    ADD_ACTION("Help/ReportIssue", m_menuHelp, QIcon(), tr("Report &Issue"), SLOT(reportIssue()), "");

    m_actions[QSL("Help/InfoAboutApp")]->setShortcut(QKeySequence(QKeySequence::HelpContents));

    // History menu
    m_menuHistory = new HistoryMenu();
    m_menuHistory->setMainWindow(m_window);

    // Bookmarks menu
    m_menuBookmarks = new BookmarksMenu();
    m_menuBookmarks->setMainWindow(m_window);

    // Other actions
    action = new QAction(QIcon::fromTheme(QSL("user-trash")), tr("Restore &Closed Tab"), this);
    action->setShortcut(QKeySequence(QSL("Ctrl+Shift+T")));
    connect(action, SIGNAL(triggered()), this, SLOT(restoreClosedTab()));
    m_actions[QSL("Other/RestoreClosedTab")] = action;

#ifdef Q_OS_MACOS
    m_actions[QSL("View/FullScreen")]->setShortcut(QKeySequence(QSL("Ctrl+Meta+F")));

    // Add standard actions to File Menu (as it won't be ever cleared) and Mac menubar should move them to "Application" menu
    m_menuFile->addAction(m_actions[QSL("Standard/About")]);
    m_menuFile->addAction(m_actions[QSL("Standard/Preferences")]);

    // Prevent ConfigInfo action to be detected as "Preferences..." action in Mac menubar
    m_actions[QSL("Help/ConfigInfo")]->setMenuRole(QAction::NoRole);

    // Create Dock menu
    QMenu* dockMenu = new QMenu(0);
    dockMenu->addAction(m_actions[QSL("File/NewTab")]);
    dockMenu->addAction(m_actions[QSL("File/NewWindow")]);
    dockMenu->addAction(m_actions[QSL("File/NewPrivateWindow")]);
    qt_mac_set_dock_menu(dockMenu);
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    m_menuEdit->addAction(m_actions[QSL("Standard/Preferences")]);
#elif !defined(Q_OS_MACOS)
    m_menuTools->addAction(m_actions[QSL("Standard/Preferences")]);
#endif

    addActionsToWindow();
}

void MainMenu::addActionsToWindow()
{
    // Make shortcuts available even in fullscreen (hidden menu)

    QList<QAction*> actions;
    actions << m_menuFile->actions();
    actions << m_menuEdit->actions();
    actions << m_menuView->actions();
    actions << m_menuTools->actions();
    actions << m_menuHelp->actions();
    actions << m_menuHistory->actions();
    actions << m_menuBookmarks->actions();
    actions << m_actions[QSL("Other/RestoreClosedTab")];

    for (int i = 0; i < actions.size(); ++i) {
        QAction* action = actions.at(i);
        if (action->menu()) {
            actions += action->menu()->actions();
        }
        m_window->addAction(action);
    }
}

void MainMenu::callSlot(const char* slot)
{
    if (m_window) {
        QMetaObject::invokeMethod(m_window, slot);
    }
}
