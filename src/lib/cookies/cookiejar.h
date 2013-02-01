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
#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include "qz_namespace.h"

#include <QFile>
#include <QStringList>
#include <QNetworkCookieJar>

class QupZilla;

class QT_QUPZILLA_EXPORT CookieJar : public QNetworkCookieJar
{
public:
    explicit CookieJar(QupZilla* mainClass, QObject* parent = 0);

    void loadSettings();
    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);
    QList<QNetworkCookie> getAllCookies();
    void setAllCookies(const QList<QNetworkCookie> &cookieList);

    void saveCookies();
    void restoreCookies();
    void clearCookies();

    void setAllowCookies(bool allow);

    static bool matchDomain(QString cookieDomain, QString siteDomain);
    static bool listMatchesDomain(const QStringList &list, const QString &cookieDomain);

private:
    bool rejectCookie(const QString &domain, const QNetworkCookie &cookie) const;

    QupZilla* p_QupZilla;

    bool m_allowCookies;
    bool m_filterTrackingCookie;
    bool m_blockThirdParty;
    bool m_deleteOnClose;

    QStringList m_whitelist;
    QStringList m_blacklist;

    QString m_activeProfil;
};

#endif // COOKIEJAR_H
