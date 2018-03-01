/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
    m_client->loadAllCookies();

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    m_client->setCookieFilter(std::bind(&CookieJar::cookieFilter, this, std::placeholders::_1));
#endif

    connect(m_client, &QWebEngineCookieStore::cookieAdded, this, &CookieJar::slotCookieAdded);
    connect(m_client, &QWebEngineCookieStore::cookieRemoved, this, &CookieJar::slotCookieRemoved);
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

void CookieJar::deleteCookie(const QNetworkCookie &cookie)
{
    m_client->deleteCookie(cookie);
}

QVector<QNetworkCookie> CookieJar::getAllCookies() const
{
    return m_cookies;
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

void CookieJar::slotCookieAdded(const QNetworkCookie &cookie)
{
    if (rejectCookie(QString(), cookie, cookie.domain())) {
        m_client->deleteCookie(cookie);
        return;
    }

    m_cookies.append(cookie);
    emit cookieAdded(cookie);
}

void CookieJar::slotCookieRemoved(const QNetworkCookie &cookie)
{
    if (m_cookies.removeOne(cookie))
        emit cookieRemoved(cookie);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
void CookieJar::cookieFilter(QWebEngineCookieStore::FilterRequest &request) const
{
    if (!m_allowCookies) {
        bool result = listMatchesDomain(m_whitelist, request.origin.host());
        if (!result) {
#ifdef COOKIE_DEBUG
            qDebug() << "not in whitelist" << request.origin;
#endif
            request.accepted = false;
            return;
        }
    }

    if (m_allowCookies) {
        bool result = listMatchesDomain(m_blacklist, request.origin.host());
        if (result) {
#ifdef COOKIE_DEBUG
            qDebug() << "found in blacklist" << request.origin.host();
#endif
            request.accepted = false;
            return;
        }
    }

    if (m_filterThirdParty && request.thirdParty) {
#ifdef COOKIE_DEBUG
        qDebug() << "thirdParty" << request.firstPartyUrl << request.origin;
#endif
        request.accepted = false;
        return;
    }
}
#endif

bool CookieJar::rejectCookie(const QString &domain, const QNetworkCookie &cookie, const QString &cookieDomain) const
{
    Q_UNUSED(domain)

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

#if QTWEBENGINE_DISABLED
    if (m_filterThirdParty) {
        bool result = matchDomain(cookieDomain, domain);
        if (!result) {
#ifdef COOKIE_DEBUG
            qDebug() << "purged for domain mismatch" << cookie << cookieDomain << domain;
#endif
            return true;
        }
    }
#endif

    if (m_filterTrackingCookie && cookie.name().startsWith("__utm")) {
#ifdef COOKIE_DEBUG
        qDebug() << "purged as tracking " << cookie;
#endif
        return true;
    }

    return false;
}
