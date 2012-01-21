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
#include "firefoximporter.h"
#include "bookmarksimportdialog.h"

FirefoxImporter::FirefoxImporter(QObject* parent)
    : QObject(parent)
    , m_error(false)
    , m_errorString(BookmarksImportDialog::tr("No Error"))
{
}

void FirefoxImporter::setFile(const QString &path)
{
    m_path = path;
}

bool FirefoxImporter::openDatabase()
{
    db = QSqlDatabase::cloneDatabase(QSqlDatabase::database(), "import");

    if (!QFile::exists(m_path)) {
        m_error = true;
        m_errorString = BookmarksImportDialog::tr("File does not exists.");
        return false;
    }
    db.setDatabaseName(m_path);
    bool open = db.open();

    if (!open) {
        m_error = true;
        m_errorString = BookmarksImportDialog::tr("Unable to open database. Is Firefox running?");
        return false;
    }

    return true;
}

QList<BookmarksModel::Bookmark> FirefoxImporter::exportBookmarks()
{
    QList<BookmarksModel::Bookmark> list;

    QSqlQuery query(db);
    query.exec("SELECT title, fk FROM moz_bookmarks WHERE title != ''");
    while (query.next()) {
        QString title = query.value(0).toString();
        int placesId = query.value(1).toInt();

        QSqlQuery query2(db);
        query2.exec("SELECT url FROM moz_places WHERE id=" + QString::number(placesId));

        if (!query2.next()) {
            continue;
        }

        QString url = query2.value(0).toString();

        if (title.isEmpty() || url.isEmpty() || url.startsWith("place:")) {
            continue;
        }

        BookmarksModel::Bookmark b;
        b.folder = "Firefox Import";
        b.title = title;
        b.url = url;

        list.append(b);
    }

    if (query.lastError().isValid()) {
        m_error = true;
        m_errorString = query.lastError().text();
    }

    return list;
}
