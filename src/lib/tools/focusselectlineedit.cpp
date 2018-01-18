/* ============================================================
* QupZilla - Qt web browser
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
#include "focusselectlineedit.h"

#include <QFocusEvent>

FocusSelectLineEdit::FocusSelectLineEdit(QWidget* parent)
    : QLineEdit(parent)
    , m_mouseFocusReason(false)
{
}

void FocusSelectLineEdit::setFocus()
{
    selectAll();

    QLineEdit::setFocus();
}

void FocusSelectLineEdit::focusInEvent(QFocusEvent* event)
{
    m_mouseFocusReason = event->reason() == Qt::MouseFocusReason;
    selectAll();

    QLineEdit::focusInEvent(event);
}

void FocusSelectLineEdit::mousePressEvent(QMouseEvent* event)
{
    if (m_mouseFocusReason) {
        m_mouseFocusReason = false;
        return;
    }

    QLineEdit::mousePressEvent(event);
}
