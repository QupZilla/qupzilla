/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#ifndef NETWORKPROXYFACTORY_H
#define NETWORKPROXYFACTORY_H

#include <QNetworkProxyFactory>
#include <QUrl>
#include <QStringList>
#include <QSettings>

class NetworkProxyFactory : public QNetworkProxyFactory
{
public:
    enum ProxyPreference { SystemProxy, NoProxy, DefinedProxy };

    explicit NetworkProxyFactory();
    void loadSettings();

    QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query = QNetworkProxyQuery());

private:
    ProxyPreference m_proxyPreference;
    QNetworkProxy::ProxyType m_proxyType;
    QString m_hostName;
    quint16 m_port;
    QString m_username;
    QString m_password;
    QStringList m_proxyExceptions;
};

#endif // NETWORKPROXYFACTORY_H
