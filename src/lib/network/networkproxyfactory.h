/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#include <QStringList>

#include "qzcommon.h"
#include "qzregexp.h"

class WildcardMatcher
{
public:
    explicit WildcardMatcher(const QString &pattern = QString());
    ~WildcardMatcher();

    void setPattern(const QString &pattern);
    QString pattern() const;

    bool match(const QString &str) const;

private:
    QString m_pattern;
    QzRegExp* m_regExp;
};

class QUPZILLA_EXPORT NetworkProxyFactory : public QNetworkProxyFactory
{
public:
    enum ProxyPreference { SystemProxy, NoProxy, ProxyAutoConfig, DefinedProxy };

    explicit NetworkProxyFactory();
    ~NetworkProxyFactory();

    void loadSettings();

    ProxyPreference proxyPreference() const;

    QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query = QNetworkProxyQuery());

private:
    ProxyPreference m_proxyPreference;
    QNetworkProxy::ProxyType m_proxyType;

    QString m_hostName;
    quint16 m_port;
    QString m_username;
    QString m_password;

    QString m_httpsHostName;
    quint16 m_httpsPort;
    QString m_httpsUsername;
    QString m_httpsPassword;

    QList<WildcardMatcher*> m_proxyExceptions;
    bool m_useDifferentProxyForHttps;
};

#endif // NETWORKPROXYFACTORY_H
