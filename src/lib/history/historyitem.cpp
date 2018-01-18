/* ============================================================
* QupZilla - WebKit based browser
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
#include "historyitem.h"
#include "qztools.h"

HistoryItem::HistoryItem(HistoryItem* parent)
    : canFetchMore(false)
    , m_parent(parent)
    , m_startTimestamp(0)
    , m_endTimestamp(0)
{
    if (m_parent) {
        m_parent->appendChild(this);
    }
}

void HistoryItem::changeParent(HistoryItem* parent)
{
    if (m_parent) {
        m_parent->removeChild(this);
    }

    m_parent = parent;

    if (m_parent) {
        m_parent->prependChild(this);
    }
}

HistoryItem* HistoryItem::parent() const
{
    return m_parent;
}

void HistoryItem::prependChild(HistoryItem* child)
{
    if (m_children.contains(child)) {
        m_children.removeAll(child);
    }

    child->m_parent = this;
    m_children.prepend(child);
}

void HistoryItem::appendChild(HistoryItem* child)
{
    if (m_children.contains(child)) {
        m_children.removeAll(child);
    }

    child->m_parent = this;
    m_children.append(child);
}

void HistoryItem::insertChild(int row, HistoryItem* child)
{
    if (m_children.contains(child)) {
        m_children.removeAll(child);
    }

    if (m_children.count() >= row) {
        child->m_parent = this;
        m_children.insert(row, child);
    }
}

void HistoryItem::removeChild(int row)
{
    if (QzTools::containsIndex(m_children, row)) {
        removeChild(m_children.at(row));
    }
}

void HistoryItem::removeChild(HistoryItem* child)
{
    m_children.removeOne(child);
}

HistoryItem* HistoryItem::child(int row) const
{
    if (QzTools::containsIndex(m_children, row)) {
        return m_children.at(row);
    }

    return 0;
}

int HistoryItem::childCount() const
{
    return m_children.count();
}

int HistoryItem::row()
{
    return m_parent ? m_parent->indexOfChild(this) : 0;
}

int HistoryItem::indexOfChild(HistoryItem* child)
{
    return m_children.indexOf(child);
}

bool HistoryItem::isTopLevel() const
{
    return (m_startTimestamp != 0);
}

QIcon HistoryItem::icon() const
{
    return m_icon;
}

void HistoryItem::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

void HistoryItem::setStartTimestamp(qint64 start)
{
    m_startTimestamp = start;
}

qint64 HistoryItem::startTimestamp() const
{
    if (m_startTimestamp == -1) {
        return QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    return m_startTimestamp;
}

void HistoryItem::setEndTimestamp(qint64 end)
{
    m_endTimestamp = end;

}

qint64 HistoryItem::endTimestamp() const
{
    return m_endTimestamp;
}

HistoryItem::~HistoryItem()
{
    if (m_parent) {
        m_parent->removeChild(this);
    }

    qDeleteAll(m_children);
}
