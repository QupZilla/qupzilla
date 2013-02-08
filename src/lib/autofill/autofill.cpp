/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "autofill.h"
#include "qupzilla.h"
#include "webpage.h"
#include "tabbedwebview.h"
#include "popupwebview.h"
#include "mainapplication.h"
#include "autofillnotification.h"
#include "pageformcompleter.h"
#include "databasewriter.h"
#include "settings.h"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QWebFrame>
#include <QNetworkRequest>

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

AutoFill::AutoFill(QupZilla* mainClass, QObject* parent)
    : QObject(parent)
    , p_QupZilla(mainClass)
    , m_isStoring(false)
{
    loadSettings();
}

void AutoFill::loadSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    m_isStoring = settings.value("SavePasswordsOnSites", true).toBool();
    settings.endGroup();
}

bool AutoFill::isStored(const QUrl &url)
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

bool AutoFill::isStoringEnabled(const QUrl &url)
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

void AutoFill::blockStoringforUrl(const QUrl &url)
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

AutoFillData AutoFill::getFirstFormData(const QUrl &url)
{
    QList<AutoFillData> list = getFormData(url, 1);

    if (list.isEmpty()) {
        AutoFillData data;
        data.id = -1;

        return data;
    }

    return list.first();
}

QList<AutoFillData> AutoFill::getFormData(const QUrl &url, int limit)
{
    QList<AutoFillData> list;

    QString server = url.host();
    if (server.isEmpty()) {
        server = url.toString();
    }

    QString queryString = "SELECT id, username, password, data FROM autofill "
                          "WHERE server=? ORDER BY last_used DESC";
    if (limit > 0) {
        queryString.append(QLatin1String(" LIMIT ?"));
    }

    QSqlQuery query;
    query.prepare(queryString);
    query.addBindValue(server);

    if (limit > 0) {
        query.addBindValue(limit);
    }

    query.exec();

    while (query.next()) {
        AutoFillData data;
        data.id = query.value(0).toInt();
        data.username = query.value(1).toString();
        data.password = query.value(2).toString();
        data.postData = query.value(3).toByteArray();

        list.append(data);
    }

    return list;

}

void AutoFill::updateLastUsed(int id)
{
    if (id < 0) {
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE autofill SET last_used=strftime('%s', 'now') WHERE id=?");
    query.addBindValue(id);
    query.exec();
}

///HTTP Authorization
void AutoFill::addEntry(const QUrl &url, const QString &name, const QString &pass)
{
    QSqlQuery query;
    QString server = url.host();
    if (server.isEmpty()) {
        server = url.toString();
    }

    // Multiple-usernames for HTTP Authorization not supported
    query.prepare("SELECT username FROM autofill WHERE server=?");
    query.addBindValue(server);
    query.exec();

    if (query.next()) {
        return;
    }

    query.prepare("INSERT INTO autofill (server, username, password, last_used) "
                  "VALUES (?,?,?,strftime('%s', 'now'))");
    query.bindValue(0, server);
    query.bindValue(1, name);
    query.bindValue(2, pass);
    mApp->dbWriter()->executeQuery(query);
}

///WEB Form
void AutoFill::addEntry(const QUrl &url, const PageFormData &formData)
{
    QString server = url.host();
    if (server.isEmpty()) {
        server = url.toString();
    }

    QSqlQuery query;
    query.prepare("INSERT INTO autofill (server, data, username, password, last_used) "
                  "VALUES (?,?,?,?,strftime('%s', 'now'))");
    query.bindValue(0, server);
    query.bindValue(1, formData.postData);
    query.bindValue(2, formData.username);
    query.bindValue(3, formData.password);
    mApp->dbWriter()->executeQuery(query);
}

void AutoFill::updateEntry(const QUrl &url, const QString &name, const QString &pass)
{
    QSqlQuery query;
    QString server = url.host();
    if (server.isEmpty()) {
        server = url.toString();
    }

    query.prepare("SELECT username FROM autofill WHERE server=?");
    query.addBindValue(server);
    query.exec();

    if (!query.next()) {
        return;
    }

    query.prepare("UPDATE autofill SET username=?, password=? WHERE server=?");
    query.bindValue(0, name);
    query.bindValue(1, pass);
    query.bindValue(2, server);
    mApp->dbWriter()->executeQuery(query);
}

void AutoFill::updateEntry(const PageFormData &formData, const AutoFillData &updateData)
{
    QSqlQuery query;
    query.prepare("UPDATE autofill SET data=?, username=?, password=? WHERE id=?");
    query.addBindValue(formData.postData);
    query.addBindValue(formData.username);
    query.addBindValue(formData.password);
    query.addBindValue(updateData.id);

    mApp->dbWriter()->executeQuery(query);
}

QList<AutoFillData> AutoFill::completePage(WebPage* page)
{
    QList<AutoFillData> list;

    if (!page) {
        return list;
    }

    QUrl pageUrl = page->url();
    if (!isStored(pageUrl)) {
        return list;
    }

    list = getFormData(pageUrl);

    if (!list.isEmpty()) {
        const AutoFillData data = getFirstFormData(pageUrl);

        PageFormCompleter completer(page);
        completer.completePage(data.postData);
    }

    return list;
}

void AutoFill::post(const QNetworkRequest &request, const QByteArray &outgoingData)
{
    // Don't save in private browsing
    if (mApp->isPrivateSession()) {
        return;
    }

    QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
    WebPage* webPage = static_cast<WebPage*>(v.value<void*>());
    if (!WebPage::isPointerSafeToUse(webPage)) {
        return;
    }
    WebView* webView = qobject_cast<WebView*>(webPage->view());
    if (!webView) {
        return;
    }

    const QUrl &siteUrl = webPage->url();

    if (!isStoringEnabled(siteUrl)) {
        return;
    }

    PageFormCompleter completer(webPage);
    const PageFormData formData = completer.extractFormData(outgoingData);

    if (!formData.found) {
        return;
    }

    AutoFillData updateData = { -1, QString(), QString(), QByteArray() };

    if (isStored(siteUrl)) {
        const QList<AutoFillData> &list = getFormData(siteUrl);

        foreach(const AutoFillData & data, list) {
            if (data.username == formData.username) {
                updateLastUsed(data.id);

                if (data.password == formData.password) {
                    return;
                }

                updateData = data;
                break;
            }
        }
    }

    AutoFillNotification* aWidget = new AutoFillNotification(siteUrl, formData, updateData);
    webView->addNotification(aWidget);
}

QByteArray AutoFill::exportPasswords()
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

bool AutoFill::importPasswords(const QByteArray &data)
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
