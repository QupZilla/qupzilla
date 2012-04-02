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
#ifndef DESKTOPNOTIFICATIONSFACTORY_H
#define DESKTOPNOTIFICATIONSFACTORY_H

#include "qz_namespace.h"

#include <QObject>
#include <QPoint>
#include <QWeakPointer>

class QPixmap;

class DesktopNotification;

class QT_QUPZILLA_EXPORT DesktopNotificationsFactory : public QObject
{
public:
    enum Type { DesktopNative, PopupWidget };

    explicit DesktopNotificationsFactory(QObject* parent = 0);

    void loadSettings();

    void showNotifications(const QPixmap &icon, const QString &heading, const QString &text);
    void nativeNotificationPreview();

private:
    bool m_enabled;
    int m_timeout;
    Type m_notifType;
    QPoint m_position;

    QWeakPointer<DesktopNotification> m_desktopNotif;
    unsigned int m_uint;
};

#endif // DESKTOPNOTIFICATIONSFACTORY_H
