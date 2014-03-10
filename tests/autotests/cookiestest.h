/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#ifndef COOKIESTEST_H
#define COOKIESTEST_H

#include <QObject>

#include "cookiejar.h"

class CookieJar_Tst : public CookieJar
{
public:
    explicit CookieJar_Tst()
        : CookieJar()
    {
    }

    bool matchDomain(QString cookieDomain, QString siteDomain) const
    {
        return CookieJar::matchDomain(cookieDomain, siteDomain);
    }

    bool listMatchesDomain(const QStringList &list, const QString &cookieDomain) const
    {
        return CookieJar::listMatchesDomain(list, cookieDomain);
    }
};

class CookiesTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void domainMatchingTest_data();
    void domainMatchingTest();

    void listMatchesDomainTest_data();
    void listMatchesDomainTest();

private:
    CookieJar_Tst *m_cookieJar;
};

#endif // COOKIESTEST_H
