/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "listitemdelegate.h"

#include <QApplication>
#include <QPainter>

ListItemDelegate::ListItemDelegate(int iconSize, QWidget* parent)
    : QStyledItemDelegate(parent)
    , m_iconSize(iconSize)
    , m_updateParentHeight(false)
    , m_itemHeight(0)
    , m_padding(0)
{
}

void ListItemDelegate::setUpdateParentHeight(bool update)
{
    m_updateParentHeight = update;
}

int ListItemDelegate::itemHeight() const
{
    return m_itemHeight;
}

void ListItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    const QWidget* w = opt.widget;
    const QStyle* style = w ? w->style() : QApplication::style();
    const Qt::LayoutDirection direction = w ? w->layoutDirection() : QApplication::layoutDirection();

#ifdef Q_OS_WIN
    const QPalette::ColorRole colorRole = QPalette::Text;
#else
    const QPalette::ColorRole colorRole = opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text;
#endif

    int topPosition = opt.rect.top() + m_padding;

    // Draw background
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w);

    // Draw icon
    QRect iconRect(opt.rect.left() + (opt.rect.width() - m_iconSize) / 2, topPosition, m_iconSize, m_iconSize);
    QRect visualIconRect = style->visualRect(direction, opt.rect, iconRect);
    QPixmap pixmap = index.data(Qt::DecorationRole).value<QIcon>().pixmap(m_iconSize);
    painter->drawPixmap(visualIconRect, pixmap);
    topPosition += m_iconSize + m_padding;

    // Draw title
    const QString &title = index.data(Qt::DisplayRole).toString();
    const int leftTitleEdge = opt.rect.left() + m_padding;
    QRect titleRect(leftTitleEdge, topPosition, opt.rect.width() - 2 * m_padding, opt.fontMetrics.height());
    QRect visualTitleRect = style->visualRect(direction, opt.rect, titleRect);
    style->drawItemText(painter, visualTitleRect, Qt::AlignCenter, opt.palette, true, title, colorRole);
}

QSize ListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!m_itemHeight) {
        QStyleOptionViewItemV4 opt(option);
        initStyleOption(&opt, index);

        const QWidget* w = opt.widget;
        const QStyle* style = w ? w->style() : QApplication::style();
        const int padding = style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0) + 1;

        m_padding = padding > 5 ? padding : 5;

        m_itemHeight = 3 * m_padding + opt.fontMetrics.height() + m_iconSize;

        // Update height of parent widget
        QWidget* p = qobject_cast<QWidget*>(parent());
        if (p && m_updateParentHeight) {
            p->setFixedHeight(m_itemHeight);
        }
    }

    int width = 2 * m_padding + option.fontMetrics.width(index.data(Qt::DisplayRole).toString());
    return QSize(width > (m_iconSize + 2 * m_padding) ? width : m_iconSize + 2 * m_padding, m_itemHeight);
}
