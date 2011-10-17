/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include <QMenuBar>
#include <QTranslator>
#include <QDesktopWidget>
#include <QDebug>
#include <QUrl>
#include <QWebView>
#include <QWebFrame>
#include <QWebHistory>
#include <QtNetwork/QtNetwork>
#include <QtSql/QtSql>
#include <QMessageBox>
#include <QFile>
#include <QMovie>
#include <QDesktopServices>
#include <QStatusBar>
#include <QSplitter>
#include <QPushButton>
#include <QProgressBar>
#include <QPrintPreviewDialog>
#include <QToolButton>
#include <QWebInspector>
#include <QPointer>
#include "qwebkitversion.h"

#include "webtab.h"
#include "webview.h"
#include "tabwidget.h"
#include "mainapplication.h"
#include "locationbar.h"

class TabWidget;
class WebView;
class LineEdit;
class SearchToolBar;
class BookmarksToolbar;
class AutoFillModel;
class MainApplication;
class WebTab;
class AdBlockIcon;
class SideBar;
class ProgressBar;
class StatusBarMessage;
class NavigationBar;
class ClickableLabel;
class WebInspectorDockWidget;
class QupZilla : public QMainWindow
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

    static const QIcon qupzillaIcon();

    explicit QupZilla(bool m_tryRestore=true, QUrl startUrl=QUrl());
    ~QupZilla();

    void refreshAddressBar();
    void addBookmark(const QUrl &url, const QString &title, const QIcon &icon);
    void installTranslator();
    void loadSettings();
    void showNavigationWithFullscreen();
    void saveSideBarWidth();

    inline WebView* weView() const { WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_tabWidget->currentIndex())); if (!webTab) return 0; return webTab->view(); }
    inline WebView* weView(int index) const { WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index)); if (!webTab) return 0; return webTab->view(); }
    inline LocationBar* locationBar() { return (LocationBar*) m_tabWidget->locationBars()->currentWidget(); }
    inline TabWidget* tabWidget() { return m_tabWidget; }
    inline BookmarksToolbar* bookmarksToolbar() { return m_bookmarksToolbar; }
    inline StatusBarMessage* statusBarMessage() { return m_statusBarMessage; }
    inline NavigationBar* navigationBar() { return m_navigationBar; }

    inline ProgressBar* progressBar(){ return m_progressBar; }
    inline QString activeProfil(){ return m_activeProfil; }
    inline QString activeLanguage(){ return m_activeLanguage; }
    inline QLabel* ipLabel(){ return m_ipLabel; }
    inline QColor menuTextColor() { return m_menuTextColor; }
    inline QMenu* menuHelp() { return m_menuHelp; }
    inline QAction* actionRestoreTab() { return m_actionRestoreTab; }
    inline QMenu* superMenu() { return m_superMenu; }

signals:
    void loadHistory();
    void startingCompleted();
    void message(MainApplication::MessageType mes, bool state);
    void setWebViewMouseTracking(bool state);

public slots:
    void setWindowTitle(const QString &t);

    void showWebInspector();
    void showBookmarksToolbar();
    void loadActionUrl();
    void loadActionUrlInNewTab();
    void bookmarkPage();
    void loadAddress(const QUrl &url);
    void showSource(const QString& selectedHtml = "");
    void showPageInfo();
    void receiveMessage(MainApplication::MessageType mes, bool state);

private slots:
    void postLaunch();
    void goNext() { weView()->forward(); }
    void goBack() { weView()->back(); }
    void goHome() { loadAddress(m_homepage); }
    void stop() { weView()->stop(); }
    void reload() { weView()->reload(); }
    void reloadByPassCache() { weView()->page()->triggerAction(QWebPage::ReloadAndBypassCache); }
    void urlEnter();
    void aboutQupZilla();
    void addTab() { m_tabWidget->addView(QUrl(), tr("New tab"), TabWidget::NewTab, true); }
    void printPage();
    void savePageScreen();

    void aboutToShowHistoryMenu(bool loadHistory = true);
    void aboutToShowClosedTabsMenu();
    void aboutToShowBookmarksMenu();
    void aboutToShowToolsMenu();
    void aboutToShowHelpMenu();
    void aboutToShowViewMenu();
    void aboutToShowEncodingMenu();

    void searchOnPage();
    void showCookieManager();
    void showHistoryManager();
    void showHistorySideBar();
    void showBookmarksManager();
    void showBookmarksSideBar();
    void showRSSManager();
    void showDownloadManager();
    void showMenubar();
    void showNavigationToolbar();
    void showStatusbar();
    void showClearPrivateData();
    void showPreferences();

    void refreshHistory();
    void bookmarkAllTabs();
    void newWindow() { mApp->makeNewWindow(false); }

    void openLocation() { locationBar()->setFocus(); locationBar()->selectAll(); }
    void openFile();
    void savePage();
    void sendLink() { QDesktopServices::openUrl(QUrl("mailto:?body="+weView()->url().toString())); }
    void webSearch();

    void copy() { QApplication::clipboard()->setText(weView()->selectedText()); }
    void selectAll() { weView()->selectAll(); }

    void zoomIn() { weView()->zoomIn(); }
    void zoomOut() { weView()->zoomOut(); }
    void zoomReset() { weView()->zoomReset(); }
    void fullScreen(bool make);
    void startPrivate(bool state);
    void changeEncoding();

    bool quitApp();

private:
    void closeEvent(QCloseEvent* event);

    void setupUi();
    void setupMenu();

    void addSideBar();

    bool m_tryRestore;
    bool m_historyMenuChanged;
    bool m_bookmarksMenuChanged;
    QUrl m_startingUrl;
    QUrl m_newtab;
    QUrl m_homepage;

    QVBoxLayout* m_mainLayout;
    QSplitter* m_mainSplitter;
    QMenu* m_superMenu;
    QMenu* m_menuFile;
    QMenu* m_menuEdit;
    QMenu* m_menuTools;
    QMenu* m_menuHelp;
    QMenu* m_menuView;
    QMenu* m_menuBookmarks;
    QMenu* m_menuHistory;
    QMenu* m_menuClosedTabs;
    QMenu* m_menuEncoding;
    QAction* m_actionShowToolbar;
    QAction* m_actionShowBookmarksToolbar;
    QAction* m_actionShowStatusbar;
    QAction* m_actionShowMenubar;
    QAction* m_actionShowFullScreen;
    QAction* m_actionShowBookmarksSideBar;
    QAction* m_actionShowHistorySideBar;
    QAction* m_actionShowRssSideBar;
    QAction* m_actionPrivateBrowsing;
    QAction* m_actionStop;
    QAction* m_actionReload;
    QAction* m_actionRestoreTab;

    QLabel* m_privateBrowsing;
    AdBlockIcon* m_adblockIcon;
    QPointer<WebInspectorDockWidget> m_webInspectorDock;

    BookmarksToolbar* m_bookmarksToolbar;
    TabWidget* m_tabWidget;
    QPointer<SideBar> m_sideBar;
    StatusBarMessage* m_statusBarMessage;
    NavigationBar* m_navigationBar;

    ProgressBar* m_progressBar;
    QLabel* m_ipLabel;

    QString m_activeProfil;
    QString m_activeLanguage;
    QColor m_menuTextColor;

    int m_sideBarWidth;

    //Used for F11 FullScreen remember visibility
    //of menubar and statusbar
    bool m_menuBarVisible;
    bool m_statusBarVisible;
    bool m_navigationVisible;
    bool m_bookmarksToolBarVisible;
};

#endif // QUPZILLA_H
