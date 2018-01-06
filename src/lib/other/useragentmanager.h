/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#ifndef USERAGENTMANAGER_H
#define USERAGENTMANAGER_H

#include <QObject>
#include <QHash>

#include "qzcommon.h"

class QUrl;

class QUPZILLA_EXPORT UserAgentManager : QObject
{
    Q_OBJECT

public:
    explicit UserAgentManager(QObject* parent = 0);

    void loadSettings();

    QString globalUserAgent() const;
    QString defaultUserAgent() const;

    bool usePerDomainUserAgents() const;
    QHash<QString, QString> perDomainUserAgentsList() const;

private:
    QString m_globalUserAgent;
    QString m_defaultUserAgent;

    bool m_usePerDomainUserAgent;
    QHash<QString, QString> m_userAgentsList;
};

#endif // USERAGENTMANAGER_H
