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
#include "adblockrule.h"
#include "adblocksubscription.h"

#include <QtTest/QtTest>
#include <QNetworkRequest>

class AdBlockMatchRule : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void networkMatch();

private:
    AdBlockSubscription* m_subscription;
};


void AdBlockMatchRule::initTestCase()
{
    m_subscription = new AdBlockSubscription("EasyList", this);
    m_subscription->setFilePath("../files/easylist.txt");
    m_subscription->loadSubscription(QStringList());
}

void AdBlockMatchRule::cleanupTestCase()
{
    delete m_subscription;
}

void AdBlockMatchRule::networkMatch()
{
    QList<QUrl> urls;
    urls << QUrl("http://www.qupzilla.com");
    urls << QUrl("https://developers.google.com/feed/v1/reference?csw=1");
    urls << QUrl("http://pagead2.googlesyndication.com/pagead/show_ads.js");
    urls << QUrl("https://qt.gitorious.org/qt-labs/qwebchannel/source/d48ca4efa70624c3178c0b97441ff7499aa2be36:src/webchannel/qwebchannel.cpp");
    urls << QUrl("https://www.google.com/search?q=qmake+add+-Werror&ie=utf-8&oe=utf-8&aq=t&rls=org.mozilla:en-US:unofficial&client=iceweasel-a&channel=fflb#channel=fflb&q=gcc+-Werror&rls=org.mozilla:en-US:unofficial&start=10");
    urls << QUrl("https://googleads.g.doubleclick.net/pagead/viewthroughconversion/977354488/?random=1397378259090&cv=7&fst=1397378259090&num=1&fmt=1&guid=ON&u_h=1080&u_w=1920&u_ah=1080&u_aw=1862&u_cd=24&u_his=3&u_tz=120&u_java=true&u_nplug=3&u_nmime=70&frm=2&url=https%3A//2507573.fls.doubleclick.net/activityi%3Bsrc%3D2507573%3Btype%3Dother026%3Bcat%3Dgoogl875%3Bord%3D8821468765381.725%3F&ref=https%3A//developers.google.com/feed/v1/reference%3Fcsw%3D1");
    urls << QUrl("http://www.google-analytics.com/__utm.gif?utmwv=1.4&utmn=52554097&utmcs=ISO-8859-1&utmsr=1920x1080&utmsc=24-bit&utmul=cs-cz&utmje=1&utmfl=11.2 r202&utmdt=HTTP Authentication example&utmhn=www.pagetutor.com&utmhid=423185901&utmr=-&utmp=/keeper/http_authentication/index.html&utmac=UA-1399726-1&utmcc=__utma%3D30852926.644467994.1395073137.1395611798.1397378358.18%3B%2B__utmz%3D30852926.1395073137.1.1.utmccn%3D(direct)%7Cutmcsr%3D(direct)%7Cutmcmd%3D(none)%3B%2B");

    QBENCHMARK {
        foreach (const QUrl &url, urls) {
            QNetworkRequest req(url);
            const AdBlockRule* rule = m_subscription->match(req, url.host(), url.toEncoded());
            if (rule)
                rule = 0;
        }
    }
}

QTEST_MAIN(AdBlockMatchRule)
#include "adblockmatchrule.moc"
