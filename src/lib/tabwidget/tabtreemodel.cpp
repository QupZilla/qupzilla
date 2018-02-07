/* ============================================================
* QupZilla - Qt web browser
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
#include "tabtreemodel.h"
#include "tabmodel.h"
#include "webtab.h"
#include "tabwidget.h"
#include "browserwindow.h"

#include <QTimer>
#include <QMimeData>

class TabTreeModelItem
{
public:
    explicit TabTreeModelItem(WebTab *tab = nullptr, const QModelIndex &index = QModelIndex());
    ~TabTreeModelItem();

    void setParent(TabTreeModelItem *item);
    void addChild(TabTreeModelItem *item, int index = -1);

    WebTab *tab = nullptr;
    TabTreeModelItem *parent = nullptr;
    QVector<TabTreeModelItem*> children;
    QPersistentModelIndex sourceIndex;
};

TabTreeModelItem::TabTreeModelItem(WebTab *tab, const QModelIndex &index)
    : tab(tab)
    , sourceIndex(index)
{
}

TabTreeModelItem::~TabTreeModelItem()
{
    for (TabTreeModelItem *child : qAsConst(children)) {
        delete child;
    }
}

void TabTreeModelItem::setParent(TabTreeModelItem *item)
{
    if (parent == item) {
        return;
    }
    if (parent) {
        parent->children.removeOne(this);
    }
    parent = item;
    if (parent) {
        parent->children.append(this);
    }
}

void TabTreeModelItem::addChild(TabTreeModelItem *item, int index)
{
    item->setParent(nullptr);
    item->parent = this;
    if (index < 0 || index > children.size()) {
        children.append(item);
    } else {
        children.insert(index, item);
    }
}

TabTreeModel::TabTreeModel(BrowserWindow *window, QObject *parent)
    : QAbstractProxyModel(parent)
    , m_window(window)
{
    connect(m_window, &BrowserWindow::aboutToClose, this, &TabTreeModel::syncTopLevelTabs);

    connect(this, &QAbstractProxyModel::sourceModelChanged, this, &TabTreeModel::init);
}

TabTreeModel::~TabTreeModel()
{
    delete m_root;
}

QModelIndex TabTreeModel::tabIndex(WebTab *tab) const
{
    TabTreeModelItem *item = m_items.value(tab);
    if (!item) {
        return QModelIndex();
    }
    return createIndex(item->parent->children.indexOf(item), 0, item);
}

WebTab *TabTreeModel::tab(const QModelIndex &index) const
{
    TabTreeModelItem *it = item(index);
    return it ? it->tab : nullptr;
}

Qt::ItemFlags TabTreeModel::flags(const QModelIndex &index) const
{
    TabTreeModelItem *it = item(index);
    if (!it || !it->tab) {
        return Qt::ItemIsDropEnabled;
    }
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (!it->tab->isPinned()) {
        flags |= Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
    }
    return flags;
}

QVariant TabTreeModel::data(const QModelIndex &index, int role) const
{
    return sourceModel()->data(mapToSource(index), role);
}

int TabTreeModel::rowCount(const QModelIndex &parent) const
{
    TabTreeModelItem *it = item(parent);
    if (!it) {
        return 0;
    }
    return it->children.count();
}

int TabTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }
    return 1;
}

bool TabTreeModel::hasChildren(const QModelIndex &parent) const
{
    TabTreeModelItem *it = item(parent);
    if (!it) {
        return false;
    }
    return !it->children.isEmpty();
}

QModelIndex TabTreeModel::parent(const QModelIndex &child) const
{
    TabTreeModelItem *it = item(child);
    if (!it) {
        return QModelIndex();
    }
    return index(it->parent);
}

QModelIndex TabTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    TabTreeModelItem *parentItem = item(parent);
    return createIndex(row, column, parentItem->children.at(row));
}

QModelIndex TabTreeModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    return tabIndex(sourceIndex.data(TabModel::WebTabRole).value<WebTab*>());
}

QModelIndex TabTreeModel::mapToSource(const QModelIndex &proxyIndex) const
{
    TabTreeModelItem *it = item(proxyIndex);
    if (!it) {
        return QModelIndex();
    }
    return it->sourceIndex;
}

bool TabTreeModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(row)
    Q_UNUSED(parent)
    if (action != Qt::MoveAction || column > 0 || !m_window) {
        return false;
    }
    const TabModelMimeData *mimeData = qobject_cast<const TabModelMimeData*>(data);
    if (!mimeData) {
        return false;
    }
    WebTab *tab = mimeData->tab();
    if (!tab) {
        return false;
    }
    // This would require moving the entire tab tree
    if (tab->browserWindow() != m_window && !tab->childTabs().isEmpty()) {
        return false;
    }
    return true;
}

bool TabTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!canDropMimeData(data, action, row, column, parent)) {
        return false;
    }

    const TabModelMimeData *mimeData = static_cast<const TabModelMimeData*>(data);
    WebTab *tab = mimeData->tab();

    if (tab->isPinned()) {
        tab->togglePinned();
    }

    if (tab->browserWindow() != m_window) {
        if (tab->browserWindow()) {
            tab->browserWindow()->tabWidget()->detachTab(tab);
            m_window->tabWidget()->addView(tab, Qz::NT_SelectedTab);
        }
    }

    TabTreeModelItem *it = m_items.value(tab);
    TabTreeModelItem *parentItem = item(parent);
    if (!it || !parentItem) {
        return false;
    }
    if (it->parent == parentItem && row < 0) {
        return false;
    }
    if (!parentItem->tab) {
        tab->setParentTab(nullptr);
        if (row < 0) {
            row = m_root->children.count();
        }
        const QModelIndex fromIdx = index(it);
        const int childPos = row > fromIdx.row() ? row - 1 : row;
        if (!beginMoveRows(fromIdx.parent(), fromIdx.row(), fromIdx.row(), QModelIndex(), row)) {
            qWarning() << "Invalid beginMoveRows" << fromIdx.parent() << fromIdx.row() << "root" << row;
            return true;
        }
        m_root->addChild(it, childPos);
        endMoveRows();
    } else {
        parentItem->tab->addChildTab(tab, row);
    }

    return true;
}

void TabTreeModel::init()
{
    delete m_root;
    m_items.clear();

    m_root = new TabTreeModelItem;

    for (int i = 0; i < sourceModel()->rowCount(); ++i) {
        const QModelIndex index = sourceModel()->index(i, 0);
        WebTab *tab = index.data(TabModel::WebTabRole).value<WebTab*>();
        if (tab && !tab->parentTab()) {
            TabTreeModelItem *item = new TabTreeModelItem(tab, index);
            m_items[tab] = item;
            m_root->addChild(createItems(item));
        }
    }

    for (TabTreeModelItem *item : qAsConst(m_items)) {
        connectTab(item->tab);
    }

    connect(sourceModel(), &QAbstractItemModel::dataChanged, this, &TabTreeModel::sourceDataChanged, Qt::UniqueConnection);
    connect(sourceModel(), &QAbstractItemModel::rowsInserted, this, &TabTreeModel::sourceRowsInserted, Qt::UniqueConnection);
    connect(sourceModel(), &QAbstractItemModel::rowsAboutToBeRemoved, this, &TabTreeModel::sourceRowsAboutToBeRemoved, Qt::UniqueConnection);
    connect(sourceModel(), &QAbstractItemModel::modelReset, this, &TabTreeModel::sourceReset, Qt::UniqueConnection);
}

QModelIndex TabTreeModel::index(TabTreeModelItem *item) const
{
    if (!item || item == m_root) {
        return QModelIndex();
    }
    return createIndex(item->parent->children.indexOf(item), 0, item);
}

TabTreeModelItem *TabTreeModel::item(const QModelIndex &index) const
{
    TabTreeModelItem *it = static_cast<TabTreeModelItem*>(index.internalPointer());
    return it ? it : m_root;
}

TabTreeModelItem *TabTreeModel::createItems(TabTreeModelItem *root)
{
    const auto children = root->tab->childTabs();
    for (WebTab *child : children) {
        const QModelIndex index = sourceModel()->index(child->tabIndex(), 0);
        TabTreeModelItem *item = new TabTreeModelItem(child, index);
        m_items[child] = item;
        root->addChild(createItems(item));
    }
    return root;
}

void TabTreeModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
}

void TabTreeModel::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; ++i) {
        insertIndex(sourceModel()->index(i, 0, parent));
    }
}

void TabTreeModel::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; ++i) {
        removeIndex(sourceModel()->index(i, 0, parent));
    }
}

void TabTreeModel::sourceReset()
{
    beginResetModel();
    init();
    endResetModel();
}

void TabTreeModel::insertIndex(const QModelIndex &sourceIndex)
{
    WebTab *tab = sourceIndex.data(TabModel::WebTabRole).value<WebTab*>();
    if (!tab) {
        return;
    }
    TabTreeModelItem *parent = m_items.value(tab->parentTab());
    if (!parent) {
        parent = m_root;
    }
    TabTreeModelItem *item = new TabTreeModelItem(tab, sourceIndex);

    const int idx = parent->children.count();
    beginInsertRows(tabIndex(tab->parentTab()), idx, idx);
    m_items[tab] = item;
    parent->addChild(item);
    endInsertRows();

    connectTab(tab);
}

void TabTreeModel::removeIndex(const QModelIndex &sourceIndex)
{
    WebTab *tab = sourceIndex.data(TabModel::WebTabRole).value<WebTab*>();
    if (!tab) {
        return;
    }
    TabTreeModelItem *item = m_items.value(tab);
    if (!item) {
        return;
    }

    const QModelIndex index = mapFromSource(sourceIndex);
    beginRemoveRows(index.parent(), index.row(), index.row());
    item->setParent(nullptr);
    Q_ASSERT(item->children.isEmpty());
    delete item;
    endRemoveRows();

    tab->disconnect(this);
}

void TabTreeModel::connectTab(WebTab *tab)
{
    TabTreeModelItem *item = m_items.value(tab);
    Q_ASSERT(item);

    connect(tab, &WebTab::parentTabChanged, this, [=](WebTab *parent) {
        // Handle only move to root, everything else is done in childTabAdded
        if (item->parent == m_root || parent) {
            return;
        }
        int pos = m_root->children.count();
        // Move it to the same spot as old parent
        if (item->parent->parent == m_root) {
            pos = m_root->children.indexOf(item->parent);
        }
        const QModelIndex fromIdx = index(item);
        if (!beginMoveRows(fromIdx.parent(), fromIdx.row(), fromIdx.row(), QModelIndex(), pos)) {
            qWarning() << "Invalid beginMoveRows" << fromIdx.parent() << fromIdx.row() << "root" << pos;
            return;
        }
        m_root->addChild(item, pos);
        endMoveRows();
    });

    connect(tab, &WebTab::childTabAdded, this, [=](WebTab *child, int pos) {
        TabTreeModelItem *from = m_items.value(child);
        if (!from) {
            return;
        }
        const QModelIndex fromIdx = index(from);
        const QModelIndex toIdx = index(item);
        const int childPos = fromIdx.parent() == toIdx && pos > fromIdx.row() ? pos - 1 : pos;
        if (!beginMoveRows(fromIdx.parent(), fromIdx.row(), fromIdx.row(), toIdx, pos)) {
            qWarning() << "Invalid beginMoveRows" << fromIdx.parent() << fromIdx.row() << toIdx << pos;
            return;
        }
        item->addChild(from, childPos);
        endMoveRows();
    });
}

void TabTreeModel::syncTopLevelTabs()
{
    // Move all normal top-level tabs to the beginning to preserve order in session

    int index = -1;

    const auto items = m_root->children;
    for (TabTreeModelItem *item : items) {
        if (!item->tab->isPinned()) {
            const int tabIndex = item->tab->tabIndex();
            if (index < 0 || tabIndex < index) {
                index = tabIndex;
            }
        }
    }

    if (index < 0) {
        return;
    }

    for (TabTreeModelItem *item : items) {
        if (!item->tab->isPinned()) {
            item->tab->moveTab(index++);
        }
    }
}
