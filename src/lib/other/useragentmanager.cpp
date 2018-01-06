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
#include "useragentmanager.h"
#include "browserwindow.h"
#include "qztools.h"
#include "settings.h"

#include <QWebEngineProfile>
#include <QRegularExpression>

UserAgentManager::UserAgentManager(QObject* parent)
    : QObject(parent)
    , m_usePerDomainUserAgent(false)
{
    m_defaultUserAgent = QWebEngineProfile::defaultProfile()->httpUserAgent();
    m_defaultUserAgent.replace(QRegularExpression(QSL("QtWebEngine/[^\\s]+")), QSL("QupZilla/%1").arg(Qz::VERSION));
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

    const QString userAgent = m_globalUserAgent.isEmpty() ? m_defaultUserAgent : m_globalUserAgent;
    QWebEngineProfile::defaultProfile()->setHttpUserAgent(userAgent);
}

QString UserAgentManager::globalUserAgent() const
{
    return m_globalUserAgent;
}

QString UserAgentManager::defaultUserAgent() const
{
    return m_defaultUserAgent;
}

bool UserAgentManager::usePerDomainUserAgents() const
{
    return m_usePerDomainUserAgent;
}

QHash<QString, QString> UserAgentManager::perDomainUserAgentsList() const
{
    return m_userAgentsList;
}
