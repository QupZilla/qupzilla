/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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

TreeWidget::TreeWidget(QWidget* parent) :
    QTreeWidget(parent)
{
}

void TreeWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier)
        emit itemControlClicked(itemAt(event->pos()));

    QTreeWidget::mousePressEvent(event);
}

QList<QTreeWidgetItem*> allTreeItems;
void iterateAllItems(QTreeWidgetItem* parent, QTreeWidget* treeWidget, bool includeTopLevelItems = true)
{
  int count = parent ? parent->childCount() : treeWidget->topLevelItemCount();

  for (int i = 0; i < count; i++)
  {
    QTreeWidgetItem *item =
      parent ? parent->child(i) : treeWidget->topLevelItem(i);

    if (includeTopLevelItems)
        allTreeItems.append(item);
    else if (item->childCount() == 0)
        allTreeItems.append(item);

    iterateAllItems(item, treeWidget, includeTopLevelItems);
  }
}

QList<QTreeWidgetItem*> TreeWidget::allItems(bool includeTopLevelItems)
{
    allTreeItems.clear();
    iterateAllItems(0, this, includeTopLevelItems);
    return allTreeItems;
}

void TreeWidget::filterStringWithoutTopItems(QString string)
{
    QList<QTreeWidgetItem*> _allItems = allItems(false);

    if (string.isEmpty()) {
        foreach (QTreeWidgetItem* item, _allItems)
            item->setHidden(false);
    } else {
        foreach (QTreeWidgetItem* item, _allItems)
            item->setHidden(!item->text(0).contains(string));
    }
}

void TreeWidget::filterStringWithTopItems(QString string)
{
    QList<QTreeWidgetItem*> _allItems = allItems();

    if (string.isEmpty()) {
        foreach (QTreeWidgetItem* item, _allItems)
            item->setHidden(false);
    } else {
        foreach (QTreeWidgetItem* item, _allItems)
            item->setHidden(!item->text(0).contains(string));
    }
}
