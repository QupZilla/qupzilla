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
    QTextDocument textDocument;
    textDocument.setHtml(text);

    QTextLayout textLayout(textDocument.begin());
    textLayout.setFont(option.font);

    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0) + 1;
    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding

    textLayout.beginLayout();
    qreal height = 0;
    while (1) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid()) {
            break;
        }

        line.setLineWidth(textRect.width());
        height += 3;
        line.setPosition(QPoint(0, height));
        height += line.height();
    }
    textLayout.endLayout();

    textLayout.draw(painter, QPointF(textRect.left(), textRect.top()));
}

QSize PluginListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QItemDelegate::sizeHint(option, index);
    size.setWidth(m_listWidget->width() - 5);
    size.setHeight(option.fontMetrics.height() * 4);

    return size;
}
