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
#include "autosaver.h"

#include <QTimerEvent>

#define SAVE_DELAY 1000 * 10 // 10 seconds

AutoSaver::AutoSaver(QObject* parent)
    : QObject(parent)
{
}

void AutoSaver::saveIfNecessary()
{
    if (m_timer.isActive()) {
        m_timer.stop();
        emit save();
    }
}

void AutoSaver::changeOccurred()
{
    if (!m_timer.isActive()) {
        m_timer.start(SAVE_DELAY, this);
    }
}

void AutoSaver::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();
        emit save();
    }

    QObject::timerEvent(event);
}
