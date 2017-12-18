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
#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include <QHash>
#include <QMutex>
#include <QFuture>
#include <QSqlQuery>

#include "qzcommon.h"

class QUPZILLA_EXPORT SqlDatabase : public QObject
{
    Q_OBJECT

public:
    explicit SqlDatabase(QObject* parent = 0);
    ~SqlDatabase();

    // Returns database connection for current thread
    QSqlDatabase database();

    // Executes query using correct database for current thread
    QSqlQuery exec(QSqlQuery &query);

    // Executes query asynchronously on one thread from QThreadPool
    QFuture<QSqlQuery> execAsync(const QSqlQuery &query);

    static SqlDatabase* instance();
};

#endif // SQLDATABASE_H
