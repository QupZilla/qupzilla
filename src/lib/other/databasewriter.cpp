/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "databasewriter.h"

#include <QTimer>

DatabaseWriter::DatabaseWriter(QObject* parent)
    : QObject(parent)
{
}

void DatabaseWriter::executeQuery(const QSqlQuery &query)
{
    m_queries.append(query);

    QTimer::singleShot(0, this, SLOT(execute()));
}

void DatabaseWriter::execute()
{
    if (m_queries.count() == 0) {
        return;
    }

    QSqlQuery query = m_queries.takeLast();
    query.exec();
}
