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

#include <QHash>
#include <QPointer>

#include "sidebarinterface.h"

class QKeyEvent;

class TabWidget;

class VerticalTabsPlugin;
class VerticalTabsWidget;

class VerticalTabsController : public SideBarInterface
{
    Q_OBJECT
public:
    explicit VerticalTabsController(VerticalTabsPlugin *plugin);

    QString title() const override;
    QAction *createMenuAction() override;
    QWidget *createSideBarWidget(BrowserWindow *window) override;

    bool handleKeyPress(QKeyEvent *event, TabWidget *tabWidget);

private:
    VerticalTabsPlugin *m_plugin;
    QHash<BrowserWindow*, QPointer<VerticalTabsWidget>> m_widgets;
};
