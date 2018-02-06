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
#include "tabmrumodel.h"
#include "tabmodel.h"
#include "webtab.h"
#include "tabwidget.h"
#include "browserwindow.h"

class TabMruModelItem
{
public:
    explicit TabMruModelItem(WebTab *tab = nullptr, const QModelIndex &index = QModelIndex());
    ~TabMruModelItem();

    WebTab *tab = nullptr;
    QVector<TabMruModelItem*> children;
    QPersistentModelIndex sourceIndex;
};

TabMruModelItem::TabMruModelItem(WebTab *tab, const QModelIndex &index)
    : tab(tab)
    , sourceIndex(index)
{
}

TabMruModelItem::~TabMruModelItem()
{
    qDeleteAll(children);
}

TabMruModel::TabMruModel(BrowserWindow *window, QObject *parent)
    : QAbstractProxyModel(parent)
    , m_window(window)
{
    connect(this, &QAbstractProxyModel::sourceModelChanged, this, &TabMruModel::init);
}

TabMruModel::~TabMruModel()
{
    delete m_root;
}

QModelIndex TabMruModel::tabIndex(WebTab *tab) const
{
    TabMruModelItem *item = m_items.value(tab);
    return item ? createIndex(m_root->children.indexOf(item), 0, item) : QModelIndex();
}

WebTab *TabMruModel::tab(const QModelIndex &index) const
{
    TabMruModelItem *it = item(index);
    return it ? it->tab : nullptr;
}

Qt::ItemFlags TabMruModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int TabMruModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_items.count();
}

int TabMruModel::columnCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }
    return 1;
}

QModelIndex TabMruModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

QModelIndex TabMruModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    return createIndex(row, column, m_root->children.at(row));
}

QModelIndex TabMruModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    return tabIndex(sourceIndex.data(TabModel::WebTabRole).value<WebTab*>());
}

QModelIndex TabMruModel::mapToSource(const QModelIndex &proxyIndex) const
{
    TabMruModelItem *it = item(proxyIndex);
    if (!it) {
        return QModelIndex();
    }
    return it->sourceIndex;
}

void TabMruModel::init()
{
    delete m_root;
    m_items.clear();

    m_root = new TabMruModelItem;
    sourceRowsInserted(QModelIndex(), 0, sourceModel()->rowCount());
    currentTabChanged(m_window->tabWidget()->currentIndex());

    connect(m_window->tabWidget(), &TabWidget::currentChanged, this, &TabMruModel::currentTabChanged, Qt::UniqueConnection);
    connect(sourceModel(), &QAbstractItemModel::dataChanged, this, &TabMruModel::sourceDataChanged, Qt::UniqueConnection);
    connect(sourceModel(), &QAbstractItemModel::rowsInserted, this, &TabMruModel::sourceRowsInserted, Qt::UniqueConnection);
    connect(sourceModel(), &QAbstractItemModel::rowsAboutToBeRemoved, this, &TabMruModel::sourceRowsAboutToBeRemoved, Qt::UniqueConnection);
    connect(sourceModel(), &QAbstractItemModel::modelReset, this, &TabMruModel::sourceReset, Qt::UniqueConnection);
}

QModelIndex TabMruModel::index(TabMruModelItem *item) const
{
    if (!item || item == m_root) {
        return QModelIndex();
    }
    return createIndex(m_root->children.indexOf(item), 0, item);
}

TabMruModelItem *TabMruModel::item(const QModelIndex &index) const
{
    return static_cast<TabMruModelItem*>(index.internalPointer());
}

void TabMruModel::currentTabChanged(int index)
{
    TabMruModelItem *it = item(mapFromSource(sourceModel()->index(index, 0)));
    if (!it) {
        return;
    }
    const int from = m_root->children.indexOf(it);
    if (from == 0) {
        return;
    }
    if (!beginMoveRows(QModelIndex(), from, from, QModelIndex(), 0)) {
        qWarning() << "Invalid beginMoveRows" << from;
        return;
    }
    m_root->children.removeAt(from);
    m_root->children.insert(0, it);
    endMoveRows();
}

void TabMruModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
}

void TabMruModel::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; ++i) {
        const QModelIndex index = sourceModel()->index(i, 0, parent);
        WebTab *tab = index.data(TabModel::WebTabRole).value<WebTab*>();
        if (tab) {
            beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
            TabMruModelItem *item = new TabMruModelItem(tab, index);
            m_items[tab] = item;
            m_root->children.append(item);
            endInsertRows();
        }
    }
}

void TabMruModel::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; ++i) {
        const QModelIndex index = sourceModel()->index(i, 0, parent);
        TabMruModelItem *it = item(mapFromSource(index));
        if (it) {
            const int idx = m_root->children.indexOf(it);
            beginRemoveRows(QModelIndex(), idx, idx);
            m_items.remove(it->tab);
            m_root->children.removeAt(idx);
            delete it;
            endRemoveRows();
        }
    }
}

void TabMruModel::sourceReset()
{
    beginResetModel();
    init();
    endResetModel();
}
