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
#include "verticaltabswidget.h"
#include "tabtreeview.h"

#include "tabmodel.h"
#include "tabtreemodel.h"
#include "browserwindow.h"

#include <QVBoxLayout>

VerticalTabsWidget::VerticalTabsWidget(BrowserWindow *window)
    : QWidget()
    , m_window(window)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    m_view = new TabTreeView(this);
    layout->addWidget(m_view);
}

void VerticalTabsWidget::setViewType(VerticalTabsPlugin::ViewType type)
{
    switch (type) {
    case VerticalTabsPlugin::TabListView:
        m_view->setModel(m_window->tabModel());
        m_view->setTabsInOrder(true);
        break;

    case VerticalTabsPlugin::TabTreeView:
        delete m_treeModel;
        m_treeModel = new TabTreeModel(this);
        m_treeModel->setSourceModel(m_window->tabModel());
        m_view->setModel(m_treeModel);
        m_view->setTabsInOrder(false);
        break;

    default:
        break;
    };
}
