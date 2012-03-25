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
#include "squeezelabelv2.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>

SqueezeLabelV2::SqueezeLabelV2(QWidget* parent)
    : QLabel(parent)
{
}

SqueezeLabelV2::SqueezeLabelV2(const QString &string)
    : QLabel()
{
    setText(string);
}

void SqueezeLabelV2::setText(const QString &txt)
{
    m_originalText = txt;
    QFontMetrics fm = fontMetrics();
    QString elided = fm.elidedText(m_originalText, Qt::ElideMiddle, width());
    QLabel::setText(elided);
}

void SqueezeLabelV2::copy()
{
    if (selectedText().length() == text().length()) {
        QApplication::clipboard()->setText(m_originalText);
    }
    else {
        QApplication::clipboard()->setText(selectedText());
    }
}

void SqueezeLabelV2::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu;
    QAction* act = menu.addAction(tr("Copy"), this, SLOT(copy()));
    act->setShortcut(QKeySequence("Ctrl+C"));
    act->setEnabled(hasSelectedText());

    menu.exec(event->globalPos());
}

void SqueezeLabelV2::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier) {
        copy();
    }
}

QString SqueezeLabelV2::originalText()
{
    return m_originalText;
}

void SqueezeLabelV2::resizeEvent(QResizeEvent* event)
{
    QLabel::resizeEvent(event);
    QFontMetrics fm = fontMetrics();
    QString elided = fm.elidedText(m_originalText, Qt::ElideMiddle, width());
    QLabel::setText(elided);
}
