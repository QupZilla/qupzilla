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
#include <QWebSettings>
#include <QDateTime>

//#define COOKIE_DEBUG

CookieJar::CookieJar(QObject* parent)
    : QNetworkCookieJar(parent)
    , m_autoSaver(0)
{
    m_autoSaver = new AutoSaver(this);
    connect(m_autoSaver, SIGNAL(save()), this, SLOT(saveCookies()));

    loadSettings();
    restoreCookies();
}

CookieJar::~CookieJar()
{
    m_autoSaver->saveIfNecessary();
}

void CookieJar::loadSettings()
{
    Settings settings;
    settings.beginGroup("Cookie-Settings");
    m_allowCookies = settings.value("allowCookies", true).toBool();
    m_allowThirdParty = settings.value("allowThirdPartyCookies", 0).toInt();
    m_filterTrackingCookie = settings.value("filterTrackingCookie", false).toBool();
    m_deleteOnClose = settings.value("deleteCookiesOnClose", false).toBool();
    m_whitelist = settings.value("whitelist", QStringList()).toStringList();
    m_blacklist = settings.value("blacklist", QStringList()).toStringList();
    settings.endGroup();

#if QTWEBKIT_FROM_2_3
    switch (m_allowThirdParty) {
    case 0:
        QWebSettings::globalSettings()->setThirdPartyCookiePolicy(QWebSettings::AlwaysAllowThirdPartyCookies);
        break;

    case 1:
        QWebSettings::globalSettings()->setThirdPartyCookiePolicy(QWebSettings::AlwaysBlockThirdPartyCookies);
        break;

    case 2:
        QWebSettings::globalSettings()->setThirdPartyCookiePolicy(QWebSettings::AllowThirdPartyWithExistingCookies);
        break;
    }
#endif
}

void CookieJar::setAllowCookies(bool allow)
{
    m_allowCookies = allow;
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    QList<QNetworkCookie> newList;

    foreach (QNetworkCookie cookie, cookieList) {
        if (!rejectCookie(url.host(), cookie)) {
            newList.append(cookie);
        }
    }

    bool added = QNetworkCookieJar::setCookiesFromUrl(newList, url);

    if (added) {
        m_autoSaver->changeOcurred();
    }

    return added;
}

QList<QNetworkCookie> CookieJar::allCookies() const
{
    return QNetworkCookieJar::allCookies();
}

void CookieJar::setAllCookies(const QList<QNetworkCookie> &cookieList)
{
    m_autoSaver->changeOcurred();
    QNetworkCookieJar::setAllCookies(cookieList);
}

void CookieJar::clearCookies()
{
    setAllCookies(QList<QNetworkCookie>());
}

void CookieJar::restoreCookies()
{
    if (mApp->isPrivate()) {
        return;
    }

    const QString cookiesFile = DataPaths::currentProfilePath() + QLatin1String("/cookies.dat");
    QDateTime now = QDateTime::currentDateTime();

    QList<QNetworkCookie> restoredCookies;
    QFile file(cookiesFile);
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

        const QNetworkCookie cookie = cookieList.at(0);

        if (cookie.expirationDate() < now) {
            continue;
        }
        restoredCookies.append(cookie);
    }

    file.close();
    QNetworkCookieJar::setAllCookies(restoredCookies);
}

void CookieJar::saveCookies()
{
    if (mApp->isPrivate()) {
        return;
    }

    QList<QNetworkCookie> cookies = allCookies();

    if (m_deleteOnClose) {
        // If we are deleting cookies on close, save only whitelisted cookies
        cookies.clear();
        QList<QNetworkCookie> aCookies = allCookies();

        foreach (const QNetworkCookie &cookie, aCookies) {
            if (listMatchesDomain(m_whitelist, cookie.domain())) {
                cookies.append(cookie);
            }
        }
    }

    QFile file(DataPaths::currentProfilePath() + QLatin1String("/cookies.dat"));
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    int count = cookies.count();

    stream << count;
    for (int i = 0; i < count; i++) {
        const QNetworkCookie cookie = cookies.at(i);

        if (cookie.isSessionCookie()) {
            continue;
        }
        stream << cookie.toRawForm();
    }

    file.close();
}

bool CookieJar::rejectCookie(const QString &domain, const QNetworkCookie &cookie) const
{
    const QString cookieDomain = cookie.domain().isEmpty() ? domain : cookie.domain();

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
    if (m_allowThirdParty) {
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
