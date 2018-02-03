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
#include "tablistview.h"
#include "tabfiltermodel.h"

#include "tabmodel.h"
#include "toolbutton.h"
#include "tabtreemodel.h"
#include "browserwindow.h"

#include <QVBoxLayout>
#include <QListView>

VerticalTabsWidget::VerticalTabsWidget(BrowserWindow *window)
    : QWidget()
    , m_window(window)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    TabListView *m_pinnedView = new TabListView(this);
    TabFilterModel *model = new TabFilterModel(m_pinnedView);
    model->setFilterPinnedTabs(false);
    model->setSourceModel(m_window->tabModel());
    m_pinnedView->setModel(model);

    m_normalView = new TabTreeView(this);
    m_pinnedView->setFocusProxy(m_normalView);

    ToolButton *buttonAddTab = new ToolButton(this);
    buttonAddTab->setObjectName(QSL("verticaltabs-button-addtab"));
    buttonAddTab->setAutoRaise(true);
    buttonAddTab->setFocusPolicy(Qt::NoFocus);
    buttonAddTab->setToolTip(tr("New Tab"));
    buttonAddTab->setIcon(QIcon::fromTheme(QSL("list-add")));
    buttonAddTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(buttonAddTab, SIGNAL(clicked()), m_window, SLOT(addTab()));

    layout->addWidget(m_pinnedView);
    layout->addWidget(m_normalView);
    layout->addWidget(buttonAddTab);
}

void VerticalTabsWidget::setViewType(VerticalTabsPlugin::ViewType type)
{
    TabFilterModel *model = new TabFilterModel(m_normalView);
    model->setFilterPinnedTabs(true);

    delete m_normalView->model();

    switch (type) {
    case VerticalTabsPlugin::TabListView:
        model->setSourceModel(m_window->tabModel());
        m_normalView->setModel(model);
        m_normalView->setTabsInOrder(true);
        break;

    case VerticalTabsPlugin::TabTreeView:
        m_treeModel = new TabTreeModel(m_window, model);
        m_treeModel->setSourceModel(m_window->tabModel());
        model->setSourceModel(m_treeModel);
        m_normalView->setModel(model);
        m_normalView->setTabsInOrder(false);
        break;

    default:
        break;
    };
}
