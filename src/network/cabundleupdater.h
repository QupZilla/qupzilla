#ifndef CABUNDLEUPDATER_H
#define CABUNDLEUPDATER_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QDateTime>
#include <QNetworkReply>

class NetworkManager;
class CaBundleUpdater : public QObject
{
    Q_OBJECT
public:
    explicit CaBundleUpdater(NetworkManager* manager, QObject* parent = 0);

signals:

public slots:

private slots:
    void start();
    void replyFinished();

private:
    enum Progress { Start, CheckLastUpdate, LoadBundle };

    NetworkManager* m_manager;
    Progress m_progress;
    QNetworkReply* m_reply;

    QString m_bundleVersionFileName;
    QString m_bundleFileName;
    QString m_lastUpdateFileName;

    int m_latestBundleVersion;
};

#endif // CABUNDLEUPDATER_H
