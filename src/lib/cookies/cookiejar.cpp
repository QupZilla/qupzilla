/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "cookiejar.h"
#include "mainapplication.h"
#include "datapaths.h"
#include "autosaver.h"
#include "settings.h"
#include "qztools.h"

#include <QNetworkCookie>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QDateTime>

//#define COOKIE_DEBUG

CookieJar::CookieJar(QObject* parent)
    : QObject(parent)
    , m_client(mApp->webProfile()->cookieStore())
{
    loadSettings();

    connect(m_client, &QWebEngineCookieStore::cookieAdded, this, &CookieJar::cookieAdded);
    connect(m_client, &QWebEngineCookieStore::cookieRemoved, this, &CookieJar::cookieRemoved);

    m_client->setCookieFilter([this](QWebEngineCookieStore::FilterRequest &req) {
        req.accepted = acceptCookie(req.firstPartyUrl, req.cookieLine, req.cookieSource);
    });
}

void CookieJar::loadSettings()
{
    Settings settings;
    settings.beginGroup("Cookie-Settings");
    m_allowCookies = settings.value("allowCookies", true).toBool();
    m_filterThirdParty = settings.value("filterThirdPartyCookies", false).toBool();
    m_filterTrackingCookie = settings.value("filterTrackingCookie", false).toBool();
    m_whitelist = settings.value("whitelist", QStringList()).toStringList();
    m_blacklist = settings.value("blacklist", QStringList()).toStringList();
    settings.endGroup();
}

void CookieJar::setAllowCookies(bool allow)
{
    m_allowCookies = allow;
}

void CookieJar::getAllCookies(const QWebEngineCallback<const QByteArray &> callback)
{
    m_client->getAllCookies(callback);
}

void CookieJar::deleteAllCookies()
{
    m_client->deleteAllCookies();
}

bool CookieJar::matchDomain(QString cookieDomain, QString siteDomain) const
{
    // According to RFC 6265

    // Remove leading dot
    if (cookieDomain.startsWith(QLatin1Char('.'))) {
        cookieDomain = cookieDomain.mid(1);
    }

    if (siteDomain.startsWith(QLatin1Char('.'))) {
        siteDomain = siteDomain.mid(1);
    }

    return QzTools::matchDomain(cookieDomain, siteDomain);
}

bool CookieJar::listMatchesDomain(const QStringList &list, const QString &cookieDomain) const
{
    foreach (const QString &d, list) {
        if (matchDomain(d, cookieDomain)) {
            return true;
        }
    }

    return false;
}

bool CookieJar::acceptCookie(const QUrl &firstPartyUrl, const QByteArray &cookieLine, const QUrl &cookieSource) const
{
    const QList<QNetworkCookie> cookies = QNetworkCookie::parseCookies(cookieLine);
    Q_ASSERT(cookies.size() == 1);

    const QNetworkCookie cookie = cookies.at(0);
    return !rejectCookie(firstPartyUrl.host(), cookie, cookieSource.host());
}

bool CookieJar::rejectCookie(const QString &domain, const QNetworkCookie &cookie, const QString &cookieDomain) const
{
    if (!m_allowCookies) {
        bool result = listMatchesDomain(m_whitelist, cookieDomain);
        if (!result) {
#ifdef COOKIE_DEBUG
            qDebug() << "not in whitelist" << cookie;
#endif
            return true;
        }
    }

    if (m_allowCookies) {
        bool result = listMatchesDomain(m_blacklist, cookieDomain);
        if (result) {
#ifdef COOKIE_DEBUG
            qDebug() << "found in blacklist" << cookie;
#endif
            return true;
        }
    }

    if (m_filterThirdParty) {
        bool result = matchDomain(cookieDomain, domain);
        if (!result) {
#ifdef COOKIE_DEBUG
            qDebug() << "purged for domain mismatch" << cookie << cookieDomain << domain;
#endif
            return true;
        }
    }

    if (m_filterTrackingCookie && cookie.name().startsWith("__utm")) {
#ifdef COOKIE_DEBUG
        qDebug() << "purged as tracking " << cookie;
#endif
        return true;
    }

    return false;
}
