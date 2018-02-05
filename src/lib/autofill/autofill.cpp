/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "browserwindow.h"
#include "webpage.h"
#include "sqldatabase.h"
#include "popupwebview.h"
#include "mainapplication.h"
#include "autofillnotification.h"
#include "settings.h"
#include "passwordmanager.h"
#include "qztools.h"
#include "scripts.h"
#include "webpage.h"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>
#include <QUrlQuery>

AutoFill::AutoFill(QObject* parent)
    : QObject(parent)
    , m_manager(new PasswordManager(this))
{
    loadSettings();

    // Setup AutoFill userscript
    QWebEngineScript script;
    script.setName(QSL("_qupzilla_autofill"));
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(WebPage::SafeJsWorld);
    script.setRunsOnSubFrames(true);
    script.setSourceCode(Scripts::setupFormObserver());
    mApp->webProfile()->scripts()->insert(script);
}

PasswordManager* AutoFill::passwordManager() const
{
    return m_manager;
}

void AutoFill::loadSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    m_isStoring = settings.value("SavePasswordsOnSites", true).toBool();
    m_isAutoComplete = settings.value("AutoCompletePasswords", true).toBool();
    settings.endGroup();
}

bool AutoFill::isStored(const QUrl &url)
{
    if (!isStoringEnabled(url)) {
        return false;
    }

    return !m_manager->getUsernames(url).isEmpty();
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

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT count(id) FROM autofill_exceptions WHERE server=?");
    query.addBindValue(server);
    query.exec();

    if (!query.next()) {
        return false;
    }

    return query.value(0).toInt() <= 0;
}

void AutoFill::blockStoringforUrl(const QUrl &url)
{
    QString server = url.host();
    if (server.isEmpty()) {
        server = url.toString();
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("INSERT INTO autofill_exceptions (server) VALUES (?)");
    query.addBindValue(server);
    query.exec();
}

QVector<PasswordEntry> AutoFill::getFormData(const QUrl &url)
{
    return m_manager->getEntries(url);
}

QVector<PasswordEntry> AutoFill::getAllFormData()
{
    return m_manager->getAllEntries();
}

void AutoFill::updateLastUsed(PasswordEntry &data)
{
    m_manager->updateLastUsed(data);
}

// HTTP Authorization
void AutoFill::addEntry(const QUrl &url, const QString &name, const QString &pass)
{
    PasswordEntry entry;
    entry.host = PasswordManager::createHost(url);
    entry.username = name;
    entry.password = pass;

    m_manager->addEntry(entry);
}

// WEB Form
void AutoFill::addEntry(const QUrl &url, const PageFormData &formData)
{
    PasswordEntry entry;
    entry.host = PasswordManager::createHost(url);
    entry.username = formData.username;
    entry.password = formData.password;
    entry.data = formData.postData;

    m_manager->addEntry(entry);
}

// HTTP Authorization
void AutoFill::updateEntry(const QUrl &url, const QString &name, const QString &pass)
{
    PasswordEntry entry;
    entry.host = PasswordManager::createHost(url);
    entry.username = name;
    entry.password = pass;

    m_manager->updateEntry(entry);
}

// WEB Form
bool AutoFill::updateEntry(const PasswordEntry &entry)
{
    return m_manager->updateEntry(entry);
}

void AutoFill::removeEntry(const PasswordEntry &entry)
{
    m_manager->removeEntry(entry);
}

void AutoFill::removeAllEntries()
{
    m_manager->removeAllEntries();
}

void AutoFill::saveForm(WebPage *page, const QUrl &frameUrl, const PageFormData &formData)
{
    // Don't save in private browsing
    if (mApp->isPrivate() || !page)
        return;

    if (!isStoringEnabled(frameUrl))
        return;

    PasswordEntry updateData;

    if (isStored(frameUrl)) {
        const QVector<PasswordEntry> &list = getFormData(frameUrl);

        foreach (const PasswordEntry &data, list) {
            if (data.username == formData.username) {
                updateData = data;
                updateLastUsed(updateData);

                if (data.password == formData.password) {
                    updateData.password.clear();
                    return;
                }

                updateData.username = formData.username;
                updateData.password = formData.password;
                updateData.data = formData.postData;
                break;
            }
        }
    }

    if (m_lastNotification && m_lastNotificationPage == page) {
        m_lastNotification->close();
    }

    AutoFillNotification* aWidget = new AutoFillNotification(frameUrl, formData, updateData);
    page->view()->addNotification(aWidget);

    m_lastNotification = aWidget;
    m_lastNotificationPage = page;
}

// Returns all saved passwords on this page
QStringList AutoFill::completePage(WebPage *page, const QUrl &frameUrl)
{
    QStringList usernames;

    if (!page || !isStored(frameUrl))
        return usernames;

    if (!m_isAutoComplete) {
        return m_manager->getUsernames(frameUrl);
    }

    const auto entries = getFormData(frameUrl);

    if (!entries.isEmpty()) {
        PasswordEntry entry = entries.at(0);
        updateLastUsed(entry);
        page->runJavaScript(Scripts::completeFormData(entry.data), WebPage::SafeJsWorld);
    }

    usernames.reserve(entries.size());
    for (const PasswordEntry &entry : entries) {
        usernames.append(entry.username);
    }
    return usernames;
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

    QVector<PasswordEntry> entries = m_manager->getAllEntries();

    foreach (const PasswordEntry &entry, entries) {
        stream.writeStartElement("entry");
        stream.writeTextElement("server", entry.host);
        stream.writeTextElement("username", entry.username);
        stream.writeTextElement("password", entry.password);
        stream.writeTextElement("data", entry.data);
        stream.writeEndElement();
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT server FROM autofill_exceptions");
    query.exec();
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
                PasswordEntry entry;

                while (xml.readNext()) {
                    if (xml.name() == QLatin1String("server")) {
                        entry.host = xml.readElementText();
                    }
                    else if (xml.name() == QLatin1String("username")) {
                        entry.username = xml.readElementText();
                    }
                    else if (xml.name() == QLatin1String("password")) {
                        entry.password = xml.readElementText();
                    }
                    else if (xml.name() == QLatin1String("data")) {
                        entry.data = xml.readElementText().toUtf8();
                    }

                    if (xml.isEndElement() && xml.name() == QLatin1String("entry")) {
                        break;
                    }
                }

                if (entry.isValid()) {
                    bool containsEntry = false;

                    foreach (const PasswordEntry &e, m_manager->getEntries(QUrl(entry.host))) {
                        if (e.username == entry.username) {
                            containsEntry = true;
                            break;
                        }
                    }

                    if (!containsEntry) {
                        m_manager->addEntry(entry);
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
                    QSqlQuery query(SqlDatabase::instance()->database());
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
