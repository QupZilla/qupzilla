#ifndef DESKTOPNOTIFICATIONSFACTORY_H
#define DESKTOPNOTIFICATIONSFACTORY_H

#include <QObject>
#include <QDBusInterface>
#include <QStringList>

class DesktopNotificationsFactory : public QObject
{
    Q_OBJECT
public:
    explicit DesktopNotificationsFactory(QObject *parent = 0);

signals:

public slots:

};

#endif // DESKTOPNOTIFICATIONSFACTORY_H
