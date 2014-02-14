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
#ifndef FIREFOXIMPORTER_H
#define FIREFOXIMPORTER_H

#include <QSqlDatabase>

#include "bookmarksimporter.h"

class QT_QUPZILLA_EXPORT FirefoxImporter : public BookmarksImporter
{
public:
    explicit FirefoxImporter(QObject* parent = 0);

    QString description() const;
    QString standardPath() const;

    QString getPath(QWidget* parent);
    bool prepareImport();

    BookmarkItem* importBookmarks();

private:
    QString m_path;
    QSqlDatabase m_db;

};

#endif // FIREFOXIMPORTER_H
