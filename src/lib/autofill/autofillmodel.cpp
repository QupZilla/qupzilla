/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "webpage.h"
#include "tabbedwebview.h"
#include "popupwebview.h"
#include "mainapplication.h"
#include "autofillnotification.h"
#include "databasewriter.h"
#include "settings.h"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QWebFrame>
#include <QNetworkRequest>

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

AutoFillModel::AutoFillModel(QupZilla* mainClass, QObject* parent)
    : QObject(parent)
    , p_QupZilla(mainClass)
    , m_isStoring(false)
{
    loadSettings();
}

void AutoFillModel::loadSettings()
{
    Settings settings;
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
    if (server.isEmpty()) {
        server = url.toString();
    }

    QSqlQuery query;
    query.prepare("SELECT count(id) FROM autofill WHERE server=?");
    query.addBindValue(server);
    query.exec();

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
    if (server.isEmpty()) {
        server = url.toString();
    }

    QSqlQuery query;
    query.prepare("SELECT count(id) FROM autofill_exceptions WHERE server=?");
    query.addBindValue(server);
    query.exec();

    query.next();
    if (query.value(0).toInt() > 0) {
        return false;
    }
    return true;
}

void AutoFillModel::blockStoringfor(const QUrl &url)
{
    QString server = url.host();
    if (server.isEmpty()) {
        server = url.toString();
    }

    QSqlQuery query;
    query.prepare("INSERT INTO autofill_exceptions (server) VALUES (?)");
    query.addBindValue(server);
    mApp->dbWriter()->executeQuery(query);
}

QString AutoFillModel::getUsername(const QUrl &url)
{
    QString server = url.host();
    if (server.isEmpty()) {
        server = url.toString();
    }

    QSqlQuery query;
    query.prepare("SELECT username FROM autofill WHERE server=?");
    query.addBindValue(server);
    query.exec();

    query.next();
    return query.value(0).toString();
}

QString AutoFillModel::getPassword(const QUrl &url)
{
    QString server = url.host();
    if (server.isEmpty()) {
        server = url.toString();
    }

    QSqlQuery query;
    query.prepare("SELECT password FROM autofill WHERE server=?");
    query.addBindValue(server);
    query.exec();

    query.next();
    return query.value(0).toString();
}

///HTTP Authorization
void AutoFillModel::addEntry(const QUrl &url, const QString &name, const QString &pass)
{
    QSqlQuery query;
    query.prepare("SELECT username FROM autofill WHERE server=?");
    query.addBindValue(url.host());
    query.exec();

    if (query.next()) {
        return;
    }

    QString server = url.host();
    if (server.isEmpty()) {
        server = url.toString();
    }

    query.prepare("INSERT INTO autofill (server, username, password) VALUES (?,?,?)");
    query.bindValue(0, server);
    query.bindValue(1, name);
    query.bindValue(2, pass);
    mApp->dbWriter()->executeQuery(query);
}

///WEB Form
void AutoFillModel::addEntry(const QUrl &url, const QByteArray &data, const QString &user, const QString &pass)
{
    QSqlQuery query;
    query.prepare("SELECT data FROM autofill WHERE server=?");
    query.addBindValue(url.host());
    query.exec();

    if (query.next()) {
        return;
    }

    QString server = url.host();
    if (server.isEmpty()) {
        server = url.toString();
    }

    query.prepare("INSERT INTO autofill (server, data, username, password) VALUES (?,?,?,?)");
    query.bindValue(0, server);
    query.bindValue(1, data);
    query.bindValue(2, user);
    query.bindValue(3, pass);
    mApp->dbWriter()->executeQuery(query);
}

void AutoFillModel::completePage(WebPage* page)
{
    if (!page) {
        return;
    }

    QUrl pageUrl = page->url();
    if (!isStored(pageUrl)) {
        return;
    }

    QWebElementCollection inputs;
    QList<QWebFrame*> frames;
    frames.append(page->mainFrame());
    while (!frames.isEmpty()) {
        QWebFrame* frame = frames.takeFirst();
        inputs.append(frame->findAllElements("input"));
        frames += frame->childFrames();
    }

    QString server = pageUrl.host();
    if (server.isEmpty()) {
        server = pageUrl.toString();
    }

    QSqlQuery query;
    query.prepare("SELECT data FROM autofill WHERE server=?");
    query.addBindValue(server);
    query.exec();
    query.next();
    QByteArray data = query.value(0).toByteArray();
    if (data.isEmpty()) {
        return;
    }

    // Why not to use encodedQueryItems = QByteArrays ?
    // Because we need to filter "+" characters that must be spaces
    // (not real "+" characters "%2B")
#if QT_VERSION >= 0x050000
    QueryItems arguments = QUrlQuery(QUrl::fromEncoded("http://bla.com/?" + data)).queryItems();
#else
    QueryItems arguments = QUrl::fromEncoded("http://bla.com/?" + data).queryItems();
#endif
    for (int i = 0; i < arguments.count(); i++) {
        QString key = arguments.at(i).first;
        QString value = arguments.at(i).second;
        key.replace(QLatin1Char('+'), QLatin1Char(' '));
        value.replace(QLatin1Char('+'), QLatin1Char(' '));

        key = QUrl::fromEncoded(key.toUtf8()).toString();
        value = QUrl::fromEncoded(value.toUtf8()).toString();

        for (int i = 0; i < inputs.count(); i++) {
            QWebElement element = inputs.at(i);

            if (element.attribute("type") != QLatin1String("text")
                    && element.attribute("type") != QLatin1String("password")
                    && !element.attribute("type").isEmpty()) {
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
    // Don't save in private browsing
    if (mApp->isPrivateSession()) {
        return;
    }

    const QByteArray &data = convertWebKitFormBoundaryIfNecessary(outgoingData);

    QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
    WebPage* webPage = static_cast<WebPage*>(v.value<void*>());
    if (!WebPage::isPointerSafeToUse(webPage)) {
        return;
    }
    WebView* webView = qobject_cast<WebView*>(webPage->view());
    if (!webView) {
        return;
    }

//    v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 101));
//    QWebPage::NavigationType type = (QWebPage::NavigationType)v.toInt();

//    if (type != QWebPage::NavigationTypeFormSubmitted) {
//        return;
//    }

    QString usernameName;
    QString usernameValue;
    QString passwordName;
    QString passwordValue;

    const QUrl &siteUrl = webPage->url();

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

    foreach(const QWebElement & formElement, allForms) {
        foreach(const QWebElement & inputElement, formElement.findAll("input[type=\"password\"]")) {
            passwordName = inputElement.attribute("name");
            passwordValue = getValueFromData(data, inputElement);

            if (!passwordValue.isEmpty() && dataContains(data, passwordName)) {
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

    foreach(const QWebElement & element, foundForm.findAll("input[type=\"text\"]")) {
        usernameName = element.attribute("name");
        usernameValue = getValueFromData(data, element);

        if (!usernameName.isEmpty() && !usernameValue.isEmpty()) {
            break;
        }
    }

    AutoFillNotification* aWidget = new AutoFillNotification(siteUrl, data, usernameValue, passwordValue);
    webView->addNotification(aWidget);
}

QString AutoFillModel::getValueFromData(const QByteArray &data, QWebElement element)
{
    QString name = element.attribute("name");
    if (name.isEmpty()) {
        return QString();
    }

    QString value = element.evaluateJavaScript("this.value").toString();
    if (value.isEmpty()) {
#if QT_VERSION >= 0x050000
        QueryItems queryItems = QUrlQuery(QUrl::fromEncoded("http://a.b/?" + data)).queryItems();
#else
        QueryItems queryItems = QUrl::fromEncoded("http://a.b/?" + data).queryItems();
#endif
        for (int i = 0; i < queryItems.count(); i++) {
            QueryItem item = queryItems.at(i);

            if (item.first == name) {
                value = item.second.toUtf8();
            }
        }
    }

    return value;
}

QByteArray AutoFillModel::convertWebKitFormBoundaryIfNecessary(const QByteArray &data)
{
    /* Sometimes, data are passed in this format:

        ------WebKitFormBoundary0bBp3bFMdGwqanMp
        Content-Disposition: form-data; name="name-of-attribute"

        value-of-attribute
        ------WebKitFormBoundary0bBp3bFMdGwqanMp--

       So this function converts this format into url
    */

    if (!data.contains(QByteArray("------WebKitFormBoundary"))) {
        return data;
    }

    QByteArray formatedData;
    QRegExp rx("name=\"(.*)------WebKitFormBoundary");
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(data, pos)) != -1) {
        QString string = rx.cap(1);
        pos += rx.matchedLength();

        int endOfAttributeName = string.indexOf(QLatin1Char('"'));
        if (endOfAttributeName == -1) {
            continue;
        }

        QString attrName = string.left(endOfAttributeName);
        QString attrValue = string.mid(endOfAttributeName + 1).trimmed().remove(QLatin1Char('\n'));

        if (attrName.isEmpty() || attrValue.isEmpty()) {
            continue;
        }

        formatedData.append(attrName + "=" + attrValue + "&");
    }

    return formatedData;
}

bool AutoFillModel::dataContains(const QByteArray &data, const QString &attributeName)
{
#if QT_VERSION >= 0x050000
    QueryItems queryItems = QUrlQuery(QUrl::fromEncoded("http://a.b/?" + data)).queryItems();
#else
    QueryItems queryItems = QUrl::fromEncoded("http://a.b/?" + data).queryItems();
#endif

    for (int i = 0; i < queryItems.count(); i++) {
        QueryItem item = queryItems.at(i);

        if (item.first == attributeName) {
            return !item.second.isEmpty();
        }
    }

    return false;
}

QByteArray AutoFillModel::exportPasswords()
{
    QByteArray output;

    QXmlStreamWriter stream(&output);
    stream.setCodec("UTF-8");
    stream.setAutoFormatting(true);

    stream.writeStartDocument();
    stream.writeStartElement("passwords");
    stream.writeAttribute("version", "1.0");

    QSqlQuery query;
    query.exec("SELECT server, username, password, data FROM autofill");
    while (query.next()) {
        stream.writeStartElement("entry");
        stream.writeTextElement("server", query.value(0).toString());
        stream.writeTextElement("username", query.value(1).toString());
        stream.writeTextElement("password", query.value(2).toString());
        stream.writeTextElement("data", query.value(3).toString());
        stream.writeEndElement();
    }

    query.exec("SELECT server FROM autofill_exceptions");
    while (query.next()) {
        stream.writeStartElement("exception");
        stream.writeTextElement("server", query.value(0).toString());
        stream.writeEndElement();
    }

    stream.writeEndElement();
    stream.writeEndDocument();

    return output;
}

bool AutoFillModel::importPasswords(const QByteArray &data)
{
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QXmlStreamReader xml(data);

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("entry")) {
                QString server;
                QString username;
                QString password;
                QByteArray data;

                while (xml.readNext()) {
                    if (xml.name() == QLatin1String("server")) {
                        server = xml.readElementText();
                    }
                    else if (xml.name() == QLatin1String("username")) {
                        username = xml.readElementText();
                    }
                    else if (xml.name() == QLatin1String("password")) {
                        password = xml.readElementText();
                    }
                    else if (xml.name() == QLatin1String("data")) {
                        data = xml.readElementText().toUtf8();
                    }

                    if (xml.isEndElement() && xml.name() == QLatin1String("entry")) {
                        break;
                    }
                }

                if (!server.isEmpty() && !password.isEmpty() && !data.isEmpty()) {
                    QSqlQuery query;
                    query.prepare("SELECT id FROM autofill WHERE server=? AND password=? AND data=?");
                    query.addBindValue(server);
                    query.addBindValue(password);
                    query.addBindValue(data);
                    query.exec();

                    if (!query.next()) {
                        query.prepare("INSERT INTO autofill (server, username, password, data) VALUES (?,?,?,?)");
                        query.addBindValue(server);
                        query.addBindValue(username);
                        query.addBindValue(password);
                        query.addBindValue(data);
                        query.exec();
                    }
                }
            }
            else if (xml.name() == QLatin1String("exception")) {
                QString server;

                while (xml.readNext()) {
                    if (xml.name() == QLatin1String("server")) {
                        server = xml.readElementText();
                    }

                    if (xml.isEndElement() && xml.name() == QLatin1String("exception")) {
                        break;
                    }
                }

                if (!server.isEmpty()) {
                    QSqlQuery query;
                    query.prepare("SELECT id FROM autofill_exceptions WHERE server=?");
                    query.addBindValue(server);
                    query.exec();

                    if (!query.next()) {
                        query.prepare("INSERT INTO autofill_exceptions (server) VALUES (?)");
                        query.addBindValue(server);
                        query.exec();
                    }
                }
            }
        }
    }
    db.commit();

    return !xml.hasError();
}
