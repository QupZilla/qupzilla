/* ============================================================
* QupZilla - WebKit based browser
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
#include "pacmanager.h"
#include "proxyautoconfig.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "followredirectreply.h"
#include "settings.h"
#include "datapaths.h"
#include "qztools.h"

#include <QNetworkProxy>
#include <QStringList>
#include <QUrl>
#include <QFile>

PacManager::PacManager(QObject* parent)
    : QObject(parent)
    , m_pacrunner(0)
    , m_reply(0)
    , m_loaded(false)
{
}

void PacManager::loadSettings()
{
    QUrl oldUrl = m_url;

    Settings settings;
    settings.beginGroup("Web-Proxy");
    m_url = settings.value("PacUrl", QUrl()).toUrl();
    settings.endGroup();

    if (m_loaded && oldUrl != m_url) {
        downloadPacFile();
    }

    m_loaded = true;
}

void PacManager::downloadPacFile()
{
    if (m_reply) {
        qWarning() << "PacManager: PAC file is already being downloaded!";
        return;
    }

    if (m_url.scheme() == QLatin1String("file")) {
        if (!QFile(m_url.path()).exists()) {
            qWarning() << "PacManager: PAC file " << m_url.path() << "doesn't exists!";
        }
        else {
            reloadScript();
        }
        return;
    }

    m_reply = new FollowRedirectReply(m_url, mApp->networkManager());
    connect(m_reply, SIGNAL(finished()), this, SLOT(replyFinished()));
}

QList<QNetworkProxy> PacManager::queryProxy(const QUrl &url)
{
    if (!m_pacrunner) {
        reloadScript();
    }

    QString proxyString = m_pacrunner->findProxyForUrl(url.toEncoded(), url.host());
    return parseProxies(proxyString.trimmed());
}

void PacManager::replyFinished()
{
    if (m_reply->error() != QNetworkReply::NoError) {
        qWarning() << "PacManager: Cannot download PAC file from" << m_url;
        m_reply->deleteLater();
        m_reply = 0;
        return;
    }

    QByteArray data = m_reply->readAll();
    m_reply->deleteLater();
    m_reply = 0;

    QFile file(DataPaths::currentProfilePath() + "/proxy.pac");

    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "PacManager: Cannot open PAC file for writing" << file.fileName();
        return;
    }

    file.write(data);
    file.close();

    reloadScript();
}

void PacManager::reloadScript()
{
    if (!m_pacrunner) {
        m_pacrunner = new ProxyAutoConfig(this);
    }

    QFile file(m_url.scheme() == QLatin1String("file") ? m_url.path() : DataPaths::currentProfilePath() + "/proxy.pac");

    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "PacManager: Cannot open PAC file for reading" << file.fileName();
        return;
    }

    m_pacrunner->setConfig(file.readAll());
}

QList<QNetworkProxy> PacManager::parseProxies(const QString &string)
{
    QString str = string.trimmed();
    QList<QNetworkProxy> proxies;

    if (str.isEmpty()) {
        return proxies;
    }

    QStringList parts = str.split(QLatin1Char(';'), QString::SkipEmptyParts);
    if (parts.isEmpty()) {
        parts.append(str);
    }

    foreach (const QString &s, parts) {
        QStringList l = s.split(QLatin1Char(' '), QString::SkipEmptyParts);

        if (l.count() != 2) {
            if (l.count() == 1 && l.at(0) == QLatin1String("DIRECT")) {
                proxies.append(QNetworkProxy::NoProxy);
            }
            continue;
        }

        QString type = l.at(0);
        QUrl url = QUrl::fromEncoded("proxy://" + l.at(1).toUtf8());

        if (type == QLatin1String("PROXY")) {
            QNetworkProxy proxy(QNetworkProxy::HttpProxy, url.host(), url.port(8080));
            proxies.append(proxy);
        }
        else if (type == QLatin1String("SOCKS")) {
            QNetworkProxy proxy(QNetworkProxy::Socks5Proxy, url.host(), url.port(1080));
            proxies.append(proxy);
        }
    }

    return proxies;
}
