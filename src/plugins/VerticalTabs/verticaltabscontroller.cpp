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
#include "verticaltabscontroller.h"
#include "verticaltabsplugin.h"
#include "verticaltabswidget.h"

#include <QAction>

VerticalTabsController::VerticalTabsController(VerticalTabsPlugin *plugin)
    : SideBarInterface(plugin)
    , m_plugin(plugin)
{
}

QString VerticalTabsController::title() const
{
    return tr("Vertical Tabs");
}

QAction *VerticalTabsController::createMenuAction()
{
    QAction *act = new QAction(title(), this);
    act->setCheckable(true);
    return act;
}

QWidget *VerticalTabsController::createSideBarWidget(BrowserWindow *window)
{
    VerticalTabsWidget *widget = new VerticalTabsWidget(window);
    widget->setViewType(m_plugin->viewType());
    widget->setStyleSheet(m_plugin->styleSheet());
    connect(m_plugin, &VerticalTabsPlugin::viewTypeChanged, widget, &VerticalTabsWidget::setViewType);
    connect(m_plugin, &VerticalTabsPlugin::styleSheetChanged, widget, &VerticalTabsWidget::setStyleSheet);

    return widget;
}
