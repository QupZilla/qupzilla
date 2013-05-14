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
#include "databasepasswordbackend.h"
#include "mainapplication.h"
#include "databasewriter.h"

#include <QVector>
#include <QSqlQuery>

DatabasePasswordBackend::DatabasePasswordBackend()
{
}

QVector<PasswordEntry> DatabasePasswordBackend::getEntries(const QUrl &url)
{
    QVector<PasswordEntry> list;

    QString server = url.host();
    if (server.isEmpty()) {
        server = url.toString();
    }

    QString query = "SELECT id, username, password, data FROM autofill "
                    "WHERE server=? ORDER BY last_used DESC";

    QSqlQuery sqlQuery;
    sqlQuery.prepare(query);
    sqlQuery.addBindValue(server);
    sqlQuery.exec();

    while (sqlQuery.next()) {
        PasswordEntry data;
        data.id = sqlQuery.value(0);
        data.url = url;
        data.username = sqlQuery.value(1).toString();
        data.password = sqlQuery.value(2).toString();
        data.data = sqlQuery.value(3).toByteArray();

        list.append(data);
    }

    return list;
}

void DatabasePasswordBackend::addEntry(const PasswordEntry &entry)
{
    QString server = entry.url.host();
    if (server.isEmpty()) {
        server = entry.url.toString();
    }

    // Data is empty only for HTTP/FTP authorization
    if (entry.data.isEmpty()) {
        // Multiple-usernames for HTTP/FTP authorization not supported
        QSqlQuery query;
        query.prepare("SELECT username FROM autofill WHERE server=?");
        query.addBindValue(server);
        query.exec();

        if (query.next()) {
            return;
        }
    }

    QSqlQuery query;
    query.prepare("INSERT INTO autofill (server, data, username, password, last_used) "
                  "VALUES (?,?,?,?,strftime('%s', 'now'))");
    query.bindValue(0, server);
    query.bindValue(1, entry.data);
    query.bindValue(2, entry.username);
    query.bindValue(3, entry.password);

    mApp->dbWriter()->executeQuery(query);
}

void DatabasePasswordBackend::updateEntry(const PasswordEntry &entry)
{
    QSqlQuery query;

    // Data is empty only for HTTP/FTP authorization
    if (entry.data.isEmpty()) {
        QString server = entry.url.host();
        if (server.isEmpty()) {
            server = entry.url.toString();
        }

        query.prepare("UPDATE autofill SET username=?, password=? WHERE server=?");
        query.bindValue(0, entry.username);
        query.bindValue(1, entry.password);
        query.bindValue(2, server);
    }
    else {
        query.prepare("UPDATE autofill SET data=?, username=?, password=? WHERE id=?");
        query.addBindValue(entry.data);
        query.addBindValue(entry.username);
        query.addBindValue(entry.password);
        query.addBindValue(entry.id);
    }

    mApp->dbWriter()->executeQuery(query);
}

void DatabasePasswordBackend::updateLastUsed(const PasswordEntry &entry)
{
    QSqlQuery query;
    query.prepare("UPDATE autofill SET last_used=strftime('%s', 'now') WHERE id=?");
    query.addBindValue(entry.id);

    mApp->dbWriter()->executeQuery(query);
}
