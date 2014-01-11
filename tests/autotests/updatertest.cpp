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
#include "updatertest.h"
#include "updater.h"

#include <QtTest/QtTest>

void UpdaterTest::parseVersionsTest_data()
{
    QTest::addColumn<QString>("versionString");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("major");
    QTest::addColumn<int>("minor");
    QTest::addColumn<int>("revision");

    QTest::newRow("zeros") << "0.0.0" << true << 0 << 0 << 0;
    QTest::newRow("zero-1") << "0.0.1" << true << 0 << 0 << 1;
    QTest::newRow("current") << "1.4.1" << true << 1 << 4 << 1;
    QTest::newRow("next-bugfix") << "1.4.2" << true << 1 << 4 << 2;
    QTest::newRow("2digits") << "2.5.15" << true << 2 << 5 << 15;
    QTest::newRow("3digits") << "123.123.333" << true << 123 << 123 << 333;
    QTest::newRow("negative") << "-1.4.1" << false << 0 << 0 << 0;
    QTest::newRow("invalid") << "0.0.0-1" << false << 0 << 0 << 0;
    QTest::newRow("invalid2") << "invalid1text" << false << 0 << 0 << 0;
}

void UpdaterTest::parseVersionsTest()
{
    QFETCH(QString, versionString);
    QFETCH(bool, valid);
    QFETCH(int, major);
    QFETCH(int, minor);
    QFETCH(int, revision);

    Updater::Version v(versionString);

    QCOMPARE(v.isValid, valid);

    if (valid) {
        QCOMPARE(v.majorVersion, major);
        QCOMPARE(v.minorVersion, minor);
        QCOMPARE(v.revisionNumber, revision);
    }
}

void UpdaterTest::compareVersionsTest_data()
{
    QTest::addColumn<QString>("version1");
    QTest::addColumn<QString>("version2");
    QTest::addColumn<bool>("less");
    QTest::addColumn<bool>("more");
    QTest::addColumn<bool>("equal");

    QTest::newRow("test1") << "0.0.1" << "0.0.2" << true << false << false;
    QTest::newRow("test2") << "0.1.2" << "0.0.2" << false << true << false;
    QTest::newRow("test3") << "1.0.1" << "0.0.2" << false << true << false;
    QTest::newRow("test4") << "1.4.1" << "1.4.2" << true << false << false;
    QTest::newRow("test5") << "1.5.0" << "1.4.2" << false << true << false;
    QTest::newRow("test6") << "1.5.0" << "1.5.0" << false << false << true;
    QTest::newRow("test7") << "1.5.1" << "1.4.2" << false << true << false;
    QTest::newRow("test8") << "1.4.1" << "1.4.2" << true << false << false;
}

void UpdaterTest::compareVersionsTest()
{
    QFETCH(QString, version1);
    QFETCH(QString, version2);
    QFETCH(bool, less);
    QFETCH(bool, more);
    QFETCH(bool, equal);

    Updater::Version v1(version1);
    Updater::Version v2(version2);

    QCOMPARE(v1 < v2, less);
    QCOMPARE(v1 > v2, more);
    QCOMPARE(v1 == v2, equal);
}
