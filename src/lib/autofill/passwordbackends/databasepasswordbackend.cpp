/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2013-2017 David Rosca <nowrep@gmail.com>
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
#include "autofill.h"
#include "sqldatabase.h"

#include <QVector>

DatabasePasswordBackend::DatabasePasswordBackend()
    : PasswordBackend()
{
}

QString DatabasePasswordBackend::name() const
{
    return AutoFill::tr("Database (plaintext)");
}

QVector<PasswordEntry> DatabasePasswordBackend::getEntries(const QUrl &url)
{
    const QString host = PasswordManager::createHost(url);

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT id, username, password, data FROM autofill "
                  "WHERE server=? ORDER BY last_used DESC");
    query.addBindValue(host);
    query.exec();

    QVector<PasswordEntry> list;

    while (query.next()) {
        PasswordEntry data;
        data.id = query.value(0);
        data.host = host;
        data.username = query.value(1).toString();
        data.password = query.value(2).toString();
        data.data = query.value(3).toByteArray();

        list.append(data);
    }

    return list;
}

QVector<PasswordEntry> DatabasePasswordBackend::getAllEntries()
{
    QVector<PasswordEntry> list;

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT id, server, username, password, data FROM autofill");
    query.exec();

    while (query.next()) {
        PasswordEntry data;
        data.id = query.value(0);
        data.host = query.value(1).toString();
        data.username = query.value(2).toString();
        data.password = query.value(3).toString();
        data.data = query.value(4).toByteArray();

        list.append(data);
    }

    return list;
}

void DatabasePasswordBackend::addEntry(const PasswordEntry &entry)
{
    // Data is empty only for HTTP/FTP authorization
    if (entry.data.isEmpty()) {
        // Multiple-usernames for HTTP/FTP authorization not supported
        QSqlQuery query(SqlDatabase::instance()->database());
        query.prepare("SELECT username FROM autofill WHERE server=?");
        query.addBindValue(entry.host);
        query.exec();

        if (query.next()) {
            return;
        }
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("INSERT INTO autofill (server, data, username, password, last_used) "
                  "VALUES (?,?,?,?,strftime('%s', 'now'))");
    query.bindValue(0, entry.host);
    query.bindValue(1, entry.data);
    query.bindValue(2, entry.username);
    query.bindValue(3, entry.password);
    query.exec();
}

bool DatabasePasswordBackend::updateEntry(const PasswordEntry &entry)
{
    QSqlQuery query(SqlDatabase::instance()->database());

    // Data is empty only for HTTP/FTP authorization
    if (entry.data.isEmpty()) {
        query.prepare("UPDATE autofill SET username=?, password=? WHERE server=?");
        query.bindValue(0, entry.username);
        query.bindValue(1, entry.password);
        query.bindValue(2, entry.host);
    }
    else {
        query.prepare("UPDATE autofill SET data=?, username=?, password=? WHERE id=?");
        query.addBindValue(entry.data);
        query.addBindValue(entry.username);
        query.addBindValue(entry.password);
        query.addBindValue(entry.id);
    }

    return query.exec();
}

void DatabasePasswordBackend::updateLastUsed(PasswordEntry &entry)
{
    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("UPDATE autofill SET last_used=strftime('%s', 'now') WHERE id=?");
    query.addBindValue(entry.id);
    query.exec();
}

void DatabasePasswordBackend::removeEntry(const PasswordEntry &entry)
{
    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("DELETE FROM autofill WHERE id=?");
    query.addBindValue(entry.id);
    query.exec();
}

void DatabasePasswordBackend::removeAll()
{
    QSqlQuery query(SqlDatabase::instance()->database());
    query.exec("DELETE FROM autofill");
}
