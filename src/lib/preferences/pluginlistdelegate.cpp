/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "pluginlistdelegate.h"

#include <QPainter>
#include <QListWidget>
#include <QApplication>

PluginListDelegate::PluginListDelegate(QListWidget* parent)
    : QStyledItemDelegate(parent)
    , m_rowHeight(0)
    , m_padding(0)
{
}

void PluginListDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    const QWidget* w = opt.widget;
    const QStyle* style = w ? w->style() : QApplication::style();
    const Qt::LayoutDirection direction = w ? w->layoutDirection() : QApplication::layoutDirection();
    const int height = opt.rect.height();
    const int center = height / 2 + opt.rect.top();

    // Prepare title font
    QFont titleFont = opt.font;
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);

    const QFontMetrics titleMetrics(titleFont);
#ifdef Q_WS_WIN
    const QPalette::ColorRole colorRole = QPalette::Text;
#else
    const QPalette::ColorRole colorRole = opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text;
#endif

    int leftPosition = m_padding;
    int rightPosition = opt.rect.right() - m_padding;

    // Draw background
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w);

    // Draw checkbox
    const int checkboxSize = 18;
    const int checkboxYPos = center - (checkboxSize / 2);
    QStyleOptionViewItemV4 opt2 = opt;
    opt2.checkState == Qt::Checked ? opt2.state |= QStyle::State_On : opt2.state |= QStyle::State_Off;
    QRect styleCheckBoxRect = style->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt2, w);
    styleCheckBoxRect.setRect(leftPosition, checkboxYPos, styleCheckBoxRect.width(), styleCheckBoxRect.height());
    QRect visualCheckBoxRect = style->visualRect(direction, opt.rect, styleCheckBoxRect);
    opt2.rect = visualCheckBoxRect;
    style->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &opt2, painter, w);
    leftPosition = styleCheckBoxRect.right() + m_padding;

    // Draw icon
    const int iconSize = 32;
    const int iconYPos = center - (iconSize / 2);
    QRect iconRect(leftPosition, iconYPos, iconSize, iconSize);
    QRect visualIconRect = style->visualRect(direction, opt.rect, iconRect);
    QPixmap pixmap = index.data(Qt::DecorationRole).value<QIcon>().pixmap(iconSize);
    painter->drawPixmap(visualIconRect, pixmap);
    leftPosition = iconRect.right() + m_padding;

    // Draw plugin name
    const QString &name = index.data(Qt::DisplayRole).toString();
    const int leftTitleEdge = leftPosition + 2;
    const int rightTitleEdge = rightPosition - m_padding;
    const int leftPosForVersion = titleMetrics.width(name) + m_padding;
    QRect nameRect(leftTitleEdge, opt.rect.top() + m_padding, rightTitleEdge - leftTitleEdge, titleMetrics.height());
    QRect visualNameRect = style->visualRect(direction, opt.rect, nameRect);
    painter->setFont(titleFont);
    style->drawItemText(painter, visualNameRect, Qt::AlignLeft, opt.palette, true, name, colorRole);

    // Draw version
    const QString &version = index.data(Qt::UserRole).toString();
    QRect versionRect(nameRect.x() + leftPosForVersion, nameRect.y(), rightTitleEdge - leftPosForVersion, titleMetrics.height());
    QFont versionFont = titleFont;
    versionFont.setBold(false);
    painter->setFont(versionFont);
    style->drawItemText(painter, versionRect, Qt::AlignLeft, opt.palette, true, version, colorRole);

    // Draw info
    const int infoYPos = nameRect.bottom() + opt.fontMetrics.leading();
    QRect infoRect(visualNameRect.x(), infoYPos, nameRect.width(), opt.fontMetrics.height());
    const QString &info = opt.fontMetrics.elidedText(index.data(Qt::UserRole + 1).toString(), Qt::ElideRight, infoRect.width());
    painter->setFont(opt.font);
    style->drawItemText(painter, infoRect, Qt::TextSingleLine | Qt::AlignLeft, opt.palette, true, info, colorRole);

    // Draw description
    const int descriptionYPos = infoRect.bottom() + opt.fontMetrics.leading();
    QRect descriptionRect(infoRect.x(), descriptionYPos, infoRect.width(), opt.fontMetrics.height());
    const QString &description = opt.fontMetrics.elidedText(index.data(Qt::UserRole + 2).toString(), Qt::ElideRight, descriptionRect.width());
    style->drawItemText(painter, descriptionRect, Qt::TextSingleLine | Qt::AlignLeft, opt.palette, true, description, colorRole);
}

QSize PluginListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    if (!m_rowHeight) {
        QStyleOptionViewItemV4 opt(option);
        initStyleOption(&opt, index);

        const QWidget* w = opt.widget;
        const QStyle* style = w ? w->style() : QApplication::style();
        const int padding = style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0) + 1;

        QFont titleFont = opt.font;
        titleFont.setBold(true);
        titleFont.setPointSize(titleFont.pointSize() + 1);

        m_padding = padding > 5 ? padding : 5;

        const QFontMetrics titleMetrics(titleFont);

        m_rowHeight = 2 * m_padding + 2 * opt.fontMetrics.leading() + 2 * opt.fontMetrics.height() + titleMetrics.height();
    }

    return QSize(200, m_rowHeight);
}
