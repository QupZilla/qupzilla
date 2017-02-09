/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "qzcommon.h"
#include "history.h"

class QTimer;

class History;
class HistoryItem;

class QUPZILLA_EXPORT HistoryModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TitleRole = Qt::UserRole + 2,
        UrlRole = Qt::UserRole + 3,
        UrlStringRole = Qt::UserRole + 4,
        IconRole = Qt::UserRole + 5,
        IsTopLevelRole = Qt::UserRole + 7,
        TimestampStartRole = Qt::UserRole + 8,
        TimestampEndRole = Qt::UserRole + 9,
        MaxRole = TimestampEndRole
    };

    explicit HistoryModel(History* history);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    bool hasChildren(const QModelIndex &parent) const;

    HistoryItem* itemFromIndex(const QModelIndex &index) const;

    void removeTopLevelIndexes(const QList<QPersistentModelIndex> &indexes);

signals:

private slots:
    void resetHistory();

    void historyEntryAdded(const HistoryEntry &entry);
    void historyEntryDeleted(const HistoryEntry &entry);
    void historyEntryEdited(const HistoryEntry &before, const HistoryEntry &after);

private:
    HistoryItem* findHistoryItem(const HistoryEntry &entry);
    void checkEmptyParentItem(HistoryItem* item);
    void init();

    HistoryItem* m_rootItem;
    HistoryItem* m_todayItem;
    History* m_history;
};

class QUPZILLA_EXPORT HistoryFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit HistoryFilterModel(QAbstractItemModel* parent);

public slots:
    void setFilterFixedString(const QString &pattern);

signals:
    void expandAllItems();
    void collapseAllItems();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private slots:
    void startFiltering();

private:
    QString m_pattern;
    QTimer* m_filterTimer;
};

#endif // HISTORYMODEL_H
