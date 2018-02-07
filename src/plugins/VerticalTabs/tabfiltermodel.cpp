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
#include "tabfiltermodel.h"

#include "tabmodel.h"

TabFilterModel::TabFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void TabFilterModel::resetFilter()
{
    m_mode = NoFilter;
    invalidateFilter();
}

void TabFilterModel::setFilterPinnedTabs(bool filter)
{
    m_mode = FilterPinnedTabs;
    m_filterPinnedTabs = filter;
    invalidateFilter();
}

void TabFilterModel::setRejectDropOnLastIndex(bool reject)
{
    m_rejectDropOnLastIndex = reject;
}

bool TabFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_mode == NoFilter) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    return index.data(TabModel::PinnedRole).toBool() != m_filterPinnedTabs;
}

bool TabFilterModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    if (m_rejectDropOnLastIndex && row == rowCount()) {
        return false;
    }
    return QSortFilterProxyModel::canDropMimeData(data, action, row, column, parent);
}
