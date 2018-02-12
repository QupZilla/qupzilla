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
#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QMenu>
#include <QPointer>

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
    explicit TabWidget(BrowserWindow *window, QWidget *parent = nullptr);
    ~TabWidget();

    BrowserWindow *browserWindow() const;

    bool restoreState(const QVector<WebTab::SavedTab> &tabs, int currentTab);

    void setCurrentIndex(int index);

    void nextTab();
    void previousTab();
    void currentTabChanged(int index);

    int normalTabsCount() const;
    int pinnedTabsCount() const;
    int extraReservedWidth() const;

    WebTab *webTab(int index = -1) const;

    TabBar* tabBar() const;
    ClosedTabsManager* closedTabsManager() const;
    QList<WebTab*> allTabs(bool withPinned = true);
    bool canRestoreTab() const;
    bool isCurrentTabFresh() const;
    void setCurrentTabFresh(bool currentTabFresh);

    QStackedWidget* locationBars() const;
    ToolButton* buttonClosedTabs() const;
    AddTabButton* buttonAddTab() const;

    void moveTab(int from, int to);
    int pinUnPinTab(int index, const QString &title = QString());

    void detachTab(WebTab* tab);

public slots:
    int addView(const LoadRequest &req, const Qz::NewTabPositionFlags &openFlags, bool selectLine = false, bool pinned = false);
    int addView(const LoadRequest &req, const QString &title = tr("New tab"), const Qz::NewTabPositionFlags &openFlags = Qz::NT_SelectedTab, bool selectLine = false, int position = -1, bool pinned = false);
    int addView(WebTab *tab, const Qz::NewTabPositionFlags &openFlags);
    int insertView(int index, WebTab *tab, const Qz::NewTabPositionFlags &openFlags);

    void addTabFromClipboard();
    int duplicateTab(int index);

    // Force close tab
    void closeTab(int index = -1);
    // Request close tab (may be rejected)
    void requestCloseTab(int index = -1);

    void reloadTab(int index);
    void reloadAllTabs();
    void stopTab(int index);
    void closeAllButCurrent(int index);
    void closeToRight(int index);
    void closeToLeft(int index);
    void detachTab(int index);
    void loadTab(int index);
    void unloadTab(int index);
    void restoreClosedTab(QObject* obj = 0);
    void restoreAllClosedTabs();
    void clearClosedTabsList();

    void moveAddTabButton(int posX);

    void tabBarOverFlowChanged(bool overflowed);

signals:
    void changed();
    void tabInserted(int index);
    void tabRemoved(int index);
    void tabMoved(int from, int to);

private slots:
    void loadSettings();

    void aboutToShowTabsMenu();
    void aboutToShowClosedTabsMenu();

    void actionChangeIndex();
    void tabWasMoved(int before, int after);

private:
    WebTab* weTab() const;
    WebTab* weTab(int index) const;
    TabIcon* tabIcon(int index) const;

    bool validIndex(int index) const;
    void updateClosedTabsButton();

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

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

    QPointer<WebTab> m_lastBackgroundTab;

    bool m_dontCloseWithOneTab;
    bool m_showClosedTabsButton;
    bool m_newTabAfterActive;
    bool m_newEmptyTabAfterActive;
    QUrl m_urlOnNewTab;

    bool m_currentTabFresh;
};

#endif // TABWIDGET_H
