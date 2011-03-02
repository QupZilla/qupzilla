#include "networkmanagerproxy.h"
#include "networkmanager.h"
#include "webpage.h"
#include "qupzilla.h"
#include "cookiejar.h"
#include "mainapplication.h"

NetworkManagerProxy::NetworkManagerProxy(QupZilla* mainClass, QObject *parent) :
    QNetworkAccessManager(parent)
    ,p_QupZilla(mainClass)
    ,m_view(0)
    ,m_page(0)
{
    setCookieJar(MainApplication::getInstance()->cookieJar());
}

void NetworkManagerProxy::populateNetworkRequest(QNetworkRequest &request)
{
    qDebug() << __FUNCTION__ << "called";
    QVariant variant = qVariantFromValue((void *) m_page);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100), variant);
}

void NetworkManagerProxy::setPrimaryNetworkAccessManager(NetworkManagerProxy *manager)
{
    Q_ASSERT(manager);
    m_manager = manager;

    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
            manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)));
    connect(this, SIGNAL(finished(QNetworkReply *)),
            manager, SIGNAL(finished(QNetworkReply *)));

    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
            manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)));
}

QNetworkReply *NetworkManagerProxy::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    if (m_manager && m_page) {
        QNetworkRequest pageRequest = request;
        m_page->populateNetworkRequest(pageRequest);
        return m_manager->createRequest(op, pageRequest, outgoingData);
    }
    return QNetworkAccessManager::createRequest(op, request, outgoingData);
}
