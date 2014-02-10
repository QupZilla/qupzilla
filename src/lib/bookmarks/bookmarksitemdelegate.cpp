/* ============================================================
* QupZilla - WebKit based browser
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
#include "bookmarksitemdelegate.h"
#include "bookmarkstreeview.h"
#include "bookmarksmodel.h"
#include "bookmarkitem.h"

#include <QStyleOptionFrameV3>
#include <QApplication>
#include <QStyle>

BookmarksItemDelegate::BookmarksItemDelegate(BookmarksTreeView* parent)
    : QStyledItemDelegate(parent)
    , m_tree(parent)
{
}

void BookmarksItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    if (index.data(BookmarksModel::TypeRole).toInt() == BookmarkItem::Separator) {
        QStyleOptionFrameV3 opt;
        opt.frameShape = QFrame::HLine;
        opt.rect = option.rect;

        // We need to fake continuous line over 2 columns
        if (m_tree->viewType() == BookmarksTreeView::BookmarksManagerViewType) {
            if (index.column() == 1) {
                opt.rect = m_lastRect;
            }
            else {
                opt.rect.setWidth(opt.rect.width() + m_tree->columnWidth(1));
                m_lastRect = opt.rect;
            }
        }

        QApplication::style()->drawControl(QStyle::CE_ShapedFrame, &opt, painter);
    }
}
