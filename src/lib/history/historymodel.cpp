/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#include "historymodel.h"
#include "historyitem.h"
#include "iconprovider.h"
#include "sqldatabase.h"

#include <QApplication>
#include <QDateTime>
#include <QTimer>

static QString dateTimeToString(const QDateTime &dateTime)
{
    const QDateTime current = QDateTime::currentDateTime();
    if (current.date() == dateTime.date()) {
        return dateTime.time().toString("h:mm");
    }

    return dateTime.toString("d.M.yyyy h:mm");
}

HistoryModel::HistoryModel(History* history)
    : QAbstractItemModel(history)
    , m_rootItem(new HistoryItem(0))
    , m_todayItem(0)
    , m_history(history)
{
    init();

    connect(m_history, SIGNAL(resetHistory()), this, SLOT(resetHistory()));
    connect(m_history, SIGNAL(historyEntryAdded(HistoryEntry)), this, SLOT(historyEntryAdded(HistoryEntry)));
    connect(m_history, SIGNAL(historyEntryDeleted(HistoryEntry)), this, SLOT(historyEntryDeleted(HistoryEntry)));
    connect(m_history, SIGNAL(historyEntryEdited(HistoryEntry,HistoryEntry)), this, SLOT(historyEntryEdited(HistoryEntry,HistoryEntry)));
}

QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Title");
        case 1:
            return tr("Address");
        case 2:
            return tr("Visit Date");
        case 3:
            return tr("Visit Count");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    HistoryItem* item = itemFromIndex(index);

    if (index.row() < 0 || !item) {
        return QVariant();
    }

    if (item->isTopLevel()) {
        switch (role) {
        case IsTopLevelRole:
            return true;
        case TimestampStartRole:
            return item->startTimestamp();
        case TimestampEndRole:
            return item->endTimestamp();
        case Qt::DisplayRole:
        case Qt::EditRole:
            return index.column() == 0 ? item->title : QVariant();
        case Qt::DecorationRole:
            return index.column() == 0 ? QIcon::fromTheme(QSL("view-calendar"), QIcon(":/icons/menu/history_entry.svg")) : QVariant();
        }

        return QVariant();
    }

    const HistoryEntry entry = item->historyEntry;

    switch (role) {
    case IdRole:
        return entry.id;
    case TitleRole:
        return entry.title;
    case UrlRole:
        return entry.url;
    case UrlStringRole:
        return entry.urlString;
    case IconRole:
        return item->icon();
    case IsTopLevelRole:
        return false;
    case TimestampStartRole:
        return -1;
    case TimestampEndRole:
        return -1;
    case Qt::ToolTipRole:
        if (index.column() == 0) {
            return QString("%1\n%2").arg(entry.title, entry.urlString);
        }
        // fallthrough
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (index.column()) {
        case 0:
            return entry.title;
        case 1:
            return entry.urlString;
        case 2:
            return dateTimeToString(entry.date);
        case 3:
            return entry.count;
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            return item->icon().isNull() ? IconProvider::emptyWebIcon() : item->icon();
        }
    }

    return QVariant();
}

bool HistoryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    HistoryItem* item = itemFromIndex(index);

    if (index.row() < 0 || !item || item->isTopLevel()) {
        return false;
    }

    if (role == IconRole) {
        item->setIcon(value.value<QIcon>());
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

QModelIndex HistoryModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    HistoryItem* parentItem = itemFromIndex(parent);
    HistoryItem* childItem = parentItem->child(row);

    return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex HistoryModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    HistoryItem* childItem = itemFromIndex(index);
    HistoryItem* parentItem = childItem->parent();

    if (!parentItem || parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

Qt::ItemFlags HistoryModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    HistoryItem* parentItem = itemFromIndex(parent);

    return parentItem->childCount();
}

int HistoryModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 4;
}

bool HistoryModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return true;
    }

    HistoryItem* item = itemFromIndex(parent);

    return item ? item->isTopLevel() : false;
}

HistoryItem* HistoryModel::itemFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        HistoryItem* item = static_cast<HistoryItem*>(index.internalPointer());

        if (item) {
            return item;
        }
    }

    return m_rootItem;
}

void HistoryModel::removeTopLevelIndexes(const QList<QPersistentModelIndex> &indexes)
{
    foreach (const QPersistentModelIndex &index, indexes) {
        if (index.parent().isValid()) {
            continue;
        }

        int row = index.row();
        HistoryItem* item = m_rootItem->child(row);

        if (!item) {
            return;
        }

        beginRemoveRows(QModelIndex(), row, row);
        delete item;
        endRemoveRows();

        if (item == m_todayItem) {
            m_todayItem = 0;
        }
    }
}

void HistoryModel::resetHistory()
{
    beginResetModel();

    delete m_rootItem;
    m_todayItem = 0;
    m_rootItem = new HistoryItem(0);

    init();

    endResetModel();
}

bool HistoryModel::canFetchMore(const QModelIndex &parent) const
{
    HistoryItem* parentItem = itemFromIndex(parent);

    return parentItem ? parentItem->canFetchMore : false;
}

void HistoryModel::fetchMore(const QModelIndex &parent)
{
    HistoryItem* parentItem = itemFromIndex(parent);

    if (!parent.isValid() || !parentItem) {
        return;
    }

    parentItem->canFetchMore = false;

    QList<int> idList;
    for (int i = 0; i < parentItem->childCount(); ++i) {
        idList.append(parentItem->child(i)->historyEntry.id);
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT id, count, title, url, date FROM history WHERE date BETWEEN ? AND ? ORDER BY date DESC");
    query.addBindValue(parentItem->endTimestamp());
    query.addBindValue(parentItem->startTimestamp());
    query.exec();

    QVector<HistoryEntry> list;

    while (query.next()) {
        HistoryEntry entry;
        entry.id = query.value(0).toInt();
        entry.count = query.value(1).toInt();
        entry.title = query.value(2).toString();
        entry.url = query.value(3).toUrl();
        entry.date = QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong());
        entry.urlString = entry.url.toEncoded();

        if (!idList.contains(entry.id)) {
            list.append(entry);
        }
    }

    if (list.isEmpty()) {
        return;
    }

    beginInsertRows(parent, 0, list.size() - 1);

    foreach (const HistoryEntry &entry, list) {
        HistoryItem* newItem = new HistoryItem(parentItem);
        newItem->historyEntry = entry;
    }

    endInsertRows();
}

void HistoryModel::historyEntryAdded(const HistoryEntry &entry)
{
    if (!m_todayItem) {
        beginInsertRows(QModelIndex(), 0, 0);

        m_todayItem = new HistoryItem(0);
        m_todayItem->setStartTimestamp(-1);
        m_todayItem->setEndTimestamp(QDateTime(QDate::currentDate()).toMSecsSinceEpoch());
        m_todayItem->title = tr("Today");

        m_rootItem->prependChild(m_todayItem);

        endInsertRows();
    }

    beginInsertRows(createIndex(0, 0, m_todayItem), 0, 0);

    HistoryItem* item = new HistoryItem();
    item->historyEntry = entry;

    m_todayItem->prependChild(item);

    endInsertRows();
}

void HistoryModel::historyEntryDeleted(const HistoryEntry &entry)
{
    HistoryItem* item = findHistoryItem(entry);
    if (!item) {
        return;
    }

    HistoryItem* parentItem = item->parent();
    int row = item->row();

    beginRemoveRows(createIndex(parentItem->row(), 0, parentItem), row, row);
    delete item;
    endRemoveRows();

    checkEmptyParentItem(parentItem);
}

void HistoryModel::historyEntryEdited(const HistoryEntry &before, const HistoryEntry &after)
{
#if 0
    HistoryItem* item = findHistoryItem(before);

    if (item) {
        HistoryItem* parentItem = item->parent();
        const QModelIndex sourceParent = createIndex(parentItem->row(), 0, parentItem);
        const QModelIndex destinationParent = createIndex(m_todayItem->row(), 0, m_todayItem);
        int row = item->row();

        beginMoveRows(sourceParent, row, row, destinationParent, 0);
        item->historyEntry = after;
        item->refreshIcon();
        item->changeParent(m_todayItem);
        endMoveRows(); // This line sometimes throw "std::bad_alloc" ... I don't know why ?!

        checkEmptyParentItem(parentItem);
    }
    else {
        historyEntryAdded(after);
    }
#endif
    historyEntryDeleted(before);
    historyEntryAdded(after);
}

HistoryItem* HistoryModel::findHistoryItem(const HistoryEntry &entry)
{
    HistoryItem* parentItem = 0;
    qint64 timestamp = entry.date.toMSecsSinceEpoch();

    for (int i = 0; i < m_rootItem->childCount(); ++i) {
        HistoryItem* item = m_rootItem->child(i);

        if (item->endTimestamp() < timestamp) {
            parentItem = item;
            break;
        }
    }

    if (!parentItem) {
        return 0;
    }

    for (int i = 0; i < parentItem->childCount(); ++i) {
        HistoryItem* item = parentItem->child(i);
        if (item->historyEntry.id == entry.id) {
            return item;
        }
    }

    return 0;
}

void HistoryModel::checkEmptyParentItem(HistoryItem* item)
{
    if (item->childCount() == 0 && item->isTopLevel()) {
        int row = item->row();

        beginRemoveRows(QModelIndex(), row, row);
        delete item;
        endRemoveRows();

        if (item == m_todayItem) {
            m_todayItem = 0;
        }
    }
}

void HistoryModel::init()
{
    QSqlQuery query(SqlDatabase::instance()->database());
    query.exec("SELECT MIN(date) FROM history");
    if (!query.next()) {
        return;
    }

    const qint64 minTimestamp = query.value(0).toLongLong();
    if (minTimestamp <= 0) {
        return;
    }

    const QDate today = QDate::currentDate();
    const QDate week = today.addDays(1 - today.dayOfWeek());
    const QDate month = QDate(today.year(), today.month(), 1);
    const qint64 currentTimestamp = QDateTime::currentMSecsSinceEpoch();

    qint64 timestamp = currentTimestamp;
    while (timestamp > minTimestamp) {
        QDate timestampDate = QDateTime::fromMSecsSinceEpoch(timestamp).date();
        qint64 endTimestamp;
        QString itemName;

        if (timestampDate == today) {
            endTimestamp = QDateTime(today).toMSecsSinceEpoch();

            itemName = tr("Today");
        }
        else if (timestampDate >= week) {
            endTimestamp = QDateTime(week).toMSecsSinceEpoch();

            itemName = tr("This Week");
        }
        else if (timestampDate.month() == month.month() && timestampDate.year() == month.year()) {
            endTimestamp = QDateTime(month).toMSecsSinceEpoch();

            itemName = tr("This Month");
        }
        else {
            QDate startDate(timestampDate.year(), timestampDate.month(), timestampDate.daysInMonth());
            QDate endDate(startDate.year(), startDate.month(), 1);

            timestamp = QDateTime(startDate, QTime(23, 59, 59)).toMSecsSinceEpoch();
            endTimestamp = QDateTime(endDate).toMSecsSinceEpoch();
            itemName = QString("%1 %2").arg(History::titleCaseLocalizedMonth(timestampDate.month()), QString::number(timestampDate.year()));
        }

        QSqlQuery query(SqlDatabase::instance()->database());
        query.prepare("SELECT id FROM history WHERE date BETWEEN ? AND ? LIMIT 1");
        query.addBindValue(endTimestamp);
        query.addBindValue(timestamp);
        query.exec();

        if (query.next()) {
            HistoryItem* item = new HistoryItem(m_rootItem);
            item->setStartTimestamp(timestamp == currentTimestamp ? -1 : timestamp);
            item->setEndTimestamp(endTimestamp);
            item->title = itemName;
            item->canFetchMore = true;

            if (timestamp == currentTimestamp) {
                m_todayItem = item;
            }
        }

        timestamp = endTimestamp - 1;
    }
}

// HistoryFilterModel
HistoryFilterModel::HistoryFilterModel(QAbstractItemModel* parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(parent);
    setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_filterTimer = new QTimer(this);
    m_filterTimer->setSingleShot(true);
    m_filterTimer->setInterval(300);

    connect(m_filterTimer, SIGNAL(timeout()), this, SLOT(startFiltering()));
}

void HistoryFilterModel::setFilterFixedString(const QString &pattern)
{
    m_pattern = pattern;

    m_filterTimer->start();
}

void HistoryFilterModel::startFiltering()
{
    if (m_pattern.isEmpty()) {
        emit collapseAllItems();
        QSortFilterProxyModel::setFilterFixedString(m_pattern);
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Expand all items also calls fetchmore
    emit expandAllItems();

    QSortFilterProxyModel::setFilterFixedString(m_pattern);

    QApplication::restoreOverrideCursor();
}

bool HistoryFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (index.data(HistoryModel::IsTopLevelRole).toBool()) {
        return true;
    }

    return (index.data(HistoryModel::UrlStringRole).toString().contains(m_pattern, Qt::CaseInsensitive) ||
            index.data(HistoryModel::TitleRole).toString().contains(m_pattern, Qt::CaseInsensitive));
}
