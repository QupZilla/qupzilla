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
#include "cookiejar.h"
#include "qupzilla.h"
#include "mainapplication.h"
#include "settings.h"

#include <QNetworkCookie>
#include <QWebSettings>
#include <QDateTime>
#include <QDebug>
#include <QWebPage> // QTWEBKIT_VERSION_CHECK macro

//#define COOKIE_DEBUG

CookieJar::CookieJar(QupZilla* mainClass, QObject* parent)
    : QNetworkCookieJar(parent)
    , p_QupZilla(mainClass)
{
    m_activeProfil = mApp->currentProfilePath();
    loadSettings();
}

void CookieJar::loadSettings()
{
    Settings settings;
    settings.beginGroup("Cookie-Settings");
    m_allowCookies = settings.value("allowCookies", true).toBool();
    m_blockThirdParty = settings.value("allowCookiesFromVisitedDomainOnly", false).toBool();
    m_filterTrackingCookie = settings.value("filterTrackingCookie", false).toBool();
    m_deleteOnClose = settings.value("deleteCookiesOnClose", false).toBool();
    m_whitelist = settings.value("whitelist", QStringList()).toStringList();
    m_blacklist = settings.value("blacklist", QStringList()).toStringList();
    settings.endGroup();

#if QTWEBKIT_FROM_2_3
    mApp->webSettings()->setThirdPartyCookiePolicy(m_blockThirdParty ?
            QWebSettings::AlwaysBlockThirdPartyCookies :
            QWebSettings::AlwaysAllowThirdPartyCookies);
#endif
}

void CookieJar::setAllowCookies(bool allow)
{
    m_allowCookies = allow;
}

bool CookieJar::rejectCookie(const QString &domain, const QNetworkCookie &cookie) const
{
    Q_UNUSED(domain)

    const QString &cookieDomain = cookie.domain();

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

// This feature is now natively in QtWebKit 2.3
#if QTWEBKIT_TO_2_3
    if (m_blockThirdParty) {
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

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    QList<QNetworkCookie> newList;

    foreach(QNetworkCookie cookie, cookieList) {
        // If cookie domain is empty, set it to url.host()
        if (cookie.domain().isEmpty()) {
            cookie.setDomain(url.host());
        }

        if (!rejectCookie(url.host(), cookie)) {
            newList.append(cookie);
        }
    }

    return QNetworkCookieJar::setCookiesFromUrl(newList, url);
}

void CookieJar::saveCookies()
{
    if (mApp->isPrivateSession()) {
        return;
    }

    QList<QNetworkCookie> allCookies;

    if (!m_deleteOnClose) {
        // If we are deleting cookies on close, let's just save empty cookie list
        allCookies = getAllCookies();
    }
    else {
        // Do not delete whitelisted cookies
        QList<QNetworkCookie> cookies = getAllCookies();
        int count = cookies.count();

        for (int i = 0; i < count; i++) {
            const QNetworkCookie &cookie = cookies.at(i);

            if (listMatchesDomain(m_whitelist, cookie.domain())) {
                allCookies.append(cookie);
            }
        }
    }

    QFile file(m_activeProfil + "cookies.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    int count = allCookies.count();

    stream << count;
    for (int i = 0; i < count; i++) {
        const QNetworkCookie &cookie = allCookies.at(i);

        if (cookie.isSessionCookie()) {
            continue;
        }
        stream << cookie.toRawForm();
    }

    file.close();
}

void CookieJar::restoreCookies()
{
    if (!QFile::exists(m_activeProfil + "cookies.dat") || mApp->isPrivateSession()) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();

    QList<QNetworkCookie> restoredCookies;
    QFile file(m_activeProfil + "cookies.dat");
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    int count;

    stream >> count;
    for (int i = 0; i < count; i++) {
        QByteArray rawForm;
        stream >> rawForm;
        const QList<QNetworkCookie> &cookieList = QNetworkCookie::parseCookies(rawForm);
        if (cookieList.isEmpty()) {
            continue;
        }

        const QNetworkCookie &cookie = cookieList.at(0);

        if (cookie.expirationDate() < now) {
            continue;
        }
        restoredCookies.append(cookie);
    }

    file.close();
    setAllCookies(restoredCookies);
}

void CookieJar::clearCookies()
{
    setAllCookies(QList<QNetworkCookie>());
}

QList<QNetworkCookie> CookieJar::getAllCookies()
{
    return QNetworkCookieJar::allCookies();
}

void CookieJar::setAllCookies(const QList<QNetworkCookie> &cookieList)
{
    QNetworkCookieJar::setAllCookies(cookieList);
}

bool CookieJar::matchDomain(QString cookieDomain, QString siteDomain)
{
    // According to RFC 6265

    // Remove leading dot
    if (cookieDomain.startsWith(QLatin1Char('.'))) {
        cookieDomain = cookieDomain.mid(1);
    }

    if (siteDomain.startsWith(QLatin1Char('.'))) {
        siteDomain = siteDomain.mid(1);
    }

    if (cookieDomain == siteDomain) {
        return true;
    }

    if (!siteDomain.endsWith(cookieDomain)) {
        return false;
    }

    int index = siteDomain.indexOf(cookieDomain);

    return index > 0 && siteDomain[index - 1] == QLatin1Char('.');
}

bool CookieJar::listMatchesDomain(const QStringList &list, const QString &cookieDomain)
{
    foreach(const QString & d, list) {
        if (matchDomain(d, cookieDomain)) {
            return true;
        }
    }

    return false;
}
