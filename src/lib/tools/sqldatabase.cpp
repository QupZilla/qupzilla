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

#include <QMetaMethod>

#define CONNECTION_NAME QSL("QupZilla::DatabaseWorker")

SqlDatabase* SqlDatabase::s_instance = 0;

// SqlDatabase
SqlDatabase::SqlDatabase(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<QSqlQuery>("QSqlQuery");

    m_worker = new DatabaseWorker;
    m_thread = new DatabaseWorkerThread(m_worker);
    m_worker->moveToThread(m_thread);
    m_thread->start();

    connect(m_thread, SIGNAL(started()), m_worker, SLOT(threadStarted()));
}

SqlDatabase::~SqlDatabase()
{
    m_thread->exit();
    m_thread->wait();

    delete m_thread;
    delete m_worker;
}

void SqlDatabase::execAsync(const QSqlQuery &query, QObject* receiver, const char* slot)
{
    m_worker->execQueryAsync(query, receiver, slot);
}

void SqlDatabase::transactionAsync(const QList<QSqlQuery> &queries)
{
    m_worker->transactionAsync(queries);
}

SqlDatabase* SqlDatabase::instance()
{
    if (!s_instance) {
        s_instance = new SqlDatabase;
    }
    return s_instance;
}

void SqlDatabase::destroy()
{
    delete s_instance;
    s_instance = 0;
}

// DatabaseWorker
DatabaseWorker::DatabaseWorker()
    : QObject()
    , m_started(false)
{
}

void DatabaseWorker::execQueryAsync(const QSqlQuery &query, QObject* receiver, const char* slot)
{
    QueryData data;
    data.queries.append(query);
    data.receiver = receiver;
    data.slot = slot;

    m_queries.enqueue(data);

    if (m_started) {
        QMetaObject::invokeMethod(this, "execPendingQueries", Qt::QueuedConnection);
    }
}

void DatabaseWorker::transactionAsync(const QList<QSqlQuery> &queries)
{
    QueryData data;
    data.queries = queries;
    data.receiver = 0;
    data.slot = 0;

    m_queries.enqueue(data);

    if (m_started) {
        QMetaObject::invokeMethod(this, "execPendingQueries", Qt::QueuedConnection);
    }
}

bool DatabaseWorker::hasPendingQueries() const
{
    return !m_queries.isEmpty();
}

void DatabaseWorker::threadStarted()
{
    m_started = true;

    QSqlDatabase::cloneDatabase(QSqlDatabase::database(), CONNECTION_NAME);
    m_db = QSqlDatabase::database(CONNECTION_NAME);

    // Execute queries that got queued before starting thread
    if (hasPendingQueries()) {
        QMetaObject::invokeMethod(this, "execPendingQueries", Qt::QueuedConnection);
    }
}

static QSqlQuery copyQueryToDatabase(const QSqlQuery &query, const QSqlDatabase &db)
{
    QSqlQuery out(db);
    out.prepare(query.lastQuery());

    const QList<QVariant> boundValues = query.boundValues().values();

    foreach (const QVariant &variant, boundValues) {
        out.addBindValue(variant);
    }

    return out;
}

void DatabaseWorker::execPendingQueries()
{
    while (hasPendingQueries()) {
        QueryData data = m_queries.dequeue();

        Q_ASSERT(!data.queries.isEmpty());

        // Transaction
        if (data.queries.size() > 1) {
            m_db.transaction();
            foreach (const QSqlQuery &q, data.queries) {
                QSqlQuery query = copyQueryToDatabase(q, m_db);
                query.exec();
            }
            m_db.commit();
        }
        // Single query
        else {
            QSqlQuery query = copyQueryToDatabase(data.queries.takeFirst(), m_db);
            query.exec();

            // Invoke connected slot
            if (data.receiver) {
                // SLOT() macro is prepending a "1" to the slot signature
                int index = data.receiver->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(data.slot + 1));
                Q_ASSERT(index >= 0);
                QMetaMethod method = data.receiver->metaObject()->method(index);
                method.invoke(data.receiver, Qt::QueuedConnection, Q_ARG(QSqlQuery, query));
            }
        }
    }
}

// DatabaseWorkerThread
DatabaseWorkerThread::DatabaseWorkerThread(DatabaseWorker* worker)
    : QThread()
    , m_worker(worker)
{
}

void DatabaseWorkerThread::run()
{
    exec();

    if (m_worker->hasPendingQueries()) {
        m_worker->execPendingQueries();
    }
}
