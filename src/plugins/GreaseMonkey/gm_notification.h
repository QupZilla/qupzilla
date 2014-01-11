/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2014  David Rosca <nowrep@gmail.com>
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
#ifndef GM_NOTIFICATION_H
#define GM_NOTIFICATION_H

#include "animatedwidget.h"

namespace Ui
{
class GM_Notification;
}

class GM_Manager;
class GM_Script;

class GM_Notification : public AnimatedWidget
{
    Q_OBJECT

public:
    explicit GM_Notification(GM_Manager* manager, const QString &tmpfileName, const QString &fileName);
    ~GM_Notification();

private slots:
    void installScript();

private:
    Ui::GM_Notification* ui;

    GM_Manager* m_manager;

    QString m_tmpFileName;
    QString m_fileName;
};

#endif // GM_NOTIFICATION_H
