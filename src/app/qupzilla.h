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
#ifndef QUPZILLA_H
#define QUPZILLA_H

//Comment for release building
//#define DEVELOPING
//Check if i don't fuck anything

#ifdef QT_NO_DEBUG
#ifdef DEVELOPING
#error "TRYING TO RELEASE WITH DEVELOPING FLAG"
#endif
#endif

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
#include "locationbar.h"
#include "mainapplication.h"
#include "websearchbar.h"

class TabWidget;
class WebView;
class LineEdit;
class LocationBar;
class SearchToolBar;
class WebSearchBar;
class BookmarksToolbar;
class AutoFillModel;
class MainApplication;
class WebTab;
class AdBlockIcon;
class QupZilla : public QMainWindow
{
    Q_OBJECT

public:
    static const QString VERSION;
    static const QString BUILDTIME;
    static const QString AUTHOR;
    static const QString COPYRIGHT;
    static const QString WWWADDRESS;
    static const QString WEBKITVERSION;

    explicit QupZilla(bool m_tryRestore=true, QUrl startUrl=QUrl());
    ~QupZilla();

    void refreshAddressBar();
    void addBookmark(const QUrl &url, const QString &title);
    void installTranslator();
    void loadSettings();
    void showInspector();
    void setBackground(QColor textColor);

    inline WebView* weView() const { WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_tabWidget->currentIndex())); if (!webTab) return 0; return webTab->view(); }
    inline WebView* weView(int index) const { WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index)); if (!webTab) return 0; return webTab->view(); }
    inline LocationBar* locationBar(){ return m_locationBar; }
    inline TabWidget* tabWidget(){ return m_tabWidget; }
    inline BookmarksToolbar* bookmarksToolbar(){ return m_bookmarksToolbar; }

    inline QAction* buttonStop(){ return m_buttonStop; }
    inline QAction* buttonReload(){ return m_buttonReload; }
    inline QProgressBar* progressBar(){ return m_progressBar; }
    inline QToolBar* navigationToolbar(){ return m_navigation; }
    inline QString activeProfil(){ return m_activeProfil; }
    inline QString activeLanguage(){ return m_activeLanguage; }
    inline QDockWidget* inspectorDock(){ return m_webInspectorDock; }
    inline QLabel* ipLabel(){ return m_ipLabel; }
    inline QColor menuTextColor() { return m_menuTextColor; }
    inline QAction* acShowBookmarksToolbar() { return m_actionShowBookmarksToolbar; }

signals:
    void loadHistory();
    void startingCompleted();
    void message(MainApplication::MessageType mes, bool state);

public slots:
    void refreshHistory(int index=-1);
    void loadActionUrl();
    void bookmarkPage();
    void loadAddress(QUrl url) { weView()->load(url); m_locationBar->setText(url.toEncoded()); }
    void showSource();
    void showPageInfo();
    void receiveMessage(MainApplication::MessageType mes, bool state);

private slots:
    void closeEvent(QCloseEvent* event);
    void postLaunch();
    void goAtHistoryIndex();
    void goNext() { weView()->forward(); }
    void goBack() { weView()->back(); }
    void goHome() { loadAddress(m_homepage); }
    void stop() { weView()->stop(); }
    void reload() { weView()->reload(); }
    void urlEnter();
    void aboutQupZilla();
    void addTab() { m_tabWidget->addView(QUrl(), tr("New tab"), TabWidget::NewTab, true); }
    void printPage();

    void aboutToShowHistoryBackMenu();
    void aboutToShowHistoryNextMenu();
    void aboutToShowHistoryMenu();
    void aboutToShowBookmarksMenu();
    void aboutToShowToolsMenu();
    void aboutToShowHelpMenu();
    void aboutToShowViewMenu();
    void aboutToShowEncodingMenu();

    void searchOnPage();
    void showCookieManager();
    void showHistoryManager();
    void showBookmarksManager();
    void showRSSManager();
    void showDownloadManager();

    void showMenubar();
    void showNavigationToolbar();
    void showBookmarksToolbar();
    void showStatusbar();
    void showClearPrivateData();
    void showPreferences();

    void bookmarkAllTabs();
    void newWindow() { mApp->makeNewWindow(false); }

    void openLocation() { m_locationBar->setFocus(); m_locationBar->selectAll(); }
    void openFile();
    void savePage();
    void sendLink() { QDesktopServices::openUrl(QUrl("mailto:?body="+weView()->url().toString())); }
    void webSearch() { m_searchLine->setFocus(); }

    void copy() { QApplication::clipboard()->setText(weView()->selectedText()); }
    void selectAll() { weView()->selectAll(); }
    void reportBug() { m_tabWidget->addView(QUrl("http://qupzilla.ic.cz/bugzilla/?do=newtask&project=2")); }

    void zoomIn() { weView()->zoomIn(); }
    void zoomOut() { weView()->zoomOut(); }
    void zoomReset() { weView()->zoomReset(); }
    void fullScreen(bool make);
    void startPrivate(bool state);
    void changeEncoding();

    bool quitApp();

private:
    void setupUi();
    void setupMenu();

    bool m_tryRestore;
    QUrl m_startingUrl;
    QUrl m_newtab;
    QUrl m_homepage;

    QToolButton* m_supMenu;
    QMenu* m_superMenu;
    QMenu* m_menuFile;
    QMenu* m_menuEdit;
    QMenu* m_menuTools;
    QMenu* m_menuHelp;
    QMenu* m_menuView;
    QMenu* m_menuBookmarks;
    QMenu* m_menuHistory;
    QMenu* m_menuBack;
    QMenu* m_menuForward;
    QMenu* m_menuEncoding;
    QAction* m_actionShowToolbar;
    QAction* m_actionShowBookmarksToolbar;
    QAction* m_actionShowStatusbar;
    QAction* m_actionShowMenubar;
    QAction* m_actionShowFullScreen;
    QAction* m_actionPrivateBrowsing;
    QAction* m_actionStop;
    QAction* m_actionReload;

    QLabel* m_privateBrowsing;
    ClickableLabel* m_adblockIcon;
    QPointer<QWebInspector> m_webInspector;
    QPointer<QDockWidget> m_webInspectorDock;

    WebSearchBar* m_searchLine;
    SearchToolBar* m_webSearchToolbar;
    BookmarksToolbar* m_bookmarksToolbar;
    LocationBar* m_locationBar;
    TabWidget* m_tabWidget;

    QSplitter* m_navigationSplitter;
    QAction* m_buttonBack;
    QAction* m_buttonNext;
    QAction* m_buttonHome;
    QAction* m_buttonStop;
    QAction* m_buttonReload;
    QAction* m_actionExitFullscreen;
    QProgressBar* m_progressBar;
    QLabel* m_ipLabel;
    QToolBar* m_navigation;

    QString m_activeProfil;
    QString m_activeLanguage;
    QColor m_menuTextColor;

    //Used for F11 FullScreen remember visibility
    //of menubar and statusbar
    bool m_menuBarVisible;
    bool m_statusBarVisible;
};

#endif // QUPZILLA_H
