/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#include "autofillmodel.h"
#include "qupzilla.h"
#include "webview.h"
#include "mainapplication.h"
#include "autofillnotification.h"
#include "databasewriter.h"

AutoFillModel::AutoFillModel(QupZilla* mainClass, QObject* parent)
    : QObject(parent)
    , p_QupZilla(mainClass)
    , m_isStoring(false)
{
    QTimer::singleShot(0, this, SLOT(loadSettings()));
}

void AutoFillModel::loadSettings()
{
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    m_isStoring = settings.value("SavePasswordsOnSites", true).toBool();
    settings.endGroup();
}

bool AutoFillModel::isStored(const QUrl &url)
{
    if (!isStoringEnabled(url)) {
        return false;
    }

    QString server = url.host();
    QSqlQuery query;
    query.exec("SELECT count(id) FROM autofill WHERE server='" + server + "'");
    query.next();
    if (query.value(0).toInt() > 0) {
        return true;
    }
    return false;
}

bool AutoFillModel::isStoringEnabled(const QUrl &url)
{
    if (!m_isStoring) {
        return false;
    }

    QString server = url.host();
    QSqlQuery query;
    query.exec("SELECT count(id) FROM autofill_exceptions WHERE server='" + server + "'");
    query.next();
    if (query.value(0).toInt() > 0) {
        return false;
    }
    return true;
}

void AutoFillModel::blockStoringfor(const QUrl &url)
{
    QString server = url.host();
    QSqlQuery query;
    query.prepare("INSERT INTO autofill_exceptions (server) VALUES (?)");
    query.addBindValue(server);
    mApp->dbWriter()->executeQuery(query);
}

QString AutoFillModel::getUsername(const QUrl &url)
{
    QString server = url.host();
    QSqlQuery query;
    query.exec("SELECT username FROM autofill WHERE server='" + server + "'");
    query.next();
    return query.value(0).toString();
}

QString AutoFillModel::getPassword(const QUrl &url)
{
    QString server = url.host();
    QSqlQuery query;
    query.exec("SELECT password FROM autofill WHERE server='" + server + "'");
    query.next();
    return query.value(0).toString();
}

///HTTP Authorization
void AutoFillModel::addEntry(const QUrl &url, const QString &name, const QString &pass)
{
    QSqlQuery query;
    query.exec("SELECT username FROM autofill WHERE server='" + url.host() + "'");
    if (query.next()) {
        return;
    }
    query.prepare("INSERT INTO autofill (server, username, password) VALUES (?,?,?)");
    query.bindValue(0, url.host());
    query.bindValue(1, name);
    query.bindValue(2, pass);
    mApp->dbWriter()->executeQuery(query);
}

///WEB Form
void AutoFillModel::addEntry(const QUrl &url, const QByteArray &data, const QString &user, const QString &pass)
{
    QSqlQuery query;
    query.exec("SELECT data FROM autofill WHERE server='" + url.host() + "'");
    if (query.next()) {
        return;
    }

    query.prepare("INSERT INTO autofill (server, data, username, password) VALUES (?,?,?,?)");
    query.bindValue(0, url.host());
    query.bindValue(1, data);
    query.bindValue(2, user);
    query.bindValue(3, pass);
    mApp->dbWriter()->executeQuery(query);
}

void AutoFillModel::completePage(WebView* view)
{
    if (!isStored(view->url())) {
        return;
    }

    QWebElementCollection inputs;
    QList<QWebFrame*> frames;
    frames.append(view->page()->mainFrame());
    while (!frames.isEmpty()) {
        QWebFrame* frame = frames.takeFirst();
        inputs.append(frame->findAllElements("input"));
        frames += frame->childFrames();
    }

    QSqlQuery query;
    query.exec("SELECT data FROM autofill WHERE server='" + view->url().host() + "'");
    query.next();
    QByteArray data = query.value(0).toByteArray();
    if (data.isEmpty()) {
        return;
    }

    QList<QPair<QString, QString> > arguments = QUrl::fromEncoded(QByteArray("http://bla.com/?" + data)).queryItems();
    for (int i = 0; i < arguments.count(); i++) {
        QString key = QUrl::fromEncoded(arguments.at(i).first.toUtf8()).toString();
        QString value = QUrl::fromEncoded(arguments.at(i).second.toUtf8()).toString();
        //key.replace("+"," ");
        //value.replace("+"," ");

        for (int i = 0; i < inputs.count(); i++) {
            QWebElement element = inputs.at(i);

            if (element.attribute("type") != "text" && element.attribute("type") != "password" && element.attribute("type") != "") {
                continue;
            }
            if (key == element.attribute("name")) {
                element.setAttribute("value", value);
            }
        }
    }
}

void AutoFillModel::post(const QNetworkRequest &request, const QByteArray &outgoingData)
{
    //Dont save in private browsing
    if (mApp->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled)) {
        return;
    }

    m_lastOutgoingData = outgoingData;

    QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
    QWebPage* webPage = (QWebPage*)(v.value<void*>());
    v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 102));
    WebView* webView = (WebView*)(v.value<void*>());
    if (!webPage || !webView) {
        return;
    }

    v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 101));
    QWebPage::NavigationType type = (QWebPage::NavigationType)v.toInt();

    if (type != QWebPage::NavigationTypeFormSubmitted) {
        return;
    }

    QString usernameName;
    QString usernameValue;
    QString passwordName;
    QString passwordValue;
    QUrl siteUrl = webView->url();

    if (!isStoringEnabled(siteUrl)) {
        return;
    }

    QWebElementCollection allForms; // All form elements on page
    QWebElement foundForm;          // Sent form element

    QList<QWebFrame*> frames;
    frames.append(webPage->mainFrame());  // Find all form elements
    while (!frames.isEmpty()) {
        QWebFrame* frame = frames.takeFirst();
        allForms.append(frame->findAllElements("form"));
        frames += frame->childFrames();
    }

    foreach(QWebElement formElement, allForms) {
        foreach(QWebElement inputElement, formElement.findAll("input[type=\"password\"]")) {
            passwordName = inputElement.attribute("name");
            passwordValue = getValueFromData(outgoingData, inputElement);

            if (!passwordValue.isEmpty() && dataContains(outgoingData, passwordName)) {
                foundForm = formElement;
                break;
            }
        }

        if (!foundForm.isNull()) {
            break;
        }
    }

    // Return if data for this page is already stored or no password element found in sent data
    if (foundForm.isNull() || isStored(siteUrl)) {
        return;
    }

    // We need to find username, we suppose that username is first not empty input[type=text] in form
    // Tell me better solution. Maybe first try to find name="user", name="username" ?

    foreach(QWebElement element, foundForm.findAll("input[type=\"text\"]")) {
        usernameName = element.attribute("name");
        usernameValue = getValueFromData(outgoingData, element);
        if (!usernameName.isEmpty() && !usernameValue.isEmpty()) {
            break;
        }
    }

    AutoFillNotification* aWidget = new AutoFillNotification(siteUrl, outgoingData, usernameValue, passwordValue);
    webView->addNotification(aWidget);
}

QString AutoFillModel::getValueFromData(const QByteArray &data, QWebElement element)
{
    QString name = element.attribute("name");
    if (name.isEmpty()) {
        return "";
    }

    QString value = element.evaluateJavaScript("this.value").toString();
    if (value.isEmpty()) {
        QueryItems queryItems = QUrl::fromEncoded("http://a.b/?" + data).queryItems();
        for (int i = 0; i < queryItems.count(); i++) {
            QueryItem item = queryItems.at(i);

            if (item.first == name) {
                value = item.second.toUtf8();
            }
        }
    }

    return value;
}

bool AutoFillModel::dataContains(const QByteArray &data, const QString &attributeName)
{
    QueryItems queryItems = QUrl::fromEncoded("http://a.b/?" + data).queryItems();
    for (int i = 0; i < queryItems.count(); i++) {
        QueryItem item = queryItems.at(i);

        if (item.first == attributeName) {
            return !item.second.isEmpty();
        }
    }

    return false;
}
