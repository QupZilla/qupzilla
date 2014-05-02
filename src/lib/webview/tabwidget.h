/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include <QMenu>

#include "tabstackedwidget.h"
#include "toolbutton.h"
#include "qz_namespace.h"
#include "webtab.h"

class QStackedWidget;
class QMenu;

class QupZilla;
class TabbedWebView;
class TabBar;
class TabWidget;
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

class QT_QUPZILLA_EXPORT MenuTabs : public QMenu
{
    Q_OBJECT
public:
    explicit MenuTabs(QWidget* parent = 0) : QMenu(parent) {}

signals:
    void closeTab(int);

private:
    void mouseReleaseEvent(QMouseEvent* event);
};

class QT_QUPZILLA_EXPORT TabWidget : public TabStackedWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QupZilla* mainclass, QWidget* parent = 0);
    ~TabWidget();

    void loadSettings();

    QByteArray saveState();
    bool restoreState(const QVector<WebTab::SavedTab> &tabs, int currentTab);
    void closeRecoveryTab();

    void savePinnedTabs();
    void restorePinnedTabs();

    void startTabAnimation(int index);
    void stopTabAnimation(int index);

    void setCurrentIndex(int index);
    void setTabIcon(int index, const QIcon &icon);
    void setTabText(int index, const QString &text);

    void nextTab();
    void previousTab();
    void currentTabChanged(int index);

    int normalTabsCount() const;
    int pinnedTabsCount() const;
    int lastTabIndex() const;

    TabBar* getTabBar() const;
    ClosedTabsManager* closedTabsManager() const;
    QList<WebTab*> allTabs(bool withPinned = true);
    bool canRestoreTab() const;

    QStackedWidget* locationBars() const;
    ToolButton* buttonListTabs() const;
    AddTabButton* buttonAddTab() const;

    void disconnectObjects();

public slots:
    int addView(const QUrl &url, const Qz::NewTabPositionFlags &openFlags, bool selectLine = false, bool pinned = false);
    int addView(const QNetworkRequest &req, const Qz::NewTabPositionFlags &openFlags, bool selectLine = false, bool pinned = false);

    int addView(const QUrl &url, const QString &title = tr("New tab"), const Qz::NewTabPositionFlags &openFlags = Qz::NT_SelectedTab, bool selectLine = false, int position = -1, bool pinned = false);
    int addView(QNetworkRequest req, const QString &title = tr("New tab"), const Qz::NewTabPositionFlags &openFlags = Qz::NT_SelectedTab, bool selectLine = false, int position = -1, bool pinned = false);
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
    void aboutToShowClosedTabsMenu();

    void moveAddTabButton(int posX);
    void showButtons();
    void hideButtons();

    void tabBarOverFlowChanged(bool overFlowed);

private slots:
    void aboutToShowTabsMenu();
    void actionChangeIndex();
    void tabMoved(int before, int after);

private:
    WebTab* weTab();
    WebTab* weTab(int index);

    inline bool validIndex(int index) const { return index >= 0 && index < count(); }

    bool m_dontQuitWithOneTab;
    bool m_closedInsteadOpened;
    bool m_newTabAfterActive;
    bool m_newEmptyTabAfterActive;
    QUrl m_urlOnNewTab;

    QupZilla* p_QupZilla;

    int m_lastTabIndex;
    int m_lastBackgroundTabIndex;
    bool m_isRestoringState;

    TabBar* m_tabBar;
    MenuTabs* m_menuTabs;
    ToolButton* m_buttonListTabs;
    AddTabButton* m_buttonAddTab;
    ToolButton* m_buttonListTabs2;
    AddTabButton* m_buttonAddTab2;
    ClosedTabsManager* m_closedTabsManager;

    QStackedWidget* m_locationBars;
};

#endif // TABWIDGET_H
