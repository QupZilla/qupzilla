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
#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include <QFile>
#include <QStringList>
#include <QNetworkCookieJar>

#include "qzcommon.h"

class AutoSaver;

class QUPZILLA_EXPORT CookieJar : public QNetworkCookieJar
{
    Q_OBJECT

public:
    explicit CookieJar(QObject* parent = 0);
    ~CookieJar();

    void loadSettings();

    void setAllowCookies(bool allow);
    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

    QList<QNetworkCookie> allCookies() const;
    void setAllCookies(const QList<QNetworkCookie> &cookieList);

    void clearCookies();
    void restoreCookies();

private slots:
    void saveCookies();

protected:
    bool matchDomain(QString cookieDomain, QString siteDomain) const;
    bool listMatchesDomain(const QStringList &list, const QString &cookieDomain) const;

private:
    bool rejectCookie(const QString &domain, const QNetworkCookie &cookie) const;

    bool m_allowCookies;
    bool m_filterTrackingCookie;
    int m_blockThirdParty;
    bool m_deleteOnClose;

    QStringList m_whitelist;
    QStringList m_blacklist;

    AutoSaver* m_autoSaver;
};

#endif // COOKIEJAR_H
