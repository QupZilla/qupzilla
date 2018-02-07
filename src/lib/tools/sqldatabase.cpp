/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2018 David Rosca <nowrep@gmail.com>
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
#include "sqldatabase.h"

#include <QApplication>
#include <QThreadStorage>

#include <QtConcurrent/QtConcurrentRun>

QThreadStorage<QSqlDatabase> s_databases;

Q_GLOBAL_STATIC(SqlDatabase, qz_sql_database)

// SqlDatabase
SqlDatabase::SqlDatabase(QObject* parent)
    : QObject(parent)
{
}

SqlDatabase::~SqlDatabase()
{
}

QSqlDatabase SqlDatabase::database()
{
    if (QThread::currentThread() == qApp->thread()) {
        return QSqlDatabase::database();
    }

    if (!s_databases.hasLocalData()) {
        const QString threadStr = QString::number((quintptr) QThread::currentThread());
        QSqlDatabase db = QSqlDatabase::addDatabase(QSL("QSQLITE"), threadStr);
        db.setDatabaseName(m_databaseName);
        db.setConnectOptions(m_connectOptions);
        db.open();
        s_databases.setLocalData(db);
    }

    return s_databases.localData();
}

void SqlDatabase::setDatabase(const QSqlDatabase &database)
{
    m_databaseName = database.databaseName();
    m_connectOptions = database.connectOptions();
}

// instance
SqlDatabase* SqlDatabase::instance()
{
    return qz_sql_database();
}
