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
#include "goicon.h"

#include <QMouseEvent>

GoIcon::GoIcon(QWidget* parent)
    : ClickableLabel(parent)
{
    setObjectName("locationbar-goicon");
    setCursor(Qt::PointingHandCursor);
    setHidden(true);
}

void GoIcon::contextMenuEvent(QContextMenuEvent* ev)
{
    // Prevent propagating to LocationBar
    ev->accept();
}


void GoIcon::mousePressEvent(QMouseEvent* ev)
{
    ClickableLabel::mousePressEvent(ev);

    // Prevent propagating to LocationBar
    ev->accept();
}
