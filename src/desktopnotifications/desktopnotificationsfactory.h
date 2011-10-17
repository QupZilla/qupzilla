/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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

#include <QObject>
#ifdef Q_WS_X11
#include <QDBusInterface>
#endif
#include <QStringList>
#include <QSettings>
#include <QPoint>
#include <QTimer>
#include <QPointer>
#include <QDir>

class DesktopNotification;
class DesktopNotificationsFactory : public QObject
{
    Q_OBJECT
public:
    enum Type { DesktopNative, PopupWidget };
    explicit DesktopNotificationsFactory(QObject* parent = 0);
    void notify(const QPixmap &icon, const QString &heading, const QString &text);

signals:

public slots:
    void loadSettings();

private:
    bool m_enabled;
    int m_timeout;
    Type m_notifType;
    QPoint m_position;

    QPointer<DesktopNotification> m_desktopNotif;
    unsigned int m_uint;
};

#endif // DESKTOPNOTIFICATIONSFACTORY_H
