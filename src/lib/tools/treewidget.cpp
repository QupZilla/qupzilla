/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "treewidget.h"

#include <QMouseEvent>

TreeWidget::TreeWidget(QWidget* parent)
    : QTreeWidget(parent)
    , m_refreshAllItemsNeeded(true)
    , m_showMode(ItemsCollapsed)
{
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(sheduleRefresh()));
}

void TreeWidget::clear()
{
    QTreeWidget::clear();
    m_allTreeItems.clear();
}

void TreeWidget::sheduleRefresh()
{
    m_refreshAllItemsNeeded = true;
}

void TreeWidget::addTopLevelItem(QTreeWidgetItem* item)
{
    m_allTreeItems.append(item);
    QTreeWidget::addTopLevelItem(item);
}

void TreeWidget::addTopLevelItems(const QList<QTreeWidgetItem*> &items)
{
    m_allTreeItems.append(items);
    QTreeWidget::addTopLevelItems(items);
}

void TreeWidget::insertTopLevelItem(int index, QTreeWidgetItem* item)
{
    m_allTreeItems.append(item);
    QTreeWidget::insertTopLevelItem(index, item);
}

void TreeWidget::insertTopLevelItems(int index, const QList<QTreeWidgetItem*> &items)
{
    m_allTreeItems.append(items);
    QTreeWidget::insertTopLevelItems(index, items);
}

void TreeWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier) {
        emit itemControlClicked(itemAt(event->pos()));
    }

    if (event->buttons() == Qt::MiddleButton) {
        emit itemMiddleButtonClicked(itemAt(event->pos()));
    }

    QTreeWidget::mousePressEvent(event);
}

void TreeWidget::iterateAllItems(QTreeWidgetItem* parent)
{
    int count = parent ? parent->childCount() : topLevelItemCount();

    for (int i = 0; i < count; i++) {
        QTreeWidgetItem* item = parent ? parent->child(i) : topLevelItem(i);

        if (item->childCount() == 0) {
            m_allTreeItems.append(item);
        }

        iterateAllItems(item);
    }
}

QList<QTreeWidgetItem*> TreeWidget::allItems()
{
    if (m_refreshAllItemsNeeded) {
        m_allTreeItems.clear();
        iterateAllItems(0);
        m_refreshAllItemsNeeded = false;
    }
    return m_allTreeItems;
}

void TreeWidget::filterString(QString string)
{
    expandAll();
    QList<QTreeWidgetItem*> _allItems = allItems();

    if (string.isEmpty()) {
        foreach(QTreeWidgetItem * item, _allItems)
        item->setHidden(false);
        for (int i = 0; i < topLevelItemCount(); i++) {
            topLevelItem(i)->setHidden(false);
        }
        if (m_showMode == ItemsCollapsed) {
            collapseAll();
        }
    }
    else {
        foreach(QTreeWidgetItem * item, _allItems) {
            item->setHidden(!item->text(0).contains(string, Qt::CaseInsensitive));
            item->setExpanded(true);
        }
        for (int i = 0; i < topLevelItemCount(); i++) {
            topLevelItem(i)->setHidden(false);
        }

        QTreeWidgetItem* firstItem = topLevelItem(0);
        QTreeWidgetItem* belowItem = itemBelow(firstItem);

        int topLvlIndex = 0;
        while (firstItem) {
            if (firstItem->text(0).contains(string, Qt::CaseInsensitive)) {
                firstItem->setHidden(false);
            }
            else if (!firstItem->parent() && !belowItem) {
                firstItem->setHidden(true);
            }
            else if (!belowItem) {
                break;
            }
            else if (!firstItem->parent() && !belowItem->parent()) {
                firstItem->setHidden(true);
            }

            topLvlIndex++;
            firstItem = topLevelItem(topLvlIndex);
            belowItem = itemBelow(firstItem);
        }
    }
}

bool TreeWidget::appendToParentItem(const QString &parentText, QTreeWidgetItem* item)
{
    QList<QTreeWidgetItem*> list = findItems(parentText, Qt::MatchExactly);
    if (list.count() == 0) {
        return false;
    }
    QTreeWidgetItem* parentItem = list.at(0);
    if (!parentItem) {
        return false;
    }

    m_allTreeItems.append(item);
    parentItem->addChild(item);
    return true;
}

bool TreeWidget::appendToParentItem(QTreeWidgetItem* parent, QTreeWidgetItem* item)
{
    if (!parent || parent->treeWidget() != this) {
        return false;
    }

    m_allTreeItems.append(item);
    parent->addChild(item);
    return true;
}

bool TreeWidget::prependToParentItem(const QString &parentText, QTreeWidgetItem* item)
{
    QList<QTreeWidgetItem*> list = findItems(parentText, Qt::MatchExactly);
    if (list.count() == 0) {
        return false;
    }
    QTreeWidgetItem* parentItem = list.at(0);
    if (!parentItem) {
        return false;
    }

    m_allTreeItems.append(item);
    parentItem->insertChild(0, item);
    return true;
}

bool TreeWidget::prependToParentItem(QTreeWidgetItem* parent, QTreeWidgetItem* item)
{
    if (!parent || parent->treeWidget() != this) {
        return false;
    }

    m_allTreeItems.append(item);
    parent->insertChild(0, item);
    return true;
}

void TreeWidget::deleteItem(QTreeWidgetItem* item)
{
//    if (m_allTreeItems.contains(item)) {
//        m_allTreeItems.removeOne(item);
//    }

    m_refreshAllItemsNeeded = true;

    delete item;
}

void TreeWidget::deleteItems(const QList<QTreeWidgetItem*> &items)
{
    m_refreshAllItemsNeeded = true;

    qDeleteAll(items);
}
