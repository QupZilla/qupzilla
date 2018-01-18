/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#ifndef BOOKMARKSITEMDELEGATE_H
#define BOOKMARKSITEMDELEGATE_H

#include <QStyledItemDelegate>

#include "qzcommon.h"

class QTreeView;

class QUPZILLA_EXPORT BookmarksItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit BookmarksItemDelegate(QTreeView* parent = 0);

    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QTreeView* m_tree;
    mutable QRect m_lastRect;
};

#endif // BOOKMARKSITEMDELEGATE_H
