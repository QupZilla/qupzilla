#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QNetworkAccessManager>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QUrl>
#include <QFormLayout>
#include <QCheckBox>
#include <QSslError>
#include <QNetworkDiskCache>

#include "networkmanagerproxy.h"

class QupZilla;
class NetworkManager : public NetworkManagerProxy
{
    Q_OBJECT
public:
    explicit NetworkManager(QupZilla* mainClass, QObject *parent = 0);
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData);

    void saveCertExceptions();
    void loadCertExceptions();
    void loadSettings();

signals:
    void finishLoading(bool state);
    void wantsFocus(const QUrl &url);
    void sslDialogClosed();

public slots:
    void authentication(QNetworkReply* reply, QAuthenticator* auth);
    void sslError(QNetworkReply* reply, QList<QSslError> errors);

private:
    QupZilla* p_QupZilla;
    QList<QSslCertificate> m_certExceptions;
    QNetworkDiskCache* m_diskCache;

};

#endif // NETWORKMANAGER_H
