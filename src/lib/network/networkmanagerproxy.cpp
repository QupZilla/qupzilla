/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "cookiejar.h"
#include "mainapplication.h"

#include <QNetworkRequest>

#if QTWEBENGINE_DISABLED

NetworkManagerProxy::NetworkManagerProxy(QObject* parent)
    : QNetworkAccessManager(parent)
    , m_page(0)
    , m_manager(0)
{
    setCookieJar(mApp->cookieJar());

    // CookieJar is shared between NetworkManagers
    mApp->cookieJar()->setParent(0);
}

void NetworkManagerProxy::setPrimaryNetworkAccessManager(NetworkManager* manager)
{
    Q_ASSERT(manager);
    m_manager = manager;

    connect(this, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), m_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)), m_manager, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)));
    connect(this, SIGNAL(finished(QNetworkReply*)), m_manager, SIGNAL(finished(QNetworkReply*)));
    connect(this, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), m_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)));
}

QNetworkReply* NetworkManagerProxy::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
{
    if (m_manager) {
        QNetworkRequest pageRequest = request;
        if (m_page) {
            m_page->populateNetworkRequest(pageRequest);
        }
        return m_manager->createRequest(op, pageRequest, outgoingData);
    }

    return QNetworkAccessManager::createRequest(op, request, outgoingData);
}

#endif
