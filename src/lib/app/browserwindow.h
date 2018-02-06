/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include <QPointer>
#include <QUrl>

#include "webtab.h"
#include "qzcommon.h"

class QLabel;
class QVBoxLayout;
class QSplitter;
class QWebEngineFrame;
class QTimer;

class Menu;
class MainMenu;
class TabWidget;
class TabbedWebView;
class LineEdit;
class HistoryMenu;
class BookmarksMenu;
class BookmarksToolbar;
class AutoFill;
class MainApplication;
class WebView;
class WebPage;
class SideBar;
class SideBarManager;
class ProgressBar;
class StatusBar;
class NavigationBar;
class NavigationContainer;
class ClickableLabel;
class LocationBar;
class TabModel;
class TabMruModel;

class QUPZILLA_EXPORT BrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    struct SavedWindow {
        QByteArray windowState;
        QByteArray windowGeometry;
        QHash<QString, QVariant> windowUiState;
        int virtualDesktop = -1;
        int currentTab = -1;
        QVector<WebTab::SavedTab> tabs;

        SavedWindow();
        SavedWindow(BrowserWindow *window);

        bool isValid() const;
        void clear();

        friend QUPZILLA_EXPORT QDataStream &operator<<(QDataStream &stream, const SavedWindow &window);
        friend QUPZILLA_EXPORT QDataStream &operator>>(QDataStream &stream, SavedWindow &window);
    };

    explicit BrowserWindow(Qz::BrowserWindowType type, const QUrl &url = QUrl());
    ~BrowserWindow();

    void setStartTab(WebTab* tab);
    void setStartPage(WebPage* page);

    void restoreWindow(const SavedWindow &window);

    bool fullScreenNavigationVisible() const;
    void showNavigationWithFullScreen();
    void hideNavigationWithFullScreen();

    void currentTabChanged();
    void updateLoadingActions();

    void addBookmark(const QUrl &url, const QString &title);
    void addDeleteOnCloseWidget(QWidget* widget);

    void createToolbarsMenu(QMenu* menu);
    void createSidebarsMenu(QMenu* menu);
    void createEncodingMenu(QMenu* menu);

    void removeActions(const QList<QAction*> &actions);

    SideBar* addSideBar();
    void saveSideBarSettings();

    int tabCount() const;
    TabbedWebView* weView() const;
    TabbedWebView* weView(int index) const;

    Qz::BrowserWindowType windowType() const;
    LocationBar* locationBar() const;
    TabWidget* tabWidget() const;
    BookmarksToolbar* bookmarksToolbar() const;
    StatusBar* statusBar() const;
    NavigationBar* navigationBar() const;
    SideBarManager* sideBarManager() const;
    QLabel* ipLabel() const;
    QMenu* superMenu() const;

    QUrl homepageUrl() const;

    QAction* action(const QString &name) const;

    TabModel *tabModel() const;
    TabMruModel *tabMruModel() const;

signals:
    void startingCompleted();
    void aboutToClose();

public slots:
    void goHome();
    void goHomeInNewTab();
    void goBack();
    void goForward();

    void reload();
    void reloadBypassCache();

    void setWindowTitle(const QString &t);

    void showWebInspector();
    void toggleWebInspector();
    void showHistoryManager();

    void toggleShowMenubar();
    void toggleShowStatusBar();
    void toggleShowBookmarksToolbar();
    void toggleShowNavigationToolbar();
    void toggleTabsOnTop(bool enable);

    void toggleFullScreen();
    void toggleHtmlFullScreen(bool enable);

    void loadActionUrl(QObject* obj = 0);
    void loadActionUrlInNewTab(QObject* obj = 0);

    void bookmarkPage();
    void bookmarkAllTabs();
    void loadAddress(const QUrl &url);
    void showSource(WebView *view = Q_NULLPTR);

private slots:
    void addTab();
    void openLocation();
    void openFile();
    void closeWindow();
    void closeTab();
    void loadSettings();
    void postLaunch();

    void refreshHistory();
    void webSearch();
    void searchOnPage();
    void changeEncoding();
    void printPage();

    void saveSettings();
    void hideNavigationSlot();

private:
    bool event(QEvent* event);
    void resizeEvent(QResizeEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void closeEvent(QCloseEvent* event);

    void setupUi();
    void setupMenu();
    void updateStartupFocus();

    QAction *createEncodingAction(const QString &codecName, const QString &activeCodecName, QMenu *menu);
    void createEncodingSubMenu(const QString &name, QStringList &codecNames, QMenu *menu);

    QHash<QString, QVariant> saveUiState();
    void restoreUiState(const QHash<QString, QVariant> &state);

    QUrl m_startUrl;
    QUrl m_homepage;
    Qz::BrowserWindowType m_windowType;
    WebTab* m_startTab;
    WebPage* m_startPage;

    QVBoxLayout* m_mainLayout;
    QSplitter* m_mainSplitter;

    TabWidget* m_tabWidget;
    QPointer<SideBar> m_sideBar;
    SideBarManager* m_sideBarManager;
    StatusBar* m_statusBar;

    NavigationContainer* m_navigationContainer;
    NavigationBar* m_navigationToolbar;
    BookmarksToolbar* m_bookmarksToolbar;

    ProgressBar* m_progressBar;
    QLabel* m_ipLabel;

    QMenu* m_superMenu;
    MainMenu* m_mainMenu;

    TabModel *m_tabModel = nullptr;
    TabMruModel *m_tabMruModel = nullptr;

    int m_sideBarWidth;
    int m_webViewWidth;

    // Shortcuts
    bool m_useTabNumberShortcuts;
    bool m_useSpeedDialNumberShortcuts;
    bool m_useSingleKeyShortcuts;

    // Remember visibility of menubar and statusbar after entering Fullscreen
    bool m_menuBarVisible;
    bool m_statusBarVisible;
    bool m_isHtmlFullScreen;
    QTimer* m_hideNavigationTimer;

    QList<QPointer<QWidget> > m_deleteOnCloseWidgets;

#ifdef QZ_WS_X11
private:
    int getCurrentVirtualDesktop() const;
    void moveToVirtualDesktop(int desktopId);
#endif
};

#endif // QUPZILLA_H
