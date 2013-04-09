/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#include "adblocktest.h"
#include "adblockrule.h"

#include <QtTest/QtTest>

class AdBlockRule_Test : public AdBlockRule
{
public:
    QStringList parseRegExpFilter(const QString &parsedFilter)
    {
        return AdBlockRule::parseRegExpFilter(parsedFilter);
    }

    bool isMatchingDomain(const QString &domain, const QString &filter) const
    {
        return AdBlockRule::isMatchingDomain(domain, filter);
    }
};

void AdBlockTest::isMatchingCookieTest_data()
{
    // Test copied from CookiesTest
    QTest::addColumn<QString>("filterDomain");
    QTest::addColumn<QString>("siteDomain");
    QTest::addColumn<bool>("result");

    QTest::newRow("test1") << "example.com" << "www.example.com" << true;
    QTest::newRow("test2") << "example.com" << "example.com" << true;
    QTest::newRow("test3") << "example.com" << "anotherexample.com" << false;
    QTest::newRow("test4") << "test.example.com" << "example.com" << false;
    QTest::newRow("test5") << "www.example.com" << "example.com" << false;
    QTest::newRow("test_empty") << "www.example.com" << "" << false;
    QTest::newRow("test_empty2") << "" << "example.com" << false;
}

void AdBlockTest::isMatchingCookieTest()
{
    AdBlockRule_Test rule_test;

    QFETCH(QString, filterDomain);
    QFETCH(QString, siteDomain);
    QFETCH(bool, result);

    QCOMPARE(rule_test.isMatchingDomain(siteDomain, filterDomain), result);
}

void AdBlockTest::parseRegExpFilterTest_data()
{
    QTest::addColumn<QString>("parsedFilter");
    QTest::addColumn<QStringList>("result");

    QTest::newRow("test1") << "||doubleclick.net/pfadx/tmg.telegraph."
                           << (QStringList() << "doubleclick.net/pfadx/tmg.telegraph.");
    QTest::newRow("test2") << "||doubleclick.net/pfadx/*.mtvi"
                           << (QStringList() << "doubleclick.net/pfadx/" << ".mtvi");
    QTest::newRow("test3") << "&prvtof=*&poru="
                           << (QStringList() << "&prvtof=" << "&poru=");
    QTest::newRow("test4") << "/addyn|*;adtech;"
                           << (QStringList() << "/addyn" << ";adtech;");
    QTest::newRow("test5") << "/eas_fif.html^"
                           << (QStringList() << "/eas_fif.html");
    QTest::newRow("test6") << "://findnsave.^.*/api/groupon.json?"
                           << (QStringList() << "://findnsave." << "/api/groupon.json?");
    QTest::newRow("test7") << "^fp=*&prvtof="
                           << (QStringList() << "fp=" << "&prvtof=");
    QTest::newRow("test8") << "|http://ax-d.*/jstag^"
                           << (QStringList() << "http://ax-d." << "/jstag");
    QTest::newRow("test9") << "||reuters.com^*/rcom-wt-mlt.js"
                           << (QStringList() << "reuters.com" <<"/rcom-wt-mlt.js");
    QTest::newRow("test10") << "||chip.de^*/tracking.js"
                           << (QStringList() << "chip.de" << "/tracking.js");
    QTest::newRow("ignore1char") << "/search.php?uid=*.*&src="
                           << (QStringList() << "/search.php?uid=" << "&src=");
    QTest::newRow("ignoreDuplicates") << "/search.*.dup.*.dup.*&src="
                           << (QStringList() << "/search." << ".dup." << "&src=");
    QTest::newRow("empty") << QString()
                           << (QStringList());
    QTest::newRow("justspaces") << QString("       ")
                           << (QStringList() << "       ");
    QTest::newRow("spacesWithMetachars") << QString("   *    ?")
                           << (QStringList() << "   " << "    ?");
}

void AdBlockTest::parseRegExpFilterTest()
{
    AdBlockRule_Test rule_test;

    QFETCH(QString, parsedFilter);
    QFETCH(QStringList, result);

    QCOMPARE(rule_test.parseRegExpFilter(parsedFilter), result);
}
