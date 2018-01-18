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
#include "frame.h"

#include <QMouseEvent>

Frame::Frame(QWidget* parent)
    : QFrame(parent)
{
}

void Frame::mousePressEvent(QMouseEvent* event)
{
    //If we proccess mouse events, then menu from bookmarkswidget
    //is going to close() with clicking in free space
    Q_UNUSED(event)
    event->accept();
}
