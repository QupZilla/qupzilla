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
#include "qztoolstest.h"
#include "qztools.h"

#include <QtTest/QtTest>

void QzToolsTest::samePartOfStrings_data()
{
    QTest::addColumn<QString>("string1");
    QTest::addColumn<QString>("string2");
    QTest::addColumn<QString>("result");

    // Lorem ipsum dolor sit amet, consectetur adipiscing elit.
    QTest::newRow("General") << "Lorem ipsum dolor" << "Lorem ipsum dolor Test_1" << "Lorem ipsum dolor";
    QTest::newRow("OneChar") << "L" << "LTest_1" << "L";
    QTest::newRow("EmptyReturn") << "Lorem ipsum dolor" << "orem ipsum dolor Test_1" << "";
    QTest::newRow("EmptyString1") << "" << "orem ipsum dolor Test_1" << "";
    QTest::newRow("EmptyString2") << "Lorem ipsum dolor" << "" << "";
    QTest::newRow("EmptyBoth") << "" << "" << "";
}

void QzToolsTest::samePartOfStrings()
{
    QFETCH(QString, string1);
    QFETCH(QString, string2);
    QFETCH(QString, result);

    QCOMPARE(QzTools::samePartOfStrings(string1, string2), result);
}

void QzToolsTest::getFileNameFromUrl_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QString>("result");

    QTest::newRow("Basic") << QUrl("http://www.google.com/filename.html") << "filename.html";
    QTest::newRow("OnlyHost") << QUrl("http://www.google.com/") << "www.google.com";
    QTest::newRow("OnlyHostWithoutSlash") << QUrl("http://www.google.com") << "www.google.com";
    QTest::newRow("EndingDirectory") << QUrl("http://www.google.com/filename/") << "filename";
    QTest::newRow("EmptyUrl") << QUrl("") << "";
    QTest::newRow("OnlyScheme") << QUrl("http:") << "";
    QTest::newRow("FileSchemeUrl") << QUrl("file:///usr/share/test/file.tx") << "file.tx";
    QTest::newRow("FileSchemeUrlDirectory") << QUrl("file:///usr/share/test/") << "test";
    QTest::newRow("FileSchemeUrlRoot") << QUrl("file:///") << "";
}

void QzToolsTest::getFileNameFromUrl()
{
    QFETCH(QUrl, url);
    QFETCH(QString, result);

    QCOMPARE(QzTools::getFileNameFromUrl(url), result);
}
