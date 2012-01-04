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
#ifndef FIREFOXIMPORTER_H
#define FIREFOXIMPORTER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

#include "bookmarksmodel.h"

class FirefoxImporter : public QObject
{
    Q_OBJECT
public:
    explicit FirefoxImporter(QObject* parent = 0);

    void setFile(const QString &path);
    bool openDatabase();

    QList<BookmarksModel::Bookmark> exportBookmarks();

    bool error() { return m_error; }
    QString errorString() { return m_errorString; }

signals:

public slots:

private:
    QString m_path;
    QSqlDatabase db;

    bool m_error;
    QString m_errorString;

};

#endif // FIREFOXIMPORTER_H
