#include "useragentmanager.h"
#include "qupzilla.h"
#include "globalfunctions.h"
#include "settings.h"

UserAgentManager::UserAgentManager()
{
    m_fakeUserAgent = QString("Mozilla/5.0 (%1) AppleWebKit/%2 (KHTML, like Gecko) Chrome/10.0 Safari/%2").arg(qz_buildSystem(), QupZilla::WEBKITVERSION);
}

void UserAgentManager::loadSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    m_globalUserAgent = settings.value("UserAgent", QString()).toString();
    settings.endGroup();

    settings.beginGroup("UserAgent-Settings");
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

QString UserAgentManager::userAgentForUrl(const QUrl &url)
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

    if (host.contains("google")) {
        return m_fakeUserAgent;
    }

    return m_globalUserAgent;
}
