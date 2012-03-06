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
#include <QTextLayout>
#include <QTextDocument>
#include <QTextBlock>
#include <QApplication>

PluginListDelegate::PluginListDelegate(QListWidget* parent)
    : QItemDelegate(parent)
    , m_listWidget(parent)
{
}

void PluginListDelegate::drawDisplay(QPainter* painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;

    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, option.palette.brush(cg, QPalette::Highlight));
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    }
    else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    QTextDocument textDocument;
    textDocument.setHtml(text);

    QTextLayout textLayout(textDocument.begin());
    textLayout.setFont(option.font);

    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0) + 1;
    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding

    textLayout.beginLayout();
    qreal height = 0;
    QTextLine line = textLayout.createLine();

    while (line.isValid()) {
        line.setLineWidth(textRect.width());
        height += 3;
        line.setPosition(QPoint(0, height));
        height += line.height();

        line = textLayout.createLine();
    }

    textLayout.endLayout();
    textLayout.draw(painter, QPointF(textRect.left(), textRect.top()));
}

QSize PluginListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0) + 1;

    QSize size;
    size.setWidth(m_listWidget->width() - 10);

    // ( height of font * 3 = 3 lines )  + ( text margins ) + ( 2 free lines = every line is 3px )
    size.setHeight((option.fontMetrics.height() * 3) + (textMargin * 2) + (2 * 3));

    return size;
}
