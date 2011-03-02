#ifndef UPDATER_H
#define UPDATER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QNetworkReply>

class QupZilla;
class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(QupZilla* mainClass, QObject *parent = 0);
    ~Updater();

signals:

public slots:
    void downCompleted(QNetworkReply* reply);
    void start();
    void goUpdate();
    void clicked(QSystemTrayIcon::ActivationReason reason);

private:
    void createTrayIcon();
    void startDownloadingUpdateInfo(const QUrl &url);

    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayIconMenu;

    QupZilla* p_QupZilla;

};

#endif // UPDATER_H
