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
#include "desktopnotificationsfactory.h"
#include "desktopnotification.h"
#include "mainapplication.h"

DesktopNotificationsFactory::DesktopNotificationsFactory(QObject* parent)
    : QObject(parent)
    , m_uint(0)
{
    loadSettings();
}

void DesktopNotificationsFactory::loadSettings()
{
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Notifications");
    m_enabled = settings.value("Enabled", true).toBool();
    m_timeout = settings.value("Timeout", 6000).toInt();
#ifdef Q_WS_X11
    m_notifType = settings.value("UseNativeDesktop", true).toBool() ? DesktopNative : PopupWidget;
#else
    m_notifType = PopupWidget;
#endif
    m_position = settings.value("Position", QPoint(10, 10)).toPoint();
    settings.endGroup();
}

void DesktopNotificationsFactory::notify(const QPixmap &icon, const QString &heading, const QString &text)
{
    if (!m_enabled) {
        return;
    }

    switch (m_notifType) {
    case PopupWidget:
        if (!m_desktopNotif) {
            m_desktopNotif = new DesktopNotification();
        }
        m_desktopNotif->setPixmap(icon);
        m_desktopNotif->setHeading(heading);
        m_desktopNotif->setText(text);
        m_desktopNotif->setTimeout(m_timeout);
        m_desktopNotif->move(m_position);
        m_desktopNotif->show();
        break;
    case DesktopNative:
#ifdef Q_WS_X11
        QFile tmp(QDir::tempPath() + "/qupzilla_notif.png");
        tmp.open(QFile::WriteOnly);
        icon.save(tmp.fileName());

        QDBusInterface dbus("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", QDBusConnection::sessionBus());
        QVariantList args;
        args.append("qupzilla");
        args.append(m_uint);
        args.append(tmp.fileName());
        args.append(heading);
        args.append(text);
        args.append(QStringList());
        args.append(QVariantMap());
        args.append(m_timeout);
        QDBusMessage message = dbus.callWithArgumentList(QDBus::Block, "Notify", args);
        QVariantList list = message.arguments();
        if (list.count() > 0) {
            m_uint = list.at(0).toInt();
        }
#endif
        break;
    }
}

void DesktopNotificationsFactory::nativeNotificationPreview()
{
#ifdef Q_WS_X11
    QFile tmp(QDir::tempPath() + "/qupzilla_notif.png");
    tmp.open(QFile::WriteOnly);
    QPixmap(":icons/preferences/stock_dialog-question.png").save(tmp.fileName());

    QDBusInterface dbus("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", QDBusConnection::sessionBus());
    QVariantList args;
    args.append("qupzilla");
    args.append(m_uint);
    args.append(tmp.fileName());
    args.append(tr("Native System Notification"));
    args.append("");
    args.append(QStringList());
    args.append(QVariantMap());
    args.append(m_timeout);
    QDBusMessage message = dbus.callWithArgumentList(QDBus::Block, "Notify", args);
    QVariantList list = message.arguments();
    if (list.count() > 0) {
        m_uint = list.at(0).toInt();
    }
#endif
}
