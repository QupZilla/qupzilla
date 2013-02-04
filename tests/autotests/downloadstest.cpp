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
#include "downloadstest.h"
#include "downloadfilehelper.h"

#include <QtTest/QtTest>
#include <QNetworkReply>

void DownloadsTest::parseContentDispositionTest_data()
{
    QTest::addColumn<QByteArray>("header");
    QTest::addColumn<QString>("result");

    QTest::newRow("filename") << QByteArray("attachment; filename=\"foo.html\"") << "foo.html";
    QTest::newRow("filename25") << QByteArray("attachment; filename=\"0000000000111111111122222\"") << "0000000000111111111122222";
    QTest::newRow("filename35") << QByteArray("attachment; filename=\"00000000001111111111222222222233333\"") << "00000000001111111111222222222233333";
    QTest::newRow("semicolon") << QByteArray("attachment; filename=\"Here's a semicolon;.html\"") << "Here's a semicolon;.html";
    QTest::newRow("semicolon2") << QByteArray("attachment; filename=\"Here's a semi\\\"colon;.html\"") << "Here's a semi\\\"colon;.html";
    QTest::newRow("semicolon3") << QByteArray("attachment; filename=\"Here's a\\\" semi\\\"colon;.html\"") << "Here's a\\\" semi\\\"colon;.html";
    QTest::newRow("invalidParameter") << QByteArray("attachment; foo=\"bar\"; filename=\"foo.html\"") << "foo.html";
    QTest::newRow("filenameUpper") << QByteArray("attachment; FILENAME=\"foo.html\"") << "foo.html";
    QTest::newRow("noQuotes") << QByteArray("attachment; filename=foo.html") << "foo.html";
    QTest::newRow("singleQuotesFileame") << QByteArray("attachment; filename='foo.bar'") << "'foo.bar'";
    QTest::newRow("filenamePlain") << QByteArray("attachment; filename=\"foo-ä.html\"") << QString::fromUtf8("foo-ä.html");
    QTest::newRow("percent") << QByteArray("attachment; filename=\"foo-%41.html\"") << "foo-%41.html";
    QTest::newRow("percent2") << QByteArray("attachment; filename=\"foo-%c3%a4-%e2%82%ac.html\"") << "foo-%c3%a4-%e2%82%ac.html";
    QTest::newRow("withSpace") << QByteArray("attachment; filename =\"foo.html\"") << "foo.html";
    QTest::newRow("filenameInside") << QByteArray("attachment; example=\"filename=example.txt\"") << "";
    QTest::newRow("xfilename") << QByteArray("attachment; xfilename=\"example.txt\"") << "";
    QTest::newRow("withSpaceBefore") << QByteArray("attachment; filename *=UTF-8''foo-%c3%a4.html") << "";
    QTest::newRow("withSpaceAfter") << QByteArray("attachment; filename*= UTF-8''foo-%c3%a4.html") << QString::fromUtf8("foo-ä.html");
    QTest::newRow("withSpaceInside") << QByteArray("attachment; filename* =UTF-8''foo-%c3%a4.html") << QString::fromUtf8("foo-ä.html");
    QTest::newRow("withDoubleQuotes") << QByteArray("attachment; filename*=\"UTF-8''foo-%c3%a4.html\"") << "";
    QTest::newRow("multiTypes") << QByteArray("attachment; filename*=UTF-8''foo-%c3%a4.html; filename=\"foo-ae.html\"") << QString::fromUtf8("foo-ä.html");

    // Ignored, but passing in browser
    // QTest::newRow("filenameUtf8") << QByteArray("attachment; filename=\"foo-Ã¤.html\"") << QString::fromUtf8("foo-ä.html");
    // QTest::newRow("*utf8") << QByteArray("attachment; filename*=UTF-8''foo-%c3%a4-%e2%82%ac.html") << QString::fromUtf8("foo-ä-€.html");
    // QTest::newRow("rfc2231") << QByteArray("attachment; filename*=UTF-8''foo-a%cc%88.html") << QString::fromUtf8("foo-ä.html");

    // ISO-8859-1 decoding not supported
    // QTest::newRow("*iso") << QByteArray("attachment; filename*=iso-8859-1''foo-%E4.html") << QString::fromUtf8("foo-ä.html");
    // QTest::newRow("multiTypes2") << QByteArray("attachment; filename*=ISO-8859-1''currency-sign%3d%a4; filename=\"foo-ae.html\"") << QString::fromUtf8("currency-sign=¤");

    // Not yet supported
    // QTest::newRow("multiType2") << QByteArray("attachment; filename*0*=ISO-8859-15''euro-sign%3d%a4; filename*=ISO-8859-1''currency-sign%3d%a4") << QString::fromUtf8("euro-sign=€");
}

void DownloadsTest::parseContentDispositionTest()
{
    QFETCH(QByteArray, header);
    QFETCH(QString, result);

    QCOMPARE(DownloadFileHelper::parseContentDisposition(header), result);
}
