/* ============================================================
* FlashCookieManager plugin for QupZilla
* Copyright (C) 2014  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#ifndef FCM_Notification_H
#define FCM_Notification_H

#include "animatedwidget.h"

namespace Ui
{
class FCM_Notification;
}

class FCM_Plugin;

class FCM_Notification : public AnimatedWidget
{
    Q_OBJECT

public:
    FCM_Notification(FCM_Plugin* manager, int newOriginsCount);
    ~FCM_Notification();

private:
    Ui::FCM_Notification* ui;

    FCM_Plugin* m_manager;
};

#endif // FCM_Notification_H
