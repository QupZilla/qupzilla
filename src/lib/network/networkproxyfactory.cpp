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
#include "networkproxyfactory.h"
#include "mainapplication.h"
#include "settings.h"

WildcardMatcher::WildcardMatcher(const QString &pattern)
    : m_regExp(0)
{
    setPattern(pattern);
}

WildcardMatcher::~WildcardMatcher()
{
    delete m_regExp;
}

void WildcardMatcher::setPattern(const QString &pattern)
{
    m_pattern = pattern;

    if (m_pattern.contains(QLatin1Char('?')) || m_pattern.contains(QLatin1Char('*'))) {
        QString regexp = m_pattern;
        regexp.replace(QLatin1Char('.'), QLatin1String("\\."))
        .replace(QLatin1Char('*'), QLatin1String(".*"))
        .replace(QLatin1Char('?'), QLatin1Char('.'));
        regexp = QString("^.*%1.*$").arg(regexp);

        m_regExp = new QzRegExp(regexp, Qt::CaseInsensitive);
    }
}

QString WildcardMatcher::pattern() const
{
    return m_pattern;
}

bool WildcardMatcher::match(const QString &str) const
{
    if (!m_regExp) {
        return str.contains(m_pattern, Qt::CaseInsensitive);
    }

    return m_regExp->indexIn(str) > -1;
}

NetworkProxyFactory::NetworkProxyFactory()
    : QNetworkProxyFactory()
    , m_proxyPreference(SystemProxy)
    , m_proxyType(QNetworkProxy::HttpProxy)
    , m_port(0)
    , m_httpsPort(0)
    , m_useDifferentProxyForHttps(false)
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

    QStringList exceptions = settings.value("ProxyExceptions", QStringList() << "localhost" << "127.0.0.1").toStringList();
    settings.endGroup();

    qDeleteAll(m_proxyExceptions);
    m_proxyExceptions.clear();

    foreach (const QString &exception, exceptions) {
        m_proxyExceptions.append(new WildcardMatcher(exception.trimmed()));
    }
}

NetworkProxyFactory::ProxyPreference NetworkProxyFactory::proxyPreference() const
{
    return m_proxyPreference;
}

QList<QNetworkProxy> NetworkProxyFactory::queryProxy(const QNetworkProxyQuery &query)
{
    QList<QNetworkProxy> proxyList;

    if (m_proxyPreference == NoProxy) {
        proxyList.append(QNetworkProxy::NoProxy);
        return proxyList;
    }

    const QString urlHost = query.url().host();
    foreach (WildcardMatcher* m, m_proxyExceptions) {
        if (m->match(urlHost)) {
            proxyList.append(QNetworkProxy::NoProxy);
            return proxyList;
        }
    }

    switch (m_proxyPreference) {
    case SystemProxy:
        proxyList.append(systemProxyForQuery(query));
        break;

    case ProxyAutoConfig:
        qWarning() << "PAC Not Implemented!";
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

NetworkProxyFactory::~NetworkProxyFactory()
{
    qDeleteAll(m_proxyExceptions);
}
