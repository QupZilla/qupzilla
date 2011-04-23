#ifndef DESKTOPNOTIFICATIONSFACTORY_H
#define DESKTOPNOTIFICATIONSFACTORY_H

#include <QObject>
#include <QDBusInterface>
#include <QStringList>
#include <QSettings>
#include <QPoint>
#include <QTimer>
#include <QPointer>

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
