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
#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QNetworkRequest>
#include <QMenu>

#include "tabstackedwidget.h"
#include "toolbutton.h"
#include "loadrequest.h"
#include "webtab.h"
#include "qzcommon.h"

class QMenu;

class TabBar;
class TabIcon;
class TabWidget;
class BrowserWindow;
class TabbedWebView;
class ClosedTabsManager;

class QUPZILLA_EXPORT AddTabButton : public ToolButton
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

class QUPZILLA_EXPORT MenuTabs : public QMenu
{
    Q_OBJECT
public:
    explicit MenuTabs(QWidget* parent = 0) : QMenu(parent) {}

signals:
    void closeTab(int);

private:
    void mouseReleaseEvent(QMouseEvent* event);
};

class QUPZILLA_EXPORT TabWidget : public TabStackedWidget
{
    Q_OBJECT
public:
    explicit TabWidget(BrowserWindow* mainclass, QWidget* parent = 0);
    ~TabWidget();

    QByteArray saveState();
    bool restoreState(const QVector<WebTab::SavedTab> &tabs, int currentTab);
    void closeRecoveryTab();

    void savePinnedTabs();
    void restorePinnedTabs();

    void setCurrentIndex(int index);

    void nextTab();
    void previousTab();
    void currentTabChanged(int index);

    int normalTabsCount() const;
    int pinnedTabsCount() const;
    int lastTabIndex() const;
    int extraReservedWidth() const;

    TabBar* getTabBar() const;
    ClosedTabsManager* closedTabsManager() const;
    QList<WebTab*> allTabs(bool withPinned = true);
    bool canRestoreTab() const;

    QStackedWidget* locationBars() const;
    ToolButton* buttonClosedTabs() const;
    AddTabButton* buttonAddTab() const;

public slots:
    int addView(const LoadRequest &req, const Qz::NewTabPositionFlags &openFlags, bool selectLine = false, bool pinned = false);
    int addView(const LoadRequest &req, const QString &title = tr("New tab"), const Qz::NewTabPositionFlags &openFlags = Qz::NT_SelectedTab, bool selectLine = false, int position = -1, bool pinned = false);
    int addView(WebTab* tab);

    void addTabFromClipboard();
    int duplicateTab(int index);

    void closeTab(int index = -1, bool force = false);
    void reloadTab(int index);
    void reloadAllTabs();
    void stopTab(int index);
    void closeAllButCurrent(int index);
    void detachTab(int index);
    void restoreClosedTab(QObject* obj = 0);
    void restoreAllClosedTabs();
    void clearClosedTabsList();

    void moveAddTabButton(int posX);

    void tabBarOverFlowChanged(bool overflowed);

signals:
    void changed();

private slots:
    void loadSettings();

    void aboutToShowTabsMenu();
    void aboutToShowClosedTabsMenu();

    void actionChangeIndex();
    void tabMoved(int before, int after);

private:
    WebTab* weTab();
    WebTab* weTab(int index);
    TabIcon* tabIcon(int index);

    bool validIndex(int index) const;
    void updateClosedTabsButton();

    BrowserWindow* m_window;
    TabBar* m_tabBar;
    QStackedWidget* m_locationBars;
    ClosedTabsManager* m_closedTabsManager;

    MenuTabs* m_menuTabs;
    ToolButton* m_buttonListTabs;
    QMenu* m_menuClosedTabs;
    ToolButton* m_buttonClosedTabs;
    AddTabButton* m_buttonAddTab;
    AddTabButton* m_buttonAddTab2;

    int m_lastTabIndex;
    int m_lastBackgroundTabIndex;
    bool m_isClosingToLastTabIndex;
    bool m_isRestoringState;

    bool m_dontCloseWithOneTab;
    bool m_showClosedTabsButton;
    bool m_newTabAfterActive;
    bool m_newEmptyTabAfterActive;
    QUrl m_urlOnNewTab;
};

#endif // TABWIDGET_H
