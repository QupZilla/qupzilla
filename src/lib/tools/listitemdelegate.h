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
#ifndef LISTITEMDELEGATE_H
#define LISTITEMDELEGATE_H

#include <QStyledItemDelegate>

#include "qzcommon.h"

class QUPZILLA_EXPORT ListItemDelegate : public QStyledItemDelegate
{
public:
    explicit ListItemDelegate(int iconSize, QWidget* parent);

    void setUpdateParentHeight(bool update);
    void setUniformItemSizes(bool uniform);

    int itemHeight() const;

    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    int m_iconSize;
    bool m_updateParentHeight;
    bool m_uniformItemSizes;

    mutable int m_itemHeight;
    mutable int m_itemWidth;
    mutable int m_padding;
};

#endif // LISTITEMDELEGATE_H
