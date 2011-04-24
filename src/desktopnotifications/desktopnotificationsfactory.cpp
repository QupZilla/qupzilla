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
    QSettings settings(mApp->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Notifications");
    m_enabled = settings.value("Enabled", true).toBool();
    m_timeout = settings.value("Timeout", 6000).toInt();
//#ifdef Q_WS_X11
#if 0
    m_notifType = settings.value("UseNativeDesktop", true).toBool() ? DesktopNative : PopupWidget;
#else
    m_notifType = PopupWidget;
#endif
    m_position = settings.value("Position", QPoint(10, 10)).toPoint();
    settings.endGroup();
}

void DesktopNotificationsFactory::notify(const QPixmap &icon, const QString &heading, const QString &text)
{
    if (!m_enabled)
        return;

    switch (m_notifType) {
    case PopupWidget:
        if (!m_desktopNotif)
            m_desktopNotif = new DesktopNotification();
        m_desktopNotif->setPixmap(icon);
        m_desktopNotif->setHeading(heading);
        m_desktopNotif->setText(text);
        m_desktopNotif->setTimeout(m_timeout);
        m_desktopNotif->move(m_position);
        m_desktopNotif->show();
        break;

    case DesktopNative:
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
        if (list.count() > 0)
            m_uint = list.at(0).toInt();
        break;
    }
}
