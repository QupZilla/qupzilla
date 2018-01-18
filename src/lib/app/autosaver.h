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
#ifndef AUTOSAVER_H
#define AUTOSAVER_H

#include <QObject>
#include <QBasicTimer>

#include "qzcommon.h"

class QUPZILLA_EXPORT AutoSaver : public QObject
{
    Q_OBJECT

public:
    explicit AutoSaver(QObject* parent = 0);

    // Emits save() if timer is running. Call this from destructor.
    void saveIfNecessary();

public slots:
    // Tells AutoSaver that change occurred. Signal save() will be emitted after a delay
    void changeOccurred();

signals:
    void save();

private:
    void timerEvent(QTimerEvent* event);

    QBasicTimer m_timer;
};

#endif // AUTOSAVER_H
