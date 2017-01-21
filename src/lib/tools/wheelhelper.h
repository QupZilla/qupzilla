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
#ifndef WHEELHELPER_H
#define WHEELHELPER_H

#include <QQueue>

#include "qzcommon.h"

class QWheelEvent;

class QUPZILLA_EXPORT WheelHelper
{
public:
    enum Direction {
        None = 0,
        WheelUp,
        WheelDown,
        WheelLeft,
        WheelRight
    };

    explicit WheelHelper();

    void reset();
    void processEvent(QWheelEvent *event);
    Direction takeDirection();

private:
    int m_wheelDelta = 0;
    QQueue<Direction> m_directions;
};

#endif // WHEELHELPER_H
