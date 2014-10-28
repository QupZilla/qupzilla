/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#include "networktest.h"

#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QEventLoop>

void NetworkTest::initTestCase()
{
    m_manager = new QNetworkAccessManager;

    QSslConfiguration conf = QSslConfiguration::defaultConfiguration();
    conf.setProtocol(QSsl::SslV3);
    QSslConfiguration::setDefaultConfiguration(conf);
}

void NetworkTest::cleanupTestCase()
{
    delete m_manager;
}

void NetworkTest::sslv3test_data()
{
    QTest::addColumn<QUrl>("url");

    // Sites that loads only with SslV3 forced and have it forced in NetworkManager
    QTest::newRow("centrum.sk") << QUrl("https://user.centrum.sk/");
    QTest::newRow("centrum.cz") << QUrl("https://user.centrum.cz/");
    QTest::newRow("oneaccount.com") << QUrl("https://service.oneaccount.com/onlineV2/OSV2?event=login&pt=3");
    QTest::newRow("office-webapps") << QUrl("https://skydrive.live.com/view.aspx?resid=4FE8716FF67627C7!1218&cid=4fe8716ff67627c7&app=Word&wdo=2");
    QTest::newRow("i0.cz") << QUrl("https://i0.cz/6/ju/css/login/centrum.sk.css");
}

void NetworkTest::sslv3test()
{
    QFETCH(QUrl, url);

    QNetworkReply *reply = m_manager->get(QNetworkRequest(url));

    QEventLoop loop;
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(QNetworkReply::NoError, reply->error());
    QCOMPARE(false, reply->readAll().isEmpty());
}
