#include "desktopnotificationsfactory.h"

DesktopNotificationsFactory::DesktopNotificationsFactory(QObject *parent) :
    QObject(parent)
{
    QDBusInterface dbus("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", QDBusConnection::sessionBus());
    QVariantList args;
    args.append("qupzilla");
    args.append(QVariant::UInt);
    args.append("/home/david/a.png");
    args.append("Summary");
    args.append("Body of notification");
    args.append(QStringList());
    args.append(QVariantMap());
    args.append(-1);
    dbus.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
}
