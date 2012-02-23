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
//#define COOKIE_DEBUG

bool containsDomain(QString string, QString domain)
{
    if (string.isEmpty()) {
        // Some cookies have empty domain() ... bug?
        return true;
    }

    string.prepend(".");
    if (domain.startsWith("www.")) {
        domain = domain.mid(4);
    }

    return string.contains(domain);
}

bool listContainsDomain(const QStringList &list, const QString &domain)
{
    foreach(const QString & d, list) {
        if (containsDomain(d, domain)) {
            return true;
        }
    }

    return false;
}

CookieJar::CookieJar(QupZilla* mainClass, QObject* parent)
    : QNetworkCookieJar(parent)
    , p_QupZilla(mainClass)
{
    m_activeProfil = mApp->getActiveProfilPath();
    loadSettings();
}

void CookieJar::loadSettings()
{
    Settings settings;
    settings.beginGroup("Cookie-Settings");
    m_allowCookies = settings.value("allowCookies", true).toBool();
    m_allowCookiesFromDomain = settings.value("allowCookiesFromVisitedDomainOnly", false).toBool();
    m_filterTrackingCookie = settings.value("filterTrackingCookie", false).toBool();
    m_deleteOnClose = settings.value("deleteCookiesOnClose", false).toBool();
    m_whitelist = settings.value("whitelist", QStringList()).toStringList();
    m_blacklist = settings.value("blacklist", QStringList()).toStringList();
    settings.endGroup();
}

void CookieJar::setAllowCookies(bool allow)
{
    m_allowCookies = allow;
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    QList<QNetworkCookie> newList = cookieList;

    foreach(const QNetworkCookie & cookie, newList) {
        if (!m_allowCookies && !listContainsDomain(m_whitelist, cookie.domain())) {
#ifdef COOKIE_DEBUG
            qDebug() << "not in whitelist" << cookie;
#endif
            newList.removeOne(cookie);
            continue;
        }

        if (m_allowCookies && listContainsDomain(m_blacklist, cookie.domain())) {
#ifdef COOKIE_DEBUG
            qDebug() << "found in blacklist" << cookie;
#endif
            newList.removeOne(cookie);
            continue;
        }

        if (m_allowCookiesFromDomain && !containsDomain(url.host(), cookie.domain())) {
#ifdef COOKIE_DEBUG
            qDebug() << "purged for domain mismatch" << cookie << cookie.domain() << url.host();
#endif
            newList.removeOne(cookie);
            continue;
        }

        if (m_filterTrackingCookie && cookie.name().startsWith("__utm")) {
#ifdef COOKIE_DEBUG
            qDebug() << "purged as tracking " << cookie;
#endif
            newList.removeOne(cookie);
            continue;
        }
    }

    return QNetworkCookieJar::setCookiesFromUrl(newList, url);
}

void CookieJar::saveCookies()
{
    if (m_deleteOnClose) {
        return;
    }

    QList<QNetworkCookie> allCookies;
    if (m_tempList.isEmpty()) {
        allCookies = getAllCookies();
    }
    else {
        allCookies = m_tempList;
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
    if (!QFile::exists(m_activeProfil + "cookies.dat")) {
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
    if (m_tempList.isEmpty()) {
        setAllCookies(QList<QNetworkCookie>());
    }
    else {
        m_tempList.clear();
    }
}

QList<QNetworkCookie> CookieJar::getAllCookies()
{
    return QNetworkCookieJar::allCookies();
}

void CookieJar::setAllCookies(const QList<QNetworkCookie> &cookieList)
{
    QNetworkCookieJar::setAllCookies(cookieList);
}

void CookieJar::turnPrivateJar(bool state)
{
    if (state) {
        m_tempList = QNetworkCookieJar::allCookies();
        QNetworkCookieJar::setAllCookies(QList<QNetworkCookie>());
    }
    else {
        QNetworkCookieJar::setAllCookies(m_tempList);
        m_tempList.clear();
    }
}

