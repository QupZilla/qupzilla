/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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

#include "qzcommon.h"

#include <QObject>
#include <QPoint>
#include <QPointer>

class QPixmap;
class QDBusMessage;
class QDBusError;

class DesktopNotification;

class QUPZILLA_EXPORT DesktopNotificationsFactory : public QObject
{
    Q_OBJECT

public:
    enum Type { DesktopNative, PopupWidget };

    explicit DesktopNotificationsFactory(QObject* parent = 0);

    void loadSettings();

    bool supportsNativeNotifications() const;

    void showNotification(const QPixmap &icon, const QString &heading, const QString &text);
    void nativeNotificationPreview();

private slots:
#if defined(Q_OS_UNIX) && !defined(DISABLE_DBUS)
    void updateLastId(const QDBusMessage &msg);
    void error(const QDBusError &error);
#endif

private:
    bool m_enabled;
    int m_timeout;
    Type m_notifType;
    QPoint m_position;

    QPointer<DesktopNotification> m_desktopNotif;
    quint32 m_uint;
};

#endif // DESKTOPNOTIFICATIONSFACTORY_H
