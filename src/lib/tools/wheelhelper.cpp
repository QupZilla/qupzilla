/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2017 David Rosca <nowrep@gmail.com>
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
#include "wheelhelper.h"

#include <QWheelEvent>

WheelHelper::WheelHelper()
{
}

void WheelHelper::reset()
{
    m_wheelDelta = 0;
    m_directions.clear();
}

void WheelHelper::processEvent(QWheelEvent *event)
{
    int delta = event->angleDelta().x() ? event->angleDelta().x() : event->angleDelta().y();
    bool directionY = delta == event->angleDelta().y();

    // When scroll to both directions, prefer the major one
    if (event->angleDelta().x() && event->angleDelta().y()) {
        if (std::abs(event->angleDelta().y()) > std::abs(event->angleDelta().x())) {
            delta = event->angleDelta().y();
            directionY = true;
        } else {
            delta = event->angleDelta().x();
            directionY = false;
        }
    }

    // Reset when direction changes
    if ((delta < 0 && m_wheelDelta > 0) || (delta > 0 && m_wheelDelta < 0)) {
        m_wheelDelta = 0;
    }

    m_wheelDelta += delta;

    // Angle delta 120 for common "one click"
    // See: http://qt-project.org/doc/qt-5/qml-qtquick-wheelevent.html#angleDelta-prop
    while (m_wheelDelta >= 120) {
        m_wheelDelta -= 120;
        if (directionY) {
            m_directions.enqueue(WheelUp);
        } else {
            m_directions.enqueue(WheelLeft);
        }
    }
    while (m_wheelDelta <= -120) {
        m_wheelDelta += 120;
        if (directionY) {
            m_directions.enqueue(WheelDown);
        } else {
            m_directions.enqueue(WheelRight);
        }
    }
}

WheelHelper::Direction WheelHelper::takeDirection()
{
    return m_directions.isEmpty() ? None : m_directions.dequeue();
}
