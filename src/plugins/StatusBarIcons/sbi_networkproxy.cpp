/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#include "sbi_networkproxy.h"

#include <QSettings>

SBI_NetworkProxy::SBI_NetworkProxy()
    : m_port(0)
    , m_httpsPort(0)
    , m_useDifferentProxyForHttps(false)
{
}

quint16 SBI_NetworkProxy::port() const
{
    return m_port;
}

void SBI_NetworkProxy::setPort(quint16 port)
{
    m_port = port;
}

QString SBI_NetworkProxy::hostName() const
{
    return m_hostname;
}

void SBI_NetworkProxy::setHostName(const QString &hostName)
{
    m_hostname = hostName;
}

QString SBI_NetworkProxy::userName() const
{
    return m_username;
}

void SBI_NetworkProxy::setUserName(const QString &userName)
{
    m_username = userName;
}

QString SBI_NetworkProxy::password() const
{
    return m_password;
}

void SBI_NetworkProxy::setPassword(const QString &password)
{
    m_password = password;
}

bool SBI_NetworkProxy::useDifferentProxyForHttps() const
{
    return m_useDifferentProxyForHttps;
}

quint16 SBI_NetworkProxy::httpsPort() const
{
    return m_httpsPort;
}

void SBI_NetworkProxy::setHttpsPort(quint16 port)
{
    m_httpsPort = port;
}

QString SBI_NetworkProxy::httpsHostName() const
{
    return m_httpsHostname;
}

void SBI_NetworkProxy::setHttpsHostName(const QString &hostName)
{
    m_httpsHostname = hostName;
}

QString SBI_NetworkProxy::httpsUserName() const
{
    return m_httpsUsername;
}

void SBI_NetworkProxy::setHttpsUserName(const QString &userName)
{
    m_httpsUsername = userName;
}

QString SBI_NetworkProxy::httpsPassword() const
{
    return m_httpsPassword;
}

void SBI_NetworkProxy::setHttpsPassword(const QString &password)
{
    m_httpsPassword = password;
}

void SBI_NetworkProxy::loadFromSettings(QSettings* settings)
{
    m_hostname = settings->value("HostName", QString()).toString();
    m_port = settings->value("Port", 0).toInt();
    m_username = settings->value("Username", QString()).toString();
    m_password = settings->value("Password", QString()).toString();

    m_httpsHostname = settings->value("HttpsHostName", QString()).toString();
    m_httpsPort = settings->value("HttpsPort", 0).toInt();
    m_httpsUsername = settings->value("HttpsUsername", QString()).toString();
    m_httpsPassword = settings->value("HttpsPassword", QString()).toString();

    m_useDifferentProxyForHttps = settings->value("UseDifferentProxyForHttps", false).toBool();
}

void SBI_NetworkProxy::saveToSettings(QSettings* settings)
{
    settings->setValue("HostName", m_hostname);
    settings->setValue("Port", m_port);
    settings->setValue("Username", m_username);
    settings->setValue("Password", m_password);

    settings->setValue("HttpsHostName", m_httpsHostname);
    settings->setValue("HttpsPort", m_httpsPort);
    settings->setValue("HttpsUsername", m_httpsUsername);
    settings->setValue("HttpsPassword", m_httpsPassword);

    settings->setValue("UseDifferentProxyForHttps", m_useDifferentProxyForHttps);
}
