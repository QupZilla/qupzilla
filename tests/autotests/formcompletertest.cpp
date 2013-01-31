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
#include "formcompletertest.h"
#include "pageformcompleter.h"

#include <QWebView>
#include <QWebPage>
#include <QWebFrame>

void FormCompleterTest::initTestCase()
{
    view = new QWebView();
}

void FormCompleterTest::cleanupTestCase()
{
    delete view;
}

void FormCompleterTest::init()
{
    view->setHtml(QString());
}

void FormCompleterTest::completePageTest1()
{
    // Basic test

    QByteArray data = "username=tst_username&password=tst_password";

    QString html = "<form name='form1' method='post' action='foo.php'>"
                   "<input id='id1' type='text' name='username' value='xx'>"
                   "<input id='id2' type='password' name='password' value='xx'>"
                   "<input type='submit' value='submit' name='submit'>"
                   "</form>";

    completeWithData(html, data);

    QCOMPARE(getElementByIdValue("id1").toString(), QString("tst_username"));
    QCOMPARE(getElementByIdValue("id2").toString(), QString("tst_password"));
}

void FormCompleterTest::completePageTest2()
{
    // Real word test: GMail login
    // uses input type=email for username

    QByteArray data = "dsh=-821344601702946291&GALX=pAkf3B9TdCo&timeStmp=&secTok="
            "&_utf8=%E2%98%83&bgresponse=%21A0IjdiuCmhg1wERY2iLkbgozxgIAAAAOUgAAA"
            "AcqAMaxqx-r1kXOOzeHkuDrRiOJvacFeSxGJ-pg_KQbwMJk3mBUCoZO2g602Uq4upHsM"
            "KVfjKRkuPCC0bVCaglP90VLBN8lqCQ5zLSrczVa-WpBFXlEZsKm5UXasE2ZZUQfDc1MI"
            "VQbDUviv7Ap54jx6vlTinen6UlxWW_wAvtLkpSO1hqrWnDSDmvFtbJZX61BlMFoHTPYk"
            "ijYnuCWzrHWsfKVI8uigtpClgwBTGovCWzuLrbFhG6txV5SokxdfNhbr3Vv-zO9xCw&E"
            "mail=tst_mail%40google.com&&Passwd=pass+%CB%87+word1&signIn=P%+se&Per"
            "sistentCookie=yes&rmShown=1";

    QString html = "<form novalidate='' id='gaia_loginform' action='ServiceLoginAuth' method='post'>"
            "<input type='hidden' name='continue' id='continue' value='http://mail.google.com/mail/'>"
            "<input type='hidden' name='service' id='service' value='mail'>"
            "<input type='hidden' name='rm' id='rm' value='false'>"
            "<input type='hidden' name='dsh' id='dsh' value='3557358644105009435'>"
            "<input type='hidden' name='ltmpl' id='ltmpl' value='default'>"
            "<input type='hidden' name='scc' id='scc' value='1'>"
            "<input type='hidden' name='GALX' value='2_UW3T0wRN4'>"
            "<input type='hidden' name='timeStmp' id='timeStmp' value=''>"
            "<input type='hidden' name='secTok' id='secTok' value=''>"
            "<input type='hidden' id='_utf8' name='_utf8' value='d'>"
            "<input type='hidden' name='bgresponse' id='bgresponse' value='js_disabled'>"
            "<div class='email-div'>"
            "<label for='Email'><strong class='email-label'>Username</strong></label>"
            "<input type='email' spellcheck='false' name='Email' id='Email' value='xx'>"
            "</div>"
            "<div class='passwd-div'>"
            "<label for='Passwd'><strong class='passwd-label'>Password</strong></label>"
            "<input type='password' name='Passwd' id='Passwd' value='xx'>"
            "</div>"
            "<input type='submit' class='g-button g-button-submit' name='signIn' id='signIn' value='Login'>"
            "<label class='remember' onclick=''>"
            "<input type='checkbox' name='PersistentCookie' id='PersistentCookie' value='yes'>"
            "<strong class='remember-label'>"
            "</strong>"
            "</label>"
            "<input type='hidden' name='rmShown' value='1'>"
            "</form>";

    completeWithData(html, data);

    QCOMPARE(getElementByIdValue("Email").toString(), QString("tst_mail@google.com"));
    QCOMPARE(getElementByIdValue("Passwd").toString(), QString::fromUtf8("pass ˇ word1"));
}

void FormCompleterTest::completePageTest3()
{
    // This test is mainly to test properly decoding special characters
    // in post data.

    QByteArray data = "user=%2B%C4%9B%C5%A1S+%CB%87+-+%2520+%2F+aa_&"
            "pass=%C3%BD%C5%BE%C4%9B%C5%A1%2B%C3%AD%C3%A1+das+%2B%2F+%5C+%C4%91";

    QString html = "<form name='form1' method='post' action='foo.php'>"
                   "<input id='id1' type='text' name='user' value='xx'>"
                   "<input id='id2' type='password' name='pass' value='xx'>"
                   "<input type='submit' value='submit' name='submit'>"
                   "</form>";

    completeWithData(html, data);

    QCOMPARE(getElementByIdValue("id1").toString(), QString::fromUtf8("+ěšS ˇ - %20 / aa_"));
    QCOMPARE(getElementByIdValue("id2").toString(), QString::fromUtf8("ýžěš+íá das +/ \\ đ"));
}

void FormCompleterTest::extractFormTest1()
{
    // Basic test

    QByteArray data = "username=tst_username&password=tst_password";

    QString html = "<form name='form1' method='post' action='foo.php'>"
                   "<input id='id1' type='text' name='username' value='tst_username'>"
                   "<input id='id2' type='password' name='password' value='tst_password'>"
                   "<input type='submit' value='submit' name='submit'>"
                   "</form>";

    PageFormData form = extractFormData(html, data);

    QVERIFY(form.found == true);
    QCOMPARE(form.username, QString("tst_username"));
    QCOMPARE(form.password, QString("tst_password"));
}

void FormCompleterTest::extractFormTest2()
{
    // Test special characters (even in input name)

    QByteArray data = "use%C2%B6+_nam%C4%8D=%2B%C4%9B+%2B%2B+%C3%A9%C3%AD%C2%A7%60%5D%7C%7E%C4%9111+%2B%21%3A"
            "&pA+%5DsQ+%2Bword=%2B%C4%9B%C5%A1+asn%7E%C4%91%C2%B0%23%26%23+%7C%E2%82%AC";

    QString html = QString::fromUtf8("<form name='form1' method='post' action='foo.php'>"
                   "<input id='id1' type='text' name='use¶ _namč' value='+ě ++ éí§`]|~đ11 +!:'>"
                   "<input id='id2' type='password' name='pA ]sQ +word' value='+ěš asn~đ°#&# |€'>"
                   "<input type='submit' value='submit' name='submit'>"
                   "</form>");

    PageFormData form = extractFormData(html, data);

    QVERIFY(form.found == true);
    QCOMPARE(form.username, QString::fromUtf8("+ě ++ éí§`]|~đ11 +!:"));
    QCOMPARE(form.password, QString::fromUtf8("+ěš asn~đ°#&# |€"));
}

void FormCompleterTest::extractFormTest3()
{
    // Test detecting sent form between 2 identical forms
    // but only one form is filled with correct data

    QByteArray data = "username=tst_username&password=tst_password";

    QString html = "<form name='form1' method='post' action='foo.php'>"
                   "<input id='id1' type='text' name='username' value='wrong_username'>"
                   "<input id='id2' type='password' name='password' value='wrong_password'>"
                   "<input type='submit' value='submit' name='submit'>"
                   "</form>";

    QString html2 = "<form name='form2' method='post' action='foo2.php'>"
                   "<input id='id3' type='text' name='username' value='tst_username'>"
                   "<input id='id4' type='password' name='password' value='tst_password'>"
                   "<input type='submit' value='submit' name='submit'>"
                   "</form>";

    PageFormData form = extractFormData(html + html2, data);

    QVERIFY(form.found == true);
    QCOMPARE(form.username, QString("tst_username"));
    QCOMPARE(form.password, QString("tst_password"));
}

void FormCompleterTest::extractFormTest4()
{
    // Test extracting form that contains only password
    // as editable input

    QByteArray data = "username=tst_username&password=tst_password";

    QString html = "<form name='form2' method='post' action='foo2.php'>"
                   "<input id='id3' type='hidden' name='username' value='tst_username'>"
                   "<input id='id4' type='password' name='password' value='tst_password'>"
                   "<input type='submit' value='submit' name='submit'>"
                   "</form>";

    PageFormData form = extractFormData(html, data);

    QVERIFY(form.found == true);
    QCOMPARE(form.username, QString());
    QCOMPARE(form.password, QString("tst_password"));
}

void FormCompleterTest::completeWithData(const QString &html, const QByteArray &data)
{
    view->setHtml(html);

    PageFormCompleter completer(view->page());
    completer.completePage(data);
}

PageFormData FormCompleterTest::extractFormData(const QString &html, const QByteArray &data)
{
    view->setHtml(html);

    PageFormCompleter completer(view->page());
    return completer.extractFormData(data);
}

QVariant FormCompleterTest::getElementByIdValue(const QString &id)
{
    QString source = QString("document.getElementById('%1').value").arg(id);
    return view->page()->mainFrame()->evaluateJavaScript(source);
}
