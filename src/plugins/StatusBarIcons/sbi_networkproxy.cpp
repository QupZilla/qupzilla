/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
    , m_type(QNetworkProxy::NoProxy)
{
}

bool SBI_NetworkProxy::operator ==(const SBI_NetworkProxy &other) const
{
    return m_port == other.m_port && m_hostname == other.m_hostname &&
           m_username == other.m_username && m_password == other.m_password &&
           m_type == other.m_type;
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

QNetworkProxy::ProxyType SBI_NetworkProxy::type() const
{
    return m_type;
}

void SBI_NetworkProxy::setType(QNetworkProxy::ProxyType type)
{
    m_type = type;
}

void SBI_NetworkProxy::loadFromSettings(const QSettings &settings)
{
    m_hostname = settings.value("HostName", QString()).toString();
    m_port = settings.value("Port", 0).toInt();
    m_username = settings.value("Username", QString()).toString();
    m_password = settings.value("Password", QString()).toString();

    m_type = QNetworkProxy::ProxyType(settings.value("ProxyType", QNetworkProxy::HttpProxy).toInt());
}

void SBI_NetworkProxy::saveToSettings(QSettings &settings) const
{
    settings.setValue("HostName", m_hostname);
    settings.setValue("Port", m_port);
    settings.setValue("Username", m_username);
    settings.setValue("Password", m_password);

    settings.setValue("ProxyType", m_type);
}

void SBI_NetworkProxy::applyProxy()
{
    QNetworkProxy proxy;
    proxy.setHostName(m_hostname);
    proxy.setPort(m_port);
    proxy.setUser(m_username);
    proxy.setPassword(m_password);
    proxy.setType(m_type);

    QNetworkProxy::setApplicationProxy(proxy);
}
