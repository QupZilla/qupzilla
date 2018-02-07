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
#include "cookiestest.h"
#include "datapaths.h"
#include "settings.h"

#include <QtTest/QtTest>
#include <QDir>

void CookiesTest::initTestCase()
{
    m_cookieJar = new CookieJar_Tst;
}

void CookiesTest::cleanupTestCase()
{
    delete m_cookieJar;
}

void CookiesTest::domainMatchingTest_data()
{
    QTest::addColumn<QString>("cookieDomain");
    QTest::addColumn<QString>("siteDomain");
    QTest::addColumn<bool>("result");

    /* http://stackoverflow.com/questions/1062963/how-do-browser-cookie-domains-work
       1) Cookie with Domain=.example.com will be available for www.example.com
       2) Cookie with Domain=.example.com will be available for example.com
       3) Cookie with Domain=example.com will be converted to .example.com and thus will also be available for www.example.com
       4) Cookie with Domain=example.com will not be available for anotherexample.com
    */

    QTest::newRow("test1") << ".example.com" << "www.example.com" << true;
    QTest::newRow("test2") << ".example.com" << "example.com" << true;
    QTest::newRow("test3") << "example.com" << "www.example.com" << true;
    QTest::newRow("test4") << ".example.com" << "anotherexample.com" << false;
    QTest::newRow("test5") << "test.example.com" << "example.com" << false;
    QTest::newRow("test6") << ".www.example.com" << "www.example.com" << true;
    QTest::newRow("test7") << ".www.example.com" << "example.com" << false;
    QTest::newRow("test_empty") << ".www.example.com" << "" << false;
    QTest::newRow("test_empty2") << "" << "example.com" << false;
}

void CookiesTest::domainMatchingTest()
{
    QFETCH(QString, cookieDomain);
    QFETCH(QString, siteDomain);
    QFETCH(bool, result);

    QCOMPARE(m_cookieJar->matchDomain(cookieDomain, siteDomain), result);
}

void CookiesTest::listMatchesDomainTest_data()
{
    QTest::addColumn<QStringList>("list");
    QTest::addColumn<QString>("cookieDomain");
    QTest::addColumn<bool>("result");

    QStringList list;
    list << "www.example.com" << "accounts.google.com";
    QStringList list2;
    list2 << "anotherexample.com" << "a.b.x.google.com";

    QTest::newRow("test1") << list << ".www.example.com" << true;
    QTest::newRow("test2") << list << ".google.com" << false;
    QTest::newRow("test3") << list << ".accounts.google.com" << true;
    QTest::newRow("test4") << list << ".example.com" << false;
    QTest::newRow("test5") << list2 << "example.com" << false;
    QTest::newRow("test6") << list2 << "tst.anotherexample.com" << true;
    QTest::newRow("test7") << list2 << "b.x.google.com" << false;
    QTest::newRow("test8") << list2 << "c.a.b.x.google.com" << true;
    QTest::newRow("test9") << list2 << ".a.b.x.google.com" << true;
    QTest::newRow("test_empty") << list2 << "" << false;
}

void CookiesTest::listMatchesDomainTest()
{
    QFETCH(QStringList, list);
    QFETCH(QString, cookieDomain);
    QFETCH(bool, result);

    QCOMPARE(m_cookieJar->listMatchesDomain(list, cookieDomain), result);
}
