/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#ifndef HISTORYITEM_H
#define HISTORYITEM_H

#include <QIcon>

#include "qzcommon.h"
#include "history.h"

class QUPZILLA_EXPORT HistoryItem
{
public:
    explicit HistoryItem(HistoryItem* parent = 0);
    ~HistoryItem();

    void changeParent(HistoryItem* parent);
    HistoryItem* parent() const;

    HistoryItem* child(int row) const;
    int childCount() const;

    void prependChild(HistoryItem* child);
    void appendChild(HistoryItem* child);
    void insertChild(int row, HistoryItem* child);

    void removeChild(int row);
    void removeChild(HistoryItem* child);

    int row();
    int indexOfChild(HistoryItem* child);

    bool isTopLevel() const;

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    void setStartTimestamp(qint64 start);
    qint64 startTimestamp() const;

    void setEndTimestamp(qint64 end);
    qint64 endTimestamp() const;

    HistoryEntry historyEntry;
    QString title;
    bool canFetchMore;

private:
    HistoryItem* m_parent;
    QList<HistoryItem*> m_children;

    QIcon m_icon;

    qint64 m_startTimestamp;
    qint64 m_endTimestamp;
};

#endif // HISTORYITEM_H
