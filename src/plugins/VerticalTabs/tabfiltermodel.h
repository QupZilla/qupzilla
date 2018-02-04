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
#pragma once

#include <QSortFilterProxyModel>

class TabFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit TabFilterModel(QObject *parent = nullptr);

    void resetFilter();

    void setFilterPinnedTabs(bool pinned);

    void setRejectDropOnLastIndex(bool reject);

private:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

    enum Mode {
        NoFilter,
        FilterPinnedTabs
    };

    Mode m_mode = NoFilter;
    bool m_filterPinnedTabs = false;
    bool m_rejectDropOnLastIndex = false;
};
