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
#include "htmlexporter.h"
#include "bookmarkitem.h"
#include "qztools.h"

#include <QTextStream>

HtmlExporter::HtmlExporter(QObject* parent)
    : BookmarksExporter(parent)
{
}

QString HtmlExporter::name() const
{
    return BookmarksExporter::tr("HTML File") + QL1S(" (bookmarks.html)");
}

QString HtmlExporter::getPath(QWidget* parent)
{
    const QString defaultPath = QDir::homePath() + QLatin1String("/bookmarks.html");
    const QString filter = BookmarksExporter::tr("HTML Bookmarks") + QL1S(" (.html)");
    m_path = QzTools::getSaveFileName("HtmlExporter", parent, BookmarksExporter::tr("Choose file..."), defaultPath, filter);
    return m_path;
}

bool HtmlExporter::exportBookmarks(BookmarkItem* root)
{
    QFile file(m_path);

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        setError(BookmarksExporter::tr("Cannot open file for writing!"));
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    stream << "<!DOCTYPE NETSCAPE-Bookmark-file-1>" << endl;
    stream << "<!-- This is an automatically generated file." << endl;
    stream << "     It will be read and overwritten." << endl;
    stream << "     DO NOT EDIT! -->" << endl;
    stream << "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">" << endl;
    stream << "<TITLE>Bookmarks</TITLE>" << endl;
    stream << "<H1>Bookmarks</H1>" << endl;

    writeBookmark(root, stream, 0);
    return true;
}

void HtmlExporter::writeBookmark(BookmarkItem* item, QTextStream &stream, int level)
{
    Q_ASSERT(item);

    QString indent;
    indent.fill(QLatin1Char(' '), level * 4);

    switch (item->type()) {
    case BookmarkItem::Url:
        stream << indent << "<DT><A HREF=\"" << item->urlString() << "\">" << item->title() << "</A>" << endl;
        break;

    case BookmarkItem::Separator:
        stream << indent << "<HR>" << endl;
        break;

    case BookmarkItem::Folder:
        stream << indent << "<DT><H3>" << item->title() << "</H3>" << endl;
        stream << indent << "<DL><p>" << endl;

        foreach (BookmarkItem* child, item->children()) {
            writeBookmark(child, stream, level + 1);
        }

        stream << indent << "</DL><p>" << endl;
        break;

    case BookmarkItem::Root:
        stream << indent << "<DL><p>" << endl;

        foreach (BookmarkItem* child, item->children()) {
            writeBookmark(child, stream, level + 1);
        }

        stream << indent << "</DL><p>" << endl;
        break;

    default:
        break;
    }
}
