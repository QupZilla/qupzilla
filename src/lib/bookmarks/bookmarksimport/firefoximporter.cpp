/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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

#include <QDir>
#include <QVariant>
#include <QSqlError>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlDatabase>

#define CONNECTION "firefox-places-import"

FirefoxImporter::FirefoxImporter(QObject* parent)
    : BookmarksImporter(parent)
{
}

FirefoxImporter::~FirefoxImporter()
{
    QSqlDatabase::removeDatabase(CONNECTION);
}

QString FirefoxImporter::description() const
{
    return BookmarksImporter::tr("Mozilla Firefox stores its bookmarks in <b>places.sqlite</b> SQLite "
                                 "database. This file is usually located in");
}

QString FirefoxImporter::standardPath() const
{
#ifdef Q_OS_WIN
    return QString("%APPDATA%/Mozilla/");
#else
    return QDir::homePath() + QLatin1String("/.mozilla/firefox/");
#endif
}

QString FirefoxImporter::getPath(QWidget* parent)
{
    m_path = QFileDialog::getOpenFileName(parent, BookmarksImporter::tr("Choose file..."), standardPath(), "Places (places.sqlite)");
    return m_path;
}

bool FirefoxImporter::prepareImport()
{
    // Make sure this connection is properly closed if already opened
    QSqlDatabase::removeDatabase(CONNECTION);

    // Create new connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION);

    if (!QFile::exists(m_path)) {
        setError(BookmarksImportDialog::tr("File does not exist."));
        return false;
    }

    db.setDatabaseName(m_path);

    if (!db.open()) {
        setError(BookmarksImportDialog::tr("Unable to open database. Is Firefox running?"));
        return false;
    }

    return true;
}

BookmarkItem* FirefoxImporter::importBookmarks()
{
    QList<Item> items;

    BookmarkItem* root = new BookmarkItem(BookmarkItem::Folder);
    root->setTitle("Firefox Import");

    QSqlQuery query(QSqlDatabase::database(CONNECTION));
    query.prepare("SELECT id, parent, type, title, fk FROM moz_bookmarks WHERE fk NOT NULL OR type = 3");
    query.exec();

    while (query.next()) {
        Item item;
        item.id = query.value(0).toInt();
        item.parent = query.value(1).toInt();
        item.type = typeFromValue(query.value(2).toInt());
        item.title = query.value(3).toString();
        int fk = query.value(4).toInt();

        if (item.type == BookmarkItem::Invalid) {
            continue;
        }

        QSqlQuery query(QSqlDatabase::database(CONNECTION));
        query.prepare("SELECT url FROM moz_places WHERE id=?");
        query.addBindValue(fk);
        query.exec();

        if (query.next()) {
            item.url = query.value(0).toUrl();
        }

        if (item.url.scheme() == QLatin1String("place")) {
            continue;
        }

        items.append(item);
    }

    if (query.lastError().isValid()) {
        setError(query.lastError().text());
    }

    QHash<int, BookmarkItem*> hash;

    foreach (const Item &item, items) {
        BookmarkItem* parent = hash.value(item.parent);
        BookmarkItem* bookmark = new BookmarkItem(item.type, parent ? parent : root);
        bookmark->setTitle(item.title.isEmpty() ? item.url.toString() : item.title);
        bookmark->setUrl(item.url);

        hash.insert(item.id, bookmark);
    }

    return root;
}

BookmarkItem::Type FirefoxImporter::typeFromValue(int value)
{
    switch (value) {
    case 1:
        return BookmarkItem::Url;
    case 2:
        return BookmarkItem::Folder;
    case 3:
        return BookmarkItem::Separator;
    default:
        return BookmarkItem::Invalid;
    }
}
