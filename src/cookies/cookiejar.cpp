/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
//#define COOKIE_DEBUG

//TODO: black/white listing
CookieJar::CookieJar(QupZilla* mainClass, QObject* parent)
    : QNetworkCookieJar(parent)
    , p_QupZilla(mainClass)
{
    m_activeProfil = mApp->getActiveProfilPath();
    loadSettings();
}

void CookieJar::loadSettings()
{
    QSettings settings(m_activeProfil + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    m_allowCookies = settings.value("allowCookies", true).toBool();
    m_allowCookiesFromDomain = settings.value("allowCookiesFromVisitedDomainOnly", false).toBool();
    m_filterTrackingCookie = settings.value("filterTrackingCookie", false).toBool();
    m_deleteOnClose = settings.value("deleteCookiesOnClose", false).toBool();
}

void CookieJar::setAllowCookies(bool allow)
{
    m_allowCookies = allow;
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    if (!m_allowCookies) {
        return QNetworkCookieJar::setCookiesFromUrl(QList<QNetworkCookie>(), url);
    }

    QList<QNetworkCookie> newList = cookieList;

    foreach(QNetworkCookie cok, newList) {
        if (m_allowCookiesFromDomain && !QString("." + url.host()).contains(cok.domain().remove("www."))) {
#ifdef COOKIE_DEBUG
            qDebug() << "purged for domain mismatch" << cok;
#endif
            newList.removeOne(cok);
            continue;
        }

        if (m_filterTrackingCookie && cok.name().startsWith("__utm")) {
#ifdef COOKIE_DEBUG
            qDebug() << "purged as tracking " << cok;
#endif
            newList.removeOne(cok);
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

    QList<QNetworkCookie> allCookies = getAllCookies();
    QFile file(m_activeProfil + "cookies.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    int count = allCookies.count();

    stream << count;
    for (int i = 0; i < count; i++) {
        stream << allCookies.at(i).toRawForm();
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
        QNetworkCookie cok = QNetworkCookie::parseCookies(rawForm).at(0);
        if (cok.expirationDate() < now) {
            continue;
        }
        if (cok.isSessionCookie()) {
            continue;
        }
        restoredCookies.append(cok);
    }

    file.close();
    setAllCookies(restoredCookies);
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

