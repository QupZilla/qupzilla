/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "networkmanagerproxy.h"
#include "networkmanager.h"
#include "webpage.h"
#include "qupzilla.h"
#include "cookiejar.h"
#include "mainapplication.h"

NetworkManagerProxy::NetworkManagerProxy(QupZilla* mainClass, QObject* parent) :
    QNetworkAccessManager(parent)
    ,p_QupZilla(mainClass)
    ,m_view(0)
    ,m_page(0)
{
    setCookieJar(mApp->cookieJar());
}

void NetworkManagerProxy::populateNetworkRequest(QNetworkRequest &request)
{
    qDebug() << __FUNCTION__ << "called";
    QVariant variant = qVariantFromValue((void *) m_page);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100), variant);
}

void NetworkManagerProxy::setPrimaryNetworkAccessManager(NetworkManager* manager)
{
    Q_ASSERT(manager);
    m_manager = manager;

    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)), manager, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)));
    connect(this, SIGNAL(finished(QNetworkReply*)), manager, SIGNAL(finished(QNetworkReply*)));
    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)));
}

QNetworkReply* NetworkManagerProxy::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
{
    if (m_manager && m_page) {
        QNetworkRequest pageRequest = request;
        m_page->populateNetworkRequest(pageRequest);
        return m_manager->createRequest(op, pageRequest, outgoingData);
    }
    return QNetworkAccessManager::createRequest(op, request, outgoingData);
}
