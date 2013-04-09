/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#ifndef ACTIONCOPY_H
#define ACTIONCOPY_H

#include <QAction>

#include "qz_namespace.h"

class QT_QUPZILLA_EXPORT ActionCopy : public QAction
{
    Q_OBJECT
public:
    explicit ActionCopy(QAction* original, QObject* parent = 0);

private slots:
    void updateAction();

    void actionToggled();
    void actionTriggered();

private:
    QAction* m_action;

};

#endif // ACTIONCOPY_H
