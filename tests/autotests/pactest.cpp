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
#include "pactest.h"

#include <QtTest/QtTest>
#include <QDateTime>

void PacTest::initTestCase()
{
    m_runner = new ProxyAutoConfig_Tst;
}

void PacTest::cleanupTestCase()
{
    delete m_runner;
}

// Tests according to
// http://web.archive.org/web/20061218002753/wp.netscape.com/eng/mozilla/2.0/relnotes/demo/proxy-live.html

void PacTest::isPlainHostNameTest_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<bool>("result");

    QTest::newRow("doc1") << "www" << true;
    QTest::newRow("doc2") << "www.netscape.com" << false;
}

void PacTest::isPlainHostNameTest()
{
    QFETCH(QString, host);
    QFETCH(bool, result);

    QString source = QString("isPlainHostName('%1')").arg(host);
    QCOMPARE(m_runner->evaluate(source).toBool(), result);
}

void PacTest::dnsDomainIsTest_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("domain");
    QTest::addColumn<bool>("result");

    QTest::newRow("doc1") << "www.netscape.com" << ".netscape.com" << true;
    QTest::newRow("doc2") << "www" << ".netscape.com" << false;
    QTest::newRow("doc3") << "www.mcom.com" << ".netscape.com" << false;
}

void PacTest::dnsDomainIsTest()
{
    QFETCH(QString, host);
    QFETCH(QString, domain);
    QFETCH(bool, result);

    QString source = QString("dnsDomainIs('%1','%2')").arg(host, domain);
    QCOMPARE(m_runner->evaluate(source).toBool(), result);
}

void PacTest::localHostOrDomainIs_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("hostdom");
    QTest::addColumn<bool>("result");

    QTest::newRow("doc1") << "www.netscape.com" << "www.netscape.com" << true;
    QTest::newRow("doc2") << "www" << "www.netscape.com" << true;
    QTest::newRow("doc3") << "www.mcom.com" << "www.netscape.com" << false;
    QTest::newRow("doc4") << "home.netscape.com" << "www.netscape.com" << false;
}

void PacTest::localHostOrDomainIs()
{
    QFETCH(QString, host);
    QFETCH(QString, hostdom);
    QFETCH(bool, result);

    QString source = QString("localHostOrDomainIs('%1','%2')").arg(host, hostdom);
    QCOMPARE(m_runner->evaluate(source).toBool(), result);
}

void PacTest::isResolvableTest_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<bool>("result");

    QTest::newRow("doc1") << "www.netscape.com" << true;
    QTest::newRow("doc2") << "bogus.domain.foobar" << false;
}

void PacTest::isResolvableTest()
{
    QFETCH(QString, host);
    QFETCH(bool, result);

    QString source = QString("isResolvable('%1')").arg(host);
    QCOMPARE(m_runner->evaluate(source).toBool(), result);
}

void PacTest::isInNetTest_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("mask");
    QTest::addColumn<bool>("result");

    // is true if the IP address of host matches exactly 198.95.249.79.
    QTest::newRow("doc1") << "198.95.249.79" << "198.95.249.79" << "255.255.255.255" << true;
    QTest::newRow("doc1-2") << "198.95.249.80" << "198.95.249.79" << "255.255.255.255" << false;
    QTest::newRow("doc1-3") << "198.95.248.79" << "198.95.249.79" << "255.255.255.255" << false;
    QTest::newRow("doc1-4") << "198.20.249.80" << "198.95.249.79" << "255.255.255.255" << false;
    QTest::newRow("doc1-5") << "123.95.249.80" << "198.95.249.79" << "255.255.255.255" << false;

    // is true if the IP address of the host matches 198.95.*.*.
    QTest::newRow("doc2") << "198.95.249.79" << "198.95.0.0" << "255.255.0.0" << true;
    QTest::newRow("doc2-2") << "198.95.0.0" << "198.95.0.0" << "255.255.0.0" << true;
    QTest::newRow("doc2-3") << "198.94.249.79" << "198.95.0.0" << "255.255.0.0" << false;
    QTest::newRow("doc2-3") << "198.94.249.79" << "198.95.0.0" << "255.255.0.0" << false;
    QTest::newRow("doc2-3") << "148.94.249.79" << "198.95.0.0" << "255.255.0.0" << false;
    QTest::newRow("doc2-3") << "128.94.249.79" << "198.95.0.0" << "255.255.0.0" << false;
    QTest::newRow("doc2-3") << "23.94.249.79" << "198.95.0.0" << "255.255.0.0" << false;

    // is true if the IP address of host matches 85.118.128.* (qupzilla.com)
    // if host is passed as hostname, the function needs to resolve it
    QTest::newRow("resolve1") << "qupzilla.com" << "85.118.128.38" << "255.255.255.0" << true;
    QTest::newRow("resolve1-2") << "yahoo.com" << "173.194.70.0" << "255.255.255.0" << false;
    QTest::newRow("resolve1-3") << "netscape.com" << "173.194.70.0" << "255.255.255.0" << false;
    QTest::newRow("resolve1-4") << "mozilla.com" << "173.194.70.0" << "255.255.255.0" << false;
}

void PacTest::isInNetTest()
{
    QFETCH(QString, host);
    QFETCH(QString, pattern);
    QFETCH(QString, mask);
    QFETCH(bool, result);

    QString source = QString("isInNet('%1','%2','%3')").arg(host, pattern, mask);
    QCOMPARE(m_runner->evaluate(source).toBool(), result);
}

void PacTest::dnsResolveTest_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("result");

    QTest::newRow("localhost") << "localhost" << "127.0.0.1";
    QTest::newRow("qz") << "qupzilla.com" << "85.118.128.38"; // This may change...
}

void PacTest::dnsResolveTest()
{
    QFETCH(QString, host);
    QFETCH(QString, result);

    QString source = QString("dnsResolve('%1')").arg(host);
    QCOMPARE(m_runner->evaluate(source).toString(), result);
}

void PacTest::dnsDomainLevelsTest_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<int>("result");

    QTest::newRow("doc1") << "www" << 0;
    QTest::newRow("doc2") << "www.netscape.com" << 2;
}

void PacTest::dnsDomainLevelsTest()
{
    QFETCH(QString, host);
    QFETCH(int, result);

    QString source = QString("dnsDomainLevels('%1')").arg(host);
    QCOMPARE(m_runner->evaluate(source).toString().toInt(), result);
}

void PacTest::shExpMatchTest_data()
{
    QTest::addColumn<QString>("str");
    QTest::addColumn<QString>("shexp");
    QTest::addColumn<bool>("result");

    QTest::newRow("doc1") << "http://home.netscape.com/people/ari/index.html" << "*/ari/*" << true;
    QTest::newRow("doc2") << "http://home.netscape.com/people/montulli/index.html" << "*/ari/*" << false;

    QTest::newRow("glob1") << "com/people" << "*om/*" << true;
    QTest::newRow("glob2") << "com/people" << "com/*" << true;
    QTest::newRow("glob3") << "com/people" << "om/*" << false;

    QTest::newRow("char1") << "com/people" << "co?/*" << true;
    QTest::newRow("char2") << "com/people" << "?com/*" << false;
    QTest::newRow("char3") << "com/people" << "?scom/*" << false;
    QTest::newRow("char4") << "com/people" << "com/pe?ple*" << true;

    QTest::newRow("dot1") << "com/people.org" << "co?/*.org" << true;
    QTest::newRow("dot2") << "com/people.org" << "co?/*.or" << false;
    QTest::newRow("dot3") << "com/people.org" << "com/people.*g" << true;
    QTest::newRow("dot4") << "com/people.org" << "com/*.*g" << true;
}

void PacTest::shExpMatchTest()
{
    QFETCH(QString, str);
    QFETCH(QString, shexp);
    QFETCH(bool, result);

    QString source = QString("shExpMatch('%1','%2')").arg(str, shexp);
    QCOMPARE(m_runner->evaluate(source).toBool(), result);
}

static QString dayName(int weekday)
{
    switch (weekday) {
    case 1: return "MON";
    case 2: return "TUE";
    case 3: return "WED";
    case 4: return "THU";
    case 5: return "FRI";
    case 6: return "SAT";
    case 7: return "SUN";
    default: return "MON";
    }
}

void PacTest::dateTimeTest()
{
    int day = QDateTime::currentDateTime().date().day();
    int hour = QDateTime::currentDateTime().time().hour();
    int week = QDateTime::currentDateTime().date().dayOfWeek();

    QString source = QString("weekdayRange('%1')").arg(dayName(week));
    QCOMPARE(m_runner->evaluate(source).toBool(), true);

    source = QString("weekdayRange('%1')").arg(dayName(week + 1));
    QCOMPARE(m_runner->evaluate(source).toBool(), false);

    source = QString("dateRange('%1')").arg(day);
    QCOMPARE(m_runner->evaluate(source).toBool(), true);

    source = QString("dateRange('%1')").arg(day + 1);
    QCOMPARE(m_runner->evaluate(source).toBool(), false);

    source = QString("timeRange('%1')").arg(hour);
    QCOMPARE(m_runner->evaluate(source).toBool(), true);

    source = QString("timeRange('%1')").arg(hour + 1);
    QCOMPARE(m_runner->evaluate(source).toBool(), false);
}
