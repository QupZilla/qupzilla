/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "networkproxyfactory.h"
#include "mainapplication.h"
#include "settings.h"
#include "pac/pacmanager.h"

NetworkProxyFactory::NetworkProxyFactory()
    : QNetworkProxyFactory()
    , m_pacManager(new PacManager)
    , m_proxyPreference(SystemProxy)
{
}

void NetworkProxyFactory::loadSettings()
{
    Settings settings;
    settings.beginGroup("Web-Proxy");
    m_proxyPreference = ProxyPreference(settings.value("UseProxy", SystemProxy).toInt());
    m_proxyType = QNetworkProxy::ProxyType(settings.value("ProxyType", QNetworkProxy::HttpProxy).toInt());
    m_useDifferentProxyForHttps = settings.value("UseDifferentProxyForHttps", false).toBool();

    m_hostName = settings.value("HostName", QString()).toString();
    m_port = settings.value("Port", 8080).toInt();
    m_username = settings.value("Username", QString()).toString();
    m_password = settings.value("Password", QString()).toString();

    m_httpsHostName = settings.value("HttpsHostName", QString()).toString();
    m_httpsPort = settings.value("HttpsPort", 8080).toInt();
    m_httpsUsername = settings.value("HttpsUsername", QString()).toString();
    m_httpsPassword = settings.value("HttpsPassword", QString()).toString();

    m_proxyExceptions = settings.value("ProxyExceptions", QStringList() << "localhost" << "127.0.0.1").toStringList();
    settings.endGroup();

    m_pacManager->loadSettings();
}

PacManager* NetworkProxyFactory::pacManager() const
{
    return m_pacManager;
}

QList<QNetworkProxy> NetworkProxyFactory::queryProxy(const QNetworkProxyQuery &query)
{
    QList<QNetworkProxy> proxyList;

    if (m_proxyExceptions.contains(query.url().host(), Qt::CaseInsensitive)) {
        proxyList.append(QNetworkProxy::NoProxy);
        return proxyList;
    }

    switch (m_proxyPreference) {
    case SystemProxy:
        proxyList.append(systemProxyForQuery(query));
        break;

    case NoProxy:
        proxyList.append(QNetworkProxy::NoProxy);
        break;

    case ProxyAutoConfig:
        proxyList.append(m_pacManager->queryProxy(query.url()));
        break;

    case DefinedProxy: {
        QNetworkProxy proxy(m_proxyType);

        if (m_useDifferentProxyForHttps && query.protocolTag() == QLatin1String("https")) {
            proxy.setHostName(m_httpsHostName);
            proxy.setPort(m_httpsPort);
            proxy.setUser(m_httpsUsername);
            proxy.setPassword(m_httpsPassword);
        }
        else {
            proxy.setHostName(m_hostName);
            proxy.setPort(m_port);
            proxy.setUser(m_username);
            proxy.setPassword(m_password);
        }

        if (proxy.hostName().isEmpty()) {
            proxy = QNetworkProxy::NoProxy;
        }

        proxyList.append(proxy);
        break;
    }

    default:
        qWarning("NetworkProxyFactory::queryProxy Unknown proxy type!");
        break;
    }

    if (!proxyList.contains(QNetworkProxy::NoProxy)) {
        proxyList.append(QNetworkProxy::NoProxy);
    }

    return proxyList;
}
