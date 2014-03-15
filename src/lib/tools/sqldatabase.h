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
#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include <QQueue>
#include <QThread>
#include <QSqlQuery>
#include <QSqlDatabase>

#include "qzcommon.h"

class QThread;
class QSqlQuery;

class DatabaseWorker;
class DatabaseWorkerThread;

// Queries are executed in FIFO order
class QUPZILLA_EXPORT SqlDatabase : public QObject
{
    Q_OBJECT

public:
    explicit SqlDatabase(QObject* parent = 0);
    ~SqlDatabase();

    // Executes query async and send result if receiver is not null.
    // Slot must have "name(QSqlQuery)" signature
    void execAsync(const QSqlQuery &query, QObject* receiver = 0, const char* slot = 0);

    // Executes transaction async without sending result
    void transactionAsync(const QList<QSqlQuery> &queries);

    // May be called only after creating QSqlDatabase connection in main thread
    static SqlDatabase* instance();
    // Must be called before closing main thread QSqlDatabase connection
    static void destroy();

private:
    DatabaseWorker* m_worker;
    DatabaseWorkerThread* m_thread;

    static SqlDatabase* s_instance;
};

class DatabaseWorker : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseWorker();

    void execQueryAsync(const QSqlQuery &query, QObject* receiver = 0, const char* slot = 0);
    void transactionAsync(const QList<QSqlQuery> &queries);

    bool hasPendingQueries() const;

public slots:
    void threadStarted();
    void execPendingQueries();

private:
    struct QueryData {
        QList<QSqlQuery> queries;
        QObject* receiver;
        const char* slot;
    };

    QQueue<QueryData> m_queries;
    QSqlDatabase m_db;
    bool m_started;
};

class DatabaseWorkerThread : public QThread
{
public:
    explicit DatabaseWorkerThread(DatabaseWorker* worker);

private:
    void run();

    DatabaseWorker* m_worker;
};

#endif // SQLDATABASE_H
