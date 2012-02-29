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
#include "clickablelabel.h"

#include <QMouseEvent>

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
{
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton && rect().contains(ev->pos())) {
        if (ev->modifiers() == Qt::ControlModifier) {
            emit middleClicked(ev->globalPos());
        }
        else {
            emit clicked(ev->globalPos());
        }
    }
    else if (ev->button() == Qt::MiddleButton && rect().contains(ev->pos())) {
        emit middleClicked(ev->globalPos());
    }
    else {
        QLabel::mouseReleaseEvent(ev);
    }
}
