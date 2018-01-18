/* ============================================================
* QupZilla - Qt web browser
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
#ifndef HTMLEXPORTER_H
#define HTMLEXPORTER_H

#include "bookmarksexporter.h"

class QTextStream;

class BookmarkItem;

class HtmlExporter : public BookmarksExporter
{
public:
    explicit HtmlExporter(QObject* parent = 0);

    QString name() const;
    QString getPath(QWidget* parent);
    bool exportBookmarks(BookmarkItem* root);

private:
    void writeBookmark(BookmarkItem* item, QTextStream &stream, int level);

    QString m_path;
};

#endif // HTMLEXPORTER_H
