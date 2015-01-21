/* ============================================================
* TabManager plugin for QupZilla
* Copyright (C) 2013  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#ifndef TABMANAGERWIDGETCONTROLLER_H
#define TABMANAGERWIDGETCONTROLLER_H

#include "sidebarinterface.h"
#include "tabmanagerwidget.h"

class WebPage;

class TabManagerWidgetController : public SideBarInterface
{
    Q_OBJECT
public:
    enum ViewType {
        ShowAsSideBar = 0,
        ShowAsWindow = 1
    };

    explicit TabManagerWidgetController(QObject* parent = 0);
    ~TabManagerWidgetController();

    QString title() const;
    QAction* createMenuAction();
    QWidget* createSideBarWidget(BrowserWindow* mainWindow);

    QWidget* createStatusBarIcon(BrowserWindow* mainWindow);

    ViewType viewType();
    void setViewType(ViewType type);

    TabManagerWidget::GroupType groupType();
    TabManagerWidget* createTabManagerWidget(BrowserWindow* mainClass, QWidget* parent = 0, bool defaultWidget = false);

    TabManagerWidget* defaultTabManager();

    void addStatusBarIcon(BrowserWindow* window);
    void removeStatusBarIcon(BrowserWindow* window);

public slots:
    void setGroupType(TabManagerWidget::GroupType type);
    void mainWindowCreated(BrowserWindow* window, bool refresh = true);
    void mainWindowDeleted(BrowserWindow* window);
    void raiseTabManager();
    void showSideBySide();

private:
    TabManagerWidget* m_defaultTabManager;
    ViewType m_viewType;
    TabManagerWidget::GroupType m_groupType;

    QHash<BrowserWindow*, QWidget*> m_statusBarIcons;
    QHash<BrowserWindow*, QAction*> m_actions;

signals:
    void requestRefreshTree(WebPage* p = 0);
    void pinStateChanged(int index, bool pinned);
};

#endif // TABMANAGERWIDGETCONTROLLER_H
