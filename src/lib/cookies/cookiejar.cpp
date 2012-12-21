/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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

//#define COOKIE_DEBUG

bool blockThirdParty(QString string, QString domain)
{
    if (string.isEmpty()) {
        // Some cookies have empty domain() ... bug?
        return false;
    }

    if (string.startsWith(QLatin1String("www."))) {
        string = string.mid(3);
    }

    if (domain.startsWith(QLatin1String("www."))) {
        domain = domain.mid(4);
    }

    return !domain.endsWith(string);
}

bool matchDomain(const QString &domain, const QString &filter)
{
    // According to RFC 6265

    if (domain == filter) {
        return true;
    }

    if (!domain.endsWith(filter)) {
        return false;
    }

    int index = domain.indexOf(filter);

    return (index > 0 && filter[0] == QLatin1Char('.'));
}

int listContainsDomain(const QStringList &list, const QString &domain)
{
    if (domain.isEmpty()) {
        return -1;
    }

    foreach(const QString & d, list) {
        if (matchDomain(domain, d)) {
            return 1;
        }
    }

    return 0;
}

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

#if QT_VERSION >= 0x050000
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
        int result = listContainsDomain(m_whitelist, cookieDomain);
        if (result != 1) {
#ifdef COOKIE_DEBUG
            qDebug() << "not in whitelist" << cookie;
#endif
            return true;
        }
    }

    if (m_allowCookies) {
        int result = listContainsDomain(m_blacklist, cookieDomain);
        if (result == 1) {
#ifdef COOKIE_DEBUG
            qDebug() << "found in blacklist" << cookie;
#endif
            return true;
        }
    }

// This feature is now natively in QtWebKit in Qt 5
#if QT_VERSION < 0x050000
    if (m_blockThirdParty) {
        bool result = blockThirdParty(cookieDomain, domain);
        if (result) {
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
    QList<QNetworkCookie> newList = cookieList;

    foreach(const QNetworkCookie & cookie, newList) {
        if (rejectCookie(url.host(), cookie)) {
            newList.removeOne(cookie);
            continue;
        }
    }

    return QNetworkCookieJar::setCookiesFromUrl(newList, url);
}

void CookieJar::saveCookies()
{
    if (m_deleteOnClose || mApp->isPrivateSession()) {
        return;
    }

    QList<QNetworkCookie> allCookies = getAllCookies();

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
