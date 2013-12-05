/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "bookmarksmodel.h"

#include <QMimeData>
#include <QMouseEvent>
#include <QSqlDatabase>
#include <QApplication>
#include <QUrl>

const int ITEM_IS_TOPLEVEL = Qt::UserRole + 20;
const int ITEM_PARENT_TITLE = Qt::UserRole + 21;

TreeWidget::TreeWidget(QWidget* parent)
    : QTreeWidget(parent)
    , m_refreshAllItemsNeeded(true)
    , m_showMode(ItemsCollapsed)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(sheduleRefresh()));
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

void TreeWidget::setMimeType(const QString &mimeType)
{
    m_mimeType = mimeType;
}

Qt::DropActions TreeWidget::supportedDropActions()
{
    return Qt::CopyAction;
}

QStringList TreeWidget::mimeTypes() const
{
    QStringList types;
    types << m_mimeType;
    return types;
}

QMimeData* TreeWidget::mimeData(const QList<QTreeWidgetItem*> items) const
{
    QMimeData* data = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QTreeWidgetItem* item, items) {
        if (!item) {
            continue;
        }

        // Why not just pass pointers ??!!
        QTreeWidgetItem* clonedItem = item->clone();

        // #1097 Clearing icon will properly write this item into stream ...
        clonedItem->setIcon(0, QIcon());

        bool parentIsRoot = !item->parent() || item->parent() == invisibleRootItem();

        clonedItem->setData(0, ITEM_IS_TOPLEVEL, parentIsRoot);
        clonedItem->setData(0, ITEM_PARENT_TITLE, (parentIsRoot ? QString() : item->parent()->text(0))) ;
        clonedItem->write(stream);
        delete clonedItem;
    }

    data->setData(m_mimeType, encodedData);
    return data;
}

bool TreeWidget::dropMimeData(QTreeWidgetItem* parent, int,
                              const QMimeData* data, Qt::DropAction action)
{
    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (parent && !parent->text(1).isEmpty()) { // parent is a bookmark, go one level up!
        parent = parent->parent();
    }

    if (!parent) {
        parent = invisibleRootItem();
    }

    bool ok = false;
    if (data->hasUrls()) {
        QString folder = (parent == invisibleRootItem()) ? QLatin1String("unsorted") : parent->text(0);
        QUrl url = data->urls().at(0);
        QString title = data->text().isEmpty() ? url.host() + url.path() : data->text();
        emit linkWasDroped(url, title, data->imageData(), folder, &ok);

        return ok;
    }

    if (!data->hasFormat(m_mimeType)) {
        return false;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QByteArray ba = data->data(m_mimeType);
    QDataStream stream(&ba, QIODevice::ReadOnly);
    if (stream.atEnd()) {
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    setUpdatesEnabled(false);

    while (!stream.atEnd()) {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->read(stream);
        bool parentIsRoot = item->data(0, ITEM_IS_TOPLEVEL).toBool();
        QString oldParentTitle = item->data(0, ITEM_PARENT_TITLE).toString();

        bool isFolder = (item && item->text(1).isEmpty());
        if (isFolder && (item->text(0) == _bookmarksMenu ||
                         item->text(0) == _bookmarksToolbar)) {
            continue;
        }

        bool parentIsOldParent = parentIsRoot ? (parent == invisibleRootItem()) : (oldParentTitle == parent->text(0));
        if (parentIsOldParent || (isFolder && parent != invisibleRootItem() &&
                                  parent->text(0) != _bookmarksToolbar)) {
            // just 'Bookmarks In ToolBar' folder can have subfolders
            continue;
        }

        if (isFolder) {
            emit folderParentChanged(item->text(0), parent->text(0) == _bookmarksToolbar, &ok);
        }
        else {
            emit bookmarkParentChanged(item->data(0, Qt::UserRole + 10).toInt(),
                                       parent->text(0), oldParentTitle, &ok);
        }

        if (!ok) {
            continue;
        }
    }

    db.commit();
    clearSelection();
    setUpdatesEnabled(true);
    QApplication::restoreOverrideCursor();
    return true;
}

void TreeWidget::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    QTreeWidget::dragEnterEvent(event);
    if (mimeData->hasUrls() || mimeData->hasFormat(m_mimeType)) {
        event->acceptProposedAction();
    }
    else {
        event->ignore();
    }
}

void TreeWidget::dragMoveEvent(QDragMoveEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    bool accept = false;
    if (mimeData->hasUrls()) {
        accept = true;
    }
    else if (mimeData->hasFormat(m_mimeType)) {
        QTreeWidgetItem* itemUnderMouse = itemAt(event->pos());
        if (!itemUnderMouse) {
            return;
        }

        bool underMouseIsFolder = (itemUnderMouse && itemUnderMouse->text(1).isEmpty());
        int top = visualItemRect(itemUnderMouse).top();
        int bottom = visualItemRect(itemUnderMouse).bottom();
        int y = event->pos().y();
        bool overEdgeOfItem = (y >= top - 1 && y <= top + 1) || (y <= bottom + 1 && y >= bottom - 1);

        QByteArray ba = mimeData->data(m_mimeType);
        QDataStream stream(&ba, QIODevice::ReadOnly);

        while (!stream.atEnd()) {
            QTreeWidgetItem* dragItem = new QTreeWidgetItem;
            dragItem->read(stream);
            bool parentIsRoot = dragItem->data(0, ITEM_IS_TOPLEVEL).toBool();
            QString oldParentTitle = dragItem->data(0, ITEM_PARENT_TITLE).toString();

            bool itemIsFolder = dragItem->text(1).isEmpty();
            if (dragItem->text(0) != _bookmarksMenu
                    && dragItem->text(0) != _bookmarksToolbar) {
                if (!itemUnderMouse->parent() && !parentIsRoot && overEdgeOfItem) {
                    accept = true;
                    break;
                }
                bool parentsAreDifferent = parentIsRoot
                                           ? itemUnderMouse->parent() != 0
                                           : (!itemUnderMouse->parent() || itemUnderMouse->parent()->text(0) != oldParentTitle);
                bool canHasSubFolder = !itemUnderMouse->parent()
                                       || itemUnderMouse->parent() == invisibleRootItem()
                                       || itemUnderMouse->parent()->text(0) == _bookmarksToolbar;

                if (!underMouseIsFolder && parentsAreDifferent) {
                    if (!itemIsFolder) {
                        accept = true;
                        break;
                    }
                    else if (!itemUnderMouse->parent()
                             || (dragItem->text(0) != itemUnderMouse->parent()->text(0)
                                 && canHasSubFolder)) {
                        accept = true;
                        break;
                    }
                }
                else if (underMouseIsFolder) {
                    if (itemIsFolder && itemUnderMouse->text(0) == _bookmarksToolbar
                            && (parentIsRoot || oldParentTitle != _bookmarksToolbar)) {
                        accept = true;
                        break;
                    }
                    else if (!itemIsFolder && oldParentTitle != itemUnderMouse->text(0)) {
                        accept = true;
                        break;
                    }
                }
            }
        }
    }

    QTreeWidget::dragMoveEvent(event);
    if (accept) {
        event->acceptProposedAction();
    }
    else {
        event->ignore();
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

void TreeWidget::filterString(const QString &string)
{
    QList<QTreeWidgetItem*> _allItems = allItems();
    QList<QTreeWidgetItem*> parents;
    bool stringIsEmpty = string.isEmpty();
    foreach (QTreeWidgetItem* item, _allItems) {
        bool containsString = stringIsEmpty || item->text(0).contains(string, Qt::CaseInsensitive);
        if (containsString) {
            item->setHidden(false);
            if (item->parent()) {
                if (!parents.contains(item->parent())) {
                    parents << item->parent();
                }
            }
        }
        else {
            item->setHidden(true);
            if (item->parent()) {
                item->parent()->setHidden(true);
            }
        }
    }

    for (int i = 0; i < parents.size(); ++i) {
        QTreeWidgetItem* parentItem = parents.at(i);
        parentItem->setHidden(false);
        if (stringIsEmpty) {
            parentItem->setExpanded(m_showMode == ItemsExpanded);
        }
        else {
            parentItem->setExpanded(true);
        }

        if (parentItem->parent() && !parents.contains(parentItem->parent())) {
            parents << parentItem->parent();
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

void TreeWidget::setDragDropReceiver(bool enable, QObject* receiver)
{
    if (!receiver) {
        enable = false;
    }
    setDragEnabled(enable);
    viewport()->setAcceptDrops(enable);
    setDropIndicatorShown(enable);
    if (enable) {
// TODO: It won't probably work in Qt5
#if QT_VERSION < 0x050000
        model()->setSupportedDragActions(Qt::CopyAction);
#endif
        connect(this, SIGNAL(folderParentChanged(QString,bool,bool*)), receiver, SLOT(changeFolderParent(QString,bool,bool*)));
        connect(this, SIGNAL(bookmarkParentChanged(int,QString,QString,bool*)), receiver, SLOT(changeBookmarkParent(int,QString,QString,bool*)));
        connect(this, SIGNAL(linkWasDroped(QUrl,QString,QVariant,QString,bool*)), receiver, SLOT(bookmarkDropedLink(QUrl,QString,QVariant,QString,bool*)));
    }
    else {
        disconnect(this, SIGNAL(folderParentChanged(QString,bool,bool*)), receiver, SLOT(changeFolderParent(QString,bool,bool*)));
        disconnect(this, SIGNAL(bookmarkParentChanged(int,QString,QString,bool*)), receiver, SLOT(changeBookmarkParent(int,QString,QString,bool*)));
        disconnect(this, SIGNAL(linkWasDroped(QUrl,QString,QVariant,QString,bool*)), receiver, SLOT(bookmarkDropedLink(QUrl,QString,QVariant,QString,bool*)));
    }
}
