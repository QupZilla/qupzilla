/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef DATABASEWRITER_H
#define DATABASEWRITER_H

#include <QObject>
#include <QSqlQuery>
#include <QVector>

#include "qzcommon.h"

class QUPZILLA_EXPORT DatabaseWriter : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseWriter(QObject* parent = 0);

    // Delayed execution of query
    void executeQuery(const QSqlQuery &query);

    static DatabaseWriter* instance();

private slots:
    void execute();

private:
    QVector<QSqlQuery> m_queries;

    static DatabaseWriter* s_instance;
};

#endif // DATABASEWRITER_H
