/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2013-2014  Mattias Cibien <mattias@mattiascibien.net>
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
#ifndef IEIMPORTER_H
#define IEIMPORTER_H

#include <QFileInfoList>

#include "bookmarksimporter.h"

class IeImporter : public BookmarksImporter
{
public:
    explicit IeImporter(QObject* parent = 0);

    QString description() const;
    QString standardPath() const;

    QString getPath(QWidget* parent);
    bool prepareImport();

    BookmarkItem* importBookmarks();

private:
    void readDir(const QDir &dir, BookmarkItem* parent);

    QString m_path;
};

#endif // IEIMPORTER_H
