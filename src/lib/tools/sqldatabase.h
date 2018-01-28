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

    // Sets database to be created for other threads
    void setDatabase(const QSqlDatabase &database);

    static SqlDatabase* instance();

private:
    QString m_databaseName;
    QString m_connectOptions;
};

#endif // SQLDATABASE_H
