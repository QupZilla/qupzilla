/* ============================================================
* QupZilla - WebKit based browser
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
#include "ieimporter.h"

#include "bookmarksimportdialog.h"

#include <QDir>
#include <QSettings>

IeImporter::IeImporter(QObject* parent)
    : QObject(parent)
    , m_error(false)
    , m_errorString(BookmarksImportDialog::tr("No Error"))
{
}

void IeImporter::setFile(const QString &path)
{
    m_path = path;
}

bool IeImporter::openFile()
{
    QDir dir(m_path);
    if (!dir.exists()) {
        m_error = true;
        m_errorString = BookmarksImportDialog::tr("Directory does not exist.");
        return false;
    }

    QStringList filters;
    filters << "*.url";

    urls = dir.entryInfoList(filters);

    if (urls.isEmpty()) {
        m_error = true;
        m_errorString = BookmarksImportDialog::tr("The directory does not contain any bookmarks.");
        return false;
    }

    return true;
}

QVector<Bookmarks::Bookmark> IeImporter::exportBookmarks()
{
    QVector<Bookmarks::Bookmark> bookmarks;

    foreach (QFileInfo file, urls) {
        QSettings urlFile(file.absoluteFilePath(), QSettings::IniFormat, this);

        QUrl url = urlFile.value("InternetShortcut/URL").toUrl();

        Bookmarks::Bookmark bookmark;
        bookmark.folder = "Internet Explorer Import";
        bookmark.title = file.baseName();
        bookmark.url = url;

        bookmarks.append(bookmark);
    }

    return bookmarks;
}
