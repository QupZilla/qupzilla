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

#include "webtab.h"
#include "tabmodel.h"
#include "tabwidget.h"
#include "toolbutton.h"
#include "tabtreemodel.h"
#include "browserwindow.h"

#include <QListView>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWheelEvent>

VerticalTabsWidget::VerticalTabsWidget(BrowserWindow *window)
    : QWidget()
    , m_window(window)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    m_pinnedView = new TabListView(m_window, this);
    TabFilterModel *model = new TabFilterModel(m_pinnedView);
    model->setFilterPinnedTabs(false);
    model->setRejectDropOnLastIndex(true);
    model->setSourceModel(m_window->tabModel());
    m_pinnedView->setModel(model);
    m_pinnedView->setHideWhenEmpty(true);

    m_normalView = new TabTreeView(m_window, this);
    m_pinnedView->setFocusProxy(m_normalView);

    ToolButton *buttonAddTab = new ToolButton(this);
    buttonAddTab->setObjectName(QSL("verticaltabs-button-addtab"));
    buttonAddTab->setAutoRaise(true);
    buttonAddTab->setFocusPolicy(Qt::NoFocus);
    buttonAddTab->setToolTip(tr("New Tab"));
    buttonAddTab->setIcon(QIcon::fromTheme(QSL("list-add")));
    buttonAddTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(buttonAddTab, SIGNAL(clicked()), m_window, SLOT(addTab()));

    m_groupMenu = new QMenu(this);
    buttonAddTab->setMenu(m_groupMenu);
    connect(m_groupMenu, &QMenu::aboutToShow, this, &VerticalTabsWidget::updateGroupMenu);

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
        m_normalView->setHaveTreeModel(false);
        break;

    case VerticalTabsPlugin::TabTreeView:
        m_treeModel = new TabTreeModel(m_window, model);
        m_treeModel->setSourceModel(m_window->tabModel());
        model->setSourceModel(m_treeModel);
        m_normalView->setModel(model);
        m_normalView->setTabsInOrder(false);
        m_normalView->setHaveTreeModel(true);
        break;

    default:
        break;
    };
}

void VerticalTabsWidget::switchToNextTab()
{
    WebTab *tab = nextTab();
    if (tab) {
        tab->makeCurrentTab();
    }
}

void VerticalTabsWidget::switchToPreviousTab()
{
    WebTab *tab = previousTab();
    if (tab) {
        tab->makeCurrentTab();
    }
}

WebTab *VerticalTabsWidget::nextTab() const
{
    QModelIndex next;
    if (m_window->tabWidget()->webTab()->isPinned()) {
        next = m_pinnedView->indexAfter(m_pinnedView->currentIndex());
        if (!next.isValid()) {
            next = m_normalView->model()->index(0, 0);
        }
    } else {
        next = m_normalView->indexBelow(m_normalView->currentIndex());
        if (!next.isValid()) {
            next = m_pinnedView->model()->index(0, 0);
        }
    }
    return next.data(TabModel::WebTabRole).value<WebTab*>();
}

WebTab *VerticalTabsWidget::previousTab() const
{
    QModelIndex previous;
    if (m_window->tabWidget()->webTab()->isPinned()) {
        previous = m_pinnedView->indexBefore(m_pinnedView->currentIndex());
        if (!previous.isValid()) {
            previous = m_normalView->model()->index(m_normalView->model()->rowCount() - 1, 0);
            while (previous.isValid()) {
                const QModelIndex below = m_normalView->indexBelow(previous);
                if (below.isValid()) {
                    previous = below;
                } else {
                    break;
                }
            }
        }
    } else {
        previous = m_normalView->indexAbove(m_normalView->currentIndex());
        if (!previous.isValid()) {
            previous = m_pinnedView->model()->index(m_pinnedView->model()->rowCount() - 1, 0);
        }
    }
    return previous.data(TabModel::WebTabRole).value<WebTab*>();
}

void VerticalTabsWidget::wheelEvent(QWheelEvent *event)
{
    if (m_normalView->verticalScrollBar()->isVisible()) {
        return;
    }

    m_wheelHelper.processEvent(event);
    while (WheelHelper::Direction direction = m_wheelHelper.takeDirection()) {
        switch (direction) {
        case WheelHelper::WheelUp:
        case WheelHelper::WheelLeft:
            switchToPreviousTab();
            break;

        case WheelHelper::WheelDown:
        case WheelHelper::WheelRight:
            switchToNextTab();
            break;

        default:
            break;
        }
    }
    event->accept();
}

void VerticalTabsWidget::updateGroupMenu()
{
    m_groupMenu->clear();

    for (int i = 0; i < m_window->tabWidget()->count(); ++i) {
        WebTab *tab = m_window->tabWidget()->webTab(i);
        if (tab->url().toString(QUrl::RemoveFragment) == QL1S("extension://verticaltabs/group")) {
            m_groupMenu->addAction(tab->url().fragment(), this, [=]() {
                QMetaObject::invokeMethod(m_window, "addTab");
                m_window->tabWidget()->webTab()->setParentTab(tab);
            });
        }
    }

    m_groupMenu->addSeparator();
    m_groupMenu->addAction(tr("Add New Group..."), this, [this]() {
        m_window->tabWidget()->addView(QUrl(QSL("extension://verticaltabs/group")), Qz::NT_SelectedTab);
    });
}
