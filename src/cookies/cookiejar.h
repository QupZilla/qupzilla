/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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

#include <QNetworkCookieJar>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QFile>

class QupZilla;
class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit CookieJar(QupZilla* mainClass, QObject* parent = 0);

    void loadSettings();
    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);
    QList<QNetworkCookie> getAllCookies();
    void setAllCookies(const QList<QNetworkCookie> &cookieList);
    void saveCookies();
    void restoreCookies();
    void setAllowCookies(bool allow);

    void turnPrivateJar(bool state);

signals:

public slots:

private:
    QupZilla* p_QupZilla;
    bool m_allowCookies;
    bool m_filterTrackingCookie;
    bool m_allowCookiesFromDomain;
    bool m_deleteOnClose;

    QString m_activeProfil;
    QList<QNetworkCookie> m_tempList;
};

#endif // COOKIEJAR_H
