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
#include "useragentmanager.h"
#include "qupzilla.h"
#include "qztools.h"
#include "settings.h"

#include <QWebPage> // QTWEBKIT_VERSION_CHECK macro

UserAgentManager::UserAgentManager()
{
    m_fakeUserAgent = QString("Mozilla/5.0 (%1) AppleWebKit/%2 (KHTML, like Gecko) Chrome/10.0 Safari/%2").arg(QzTools::operatingSystem(), QupZilla::WEBKITVERSION);
}

void UserAgentManager::loadSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    m_globalUserAgent = settings.value("UserAgent", QString()).toString();
    settings.endGroup();

    settings.beginGroup("User-Agent-Settings");
    m_usePerDomainUserAgent = settings.value("UsePerDomainUA", false).toBool();
    QStringList domainList = settings.value("DomainList", QStringList()).toStringList();
    QStringList userAgentsList = settings.value("UserAgentsList", QStringList()).toStringList();
    settings.endGroup();

    m_usePerDomainUserAgent = (m_usePerDomainUserAgent && domainList.count() == userAgentsList.count());

    if (m_usePerDomainUserAgent) {
        for (int i = 0; i < domainList.count(); ++i) {
            m_userAgentsList[domainList.at(i)] = userAgentsList.at(i);
        }
    }
}

QString UserAgentManager::userAgentForUrl(const QUrl &url) const
{
    const QString &host = url.host();

    if (m_usePerDomainUserAgent) {
        QHashIterator<QString, QString> i(m_userAgentsList);
        while (i.hasNext()) {
            i.next();
            if (host.endsWith(i.key())) {
                return i.value();
            }
        }
    }

#if QTWEBKIT_TO_2_3
    if (host.contains(QLatin1String("google"))) {
        return m_fakeUserAgent;
    }
#endif

    return m_globalUserAgent;
}

QString UserAgentManager::globalUserAgent() const
{
    return m_globalUserAgent;
}

bool UserAgentManager::usePerDomainUserAgents() const
{
    return m_usePerDomainUserAgent;
}

QHash<QString, QString> UserAgentManager::perDomainUserAgentsList() const
{
    return m_userAgentsList;
}
