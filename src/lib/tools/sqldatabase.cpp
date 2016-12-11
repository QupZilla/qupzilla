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
#include "sqldatabase.h"

#include <QThread>
#include <QMutexLocker>
#include <QApplication>

#include <QtConcurrent/QtConcurrentRun>

Q_GLOBAL_STATIC(SqlDatabase, qz_sql_database)

// SqlDatabase
SqlDatabase::SqlDatabase(QObject* parent)
    : QObject(parent)
{
}

SqlDatabase::~SqlDatabase()
{
    QMutableHashIterator<QThread*, QSqlDatabase> i(m_databases);
    while (i.hasNext()) {
        i.next();
        i.value().close();
    }
}

QSqlDatabase SqlDatabase::databaseForThread(QThread* thread)
{
    QMutexLocker lock(&m_mutex);

    if (!m_databases.contains(thread)) {
        const QString threadStr = QString::number((quintptr) thread);
        m_databases[thread] = QSqlDatabase::cloneDatabase(QSqlDatabase::database(), QL1S("QupZilla/") + threadStr);
        m_databases[thread].open();
    }

    Q_ASSERT(m_databases[thread].isOpen());

    return m_databases[thread];
}

QSqlQuery SqlDatabase::exec(QSqlQuery &query)
{
    if (QThread::currentThread() == qApp->thread()) {
        query.exec();
        return query;
    }

    QSqlQuery out(databaseForThread(QThread::currentThread()));
    out.prepare(query.lastQuery());

    const QList<QVariant> boundValues = query.boundValues().values();

    foreach (const QVariant &variant, boundValues) {
        out.addBindValue(variant);
    }

    out.exec();
    return out;
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
