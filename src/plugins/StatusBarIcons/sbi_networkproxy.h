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
#ifndef SBI_NETWORKPROXY_H
#define SBI_NETWORKPROXY_H

#include <QUrl>

#include "networkproxyfactory.h"

class QSettings;

class SBI_NetworkProxy
{
public:
    explicit SBI_NetworkProxy();

    bool operator==(const SBI_NetworkProxy &other) const;

    quint16 port() const;
    void setPort(quint16 port);

    QString hostName() const;
    void setHostName(const QString &hostName);

    QString userName() const;
    void setUserName(const QString &userName);

    QString password() const;
    void setPassword(const QString &password);

    QNetworkProxy::ProxyType type() const;
    void setType(QNetworkProxy::ProxyType type);

    void loadFromSettings(const QSettings &settings);
    void saveToSettings(QSettings &settings) const;

    void applyProxy();

private:
    quint16 m_port;
    QString m_hostname;
    QString m_username;
    QString m_password;

    QNetworkProxy::ProxyType m_type;
};

#endif // SBI_NETWORKPROXY_H
