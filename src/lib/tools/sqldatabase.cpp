/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2017 David Rosca <nowrep@gmail.com>
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
        QSqlDatabase db = QSqlDatabase::cloneDatabase(QSqlDatabase::database(), QL1S("QupZilla/") + threadStr);
        db.open();
        s_databases.setLocalData(db);
    }

    qDebug() << "For" << QThread::currentThread() << "got" << s_databases.localData().connectionName();

    return s_databases.localData();
}

QSqlQuery SqlDatabase::exec(QSqlQuery &query)
{
    QSqlQuery out(database());
    out.prepare(query.lastQuery());

    const QList<QVariant> boundValues = query.boundValues().values();

    foreach (const QVariant &variant, boundValues) {
        out.addBindValue(variant);
    }

    out.exec();
    return query = out;
}

QFuture<QSqlQuery> SqlDatabase::execAsync(const QSqlQuery &query)
{
    return QtConcurrent::run(this, &SqlDatabase::exec, query);
}

// instance
SqlDatabase* SqlDatabase::instance()
{
    return qz_sql_database();
}
