/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/
/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "plaineditwithlines.h"

#include <QPainter>
#include <QTextBlock>

PlainEditWithLines::PlainEditWithLines(QWidget* parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
    , m_isShowingCursor(false)
    , m_countCache(-1, -1)
{
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int PlainEditWithLines::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 5 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void PlainEditWithLines::setShowingCursor(bool show)
{
    m_isShowingCursor = show;
}

bool PlainEditWithLines::isShowingCursor() const
{
    return m_isShowingCursor;
}

void PlainEditWithLines::setReadOnly(bool ro)
{
    QPlainTextEdit::setReadOnly(ro);

    highlightCurrentLine();
}

void PlainEditWithLines::goToLine(int line)
{
    setUpdatesEnabled(false);

    moveCursor(QTextCursor::Start);

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line - 1);

    setTextCursor(cursor);

    setUpdatesEnabled(true);
}

void PlainEditWithLines::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void PlainEditWithLines::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
    }
    else if (m_countCache.first != blockCount() || m_countCache.second != textCursor().block().lineCount()) {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
        m_countCache.first = blockCount();
        m_countCache.second = textCursor().block().lineCount();
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void PlainEditWithLines::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void PlainEditWithLines::highlightCurrentLine()
{
    if (!m_isShowingCursor) {
        return;
    }

    const QColor lineColor = palette().color(QPalette::Highlight).lighter();
    QList<QTextEdit::ExtraSelection> selectionsList;
    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, QVariant(true));
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    selectionsList.append(selection);

    setExtraSelections(selectionsList);
}

void PlainEditWithLines::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    const QRect eventRect = event->rect();

    QPainter painter(m_lineNumberArea);
    painter.fillRect(eventRect, QColor(235, 235, 235));
    painter.fillRect(QRect(eventRect.width() - 1, 0, 1, viewport()->height()), QColor(175, 175, 175));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    QColor textColor(175, 175, 175);

    while (block.isValid() && top <= eventRect.bottom()) {
        if (block.isVisible() && bottom >= eventRect.top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(textColor);

            painter.drawText(0, top, m_lineNumberArea->width() - 3, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

LineNumberArea::LineNumberArea(PlainEditWithLines* editor)
    : QWidget(editor)
    , m_codeEditor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent* event)
{
    m_codeEditor->lineNumberAreaPaintEvent(event);
}
