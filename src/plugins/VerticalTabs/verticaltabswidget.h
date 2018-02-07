/* ============================================================
* VerticalTabs plugin for QupZilla
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#pragma once

#include <QWidget>

#include "wheelhelper.h"

#include "verticaltabsplugin.h"

class QMenu;

class WebTab;
class BrowserWindow;
class TabTreeModel;

class TabListView;
class TabTreeView;

class VerticalTabsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VerticalTabsWidget(BrowserWindow *window);

    void setViewType(VerticalTabsPlugin::ViewType type);

    void switchToNextTab();
    void switchToPreviousTab();

private:
    WebTab *nextTab() const;
    WebTab *previousTab() const;

    void wheelEvent(QWheelEvent *event) override;
    void updateGroupMenu();

    BrowserWindow *m_window;
    TabListView *m_pinnedView;
    TabTreeView *m_normalView;
    TabTreeModel *m_treeModel = nullptr;
    WheelHelper m_wheelHelper;
    QMenu *m_groupMenu;
};
