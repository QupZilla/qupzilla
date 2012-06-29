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
#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QUrl>
#include <QNetworkRequest>

#include "toolbutton.h"
#include "qz_namespace.h"

class QStackedWidget;
class QMenu;

class QupZilla;
class TabbedWebView;
class TabBar;
class TabWidget;
class WebTab;
class ClosedTabsManager;

class QT_QUPZILLA_EXPORT AddTabButton : public ToolButton
{
public:
    explicit AddTabButton(TabWidget* tabWidget, TabBar* tabBar);

private:
    void wheelEvent(QWheelEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

    TabBar* m_tabBar;
    TabWidget* m_tabWidget;
};

class QT_QUPZILLA_EXPORT TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QupZilla* mainclass, QWidget* parent = 0);
    ~TabWidget();

    void loadSettings();

    QByteArray saveState();
    bool restoreState(const QByteArray &state);

    void savePinnedTabs();
    void restorePinnedTabs();

    void startTabAnimation(int index);
    void stopTabAnimation(int index);

    void setTabIcon(int index, const QIcon &icon);
    void setTabText(int index, const QString &text);

    void nextTab();
    void previousTab();

    void showTabBar();

    TabBar* getTabBar() { return m_tabBar; }
    ClosedTabsManager* closedTabsManager() { return m_closedTabsManager; }
    bool canRestoreTab();
    QList<WebTab*> allTabs(bool withPinned = true);
    QStackedWidget* locationBars() { return m_locationBars; }
    ToolButton* buttonListTabs() { return m_buttonListTabs; }
    AddTabButton* buttonAddTab() { return m_buttonAddTab; }

    void disconnectObjects();

signals:
    void pinnedTabClosed();
    void pinnedTabAdded();

public slots:
    int addView(const QUrl &url, const Qz::NewTabPositionFlags &openFlags, bool selectLine = false);
    int addView(const QNetworkRequest &req, const Qz::NewTabPositionFlags &openFlags, bool selectLine = false);

    int addView(const QUrl &url, const QString &title = tr("New tab"), const Qz::NewTabPositionFlags &openFlags = Qz::NT_SelectedTab, bool selectLine = false, int position = -1);
    int addView(QNetworkRequest req, const QString &title = tr("New tab"), const Qz::NewTabPositionFlags &openFlags = Qz::NT_SelectedTab, bool selectLine = false, int position = -1);

    int duplicateTab(int index);

    void closeTab(int index = -1);
    void reloadTab(int index);
    void reloadAllTabs();
    void stopTab(int index);
    void closeAllButCurrent(int index);
    void restoreClosedTab();
    void restoreAllClosedTabs();
    void clearClosedTabsList();
    void aboutToShowClosedTabsMenu();

    void moveAddTabButton(int posX);
    void showButtons();
    void hideButtons();

private slots:
    void aboutToShowTabsMenu();
    void actionChangeIndex();
    void currentTabChanged(int index);
    void tabMoved(int before, int after);

private:
    void tabInserted(int index);
    void tabRemoved(int index);

    void resizeEvent(QResizeEvent* e);

    WebTab* weTab();
    WebTab* weTab(int index);

    bool m_hideTabBarWithOneTab;
    bool m_dontQuitWithOneTab;
    bool m_closedInsteadOpened;
    bool m_newTabAfterActive;
    QUrl m_urlOnNewTab;
    QupZilla* p_QupZilla;

    int m_lastTabIndex;
    int m_lastBackgroundTabIndex;
    bool m_isClosingToLastTabIndex;
    bool m_isRestoringState;

    TabBar* m_tabBar;
    QMenu* m_menuTabs;
    ToolButton* m_buttonListTabs;
    AddTabButton* m_buttonAddTab;
    ClosedTabsManager* m_closedTabsManager;

    QStackedWidget* m_locationBars;
};

#endif // TABWIDGET_H
