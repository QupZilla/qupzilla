#ifndef USERAGENTMANAGER_H
#define USERAGENTMANAGER_H

#include <QHash>

#include "qz_namespace.h"

class QUrl;

class QT_QUPZILLA_EXPORT UserAgentManager
{
public:
    explicit UserAgentManager();

    void loadSettings();

    QString userAgentForUrl(const QUrl &url);

private:
    QString m_globalUserAgent;
    QString m_fakeUserAgent;

    bool m_usePerDomainUserAgent;
    QHash<QString, QString> m_userAgentsList;

};

#endif // USERAGENTMANAGER_H
