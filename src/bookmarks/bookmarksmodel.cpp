/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include "bookmarksmodel.h"
#include "mainapplication.h"
#include "webview.h"
#include "iconprovider.h"

// SQLite DB -> table bookmarks + folders
// Unique in bookmarks table is id
// However from bookmark icon, it is not possible to add more than one bookmark
// Only from Ctrl+D dialog it is possible

BookmarksModel::BookmarksModel(QObject *parent)
    : QObject(parent)
{
    loadSettings();
}

void BookmarksModel::loadSettings()
{
    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    m_showMostVisited = settings.value("showMostVisited",true).toBool();
    settings.endGroup();
}

void BookmarksModel::setShowingMostVisited(bool state)
{
    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("showMostVisited",state);
    settings.endGroup();
    m_showMostVisited = state;
}

bool BookmarksModel::isBookmarked(const QUrl &url)
{
    QSqlQuery query;
    query.prepare("SELECT count(id) FROM bookmarks WHERE url=?");
    query.bindValue(0, url.toString());
    query.exec();
    query.next();
    return query.value(0).toInt()>0;
}

// Bookmark search priority:
// Bookmarks in menu > bookmarks in toolbar -> user folders and unsorted
int BookmarksModel::bookmarkId(const QUrl &url)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM bookmarks WHERE url=? AND folder='bookmarksMenu' ");
    query.bindValue(0, url.toString());
    query.exec();
    if (query.next())
        return query.value(0).toInt();

    query.prepare("SELECT id FROM bookmarks WHERE url=? AND folder='bookmarksToolbar' ");
    query.bindValue(0, url.toString());
    query.exec();
    if (query.next())
        return query.value(0).toInt();

    query.prepare("SELECT id FROM bookmarks WHERE url=? ");
    query.bindValue(0, url.toString());
    query.exec();
    if (query.next())
        return query.value(0).toInt();

    return -1;
}

int BookmarksModel::bookmarkId(const QUrl &url, const QString &title, const QString &folder)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM bookmarks WHERE url=? AND title=? AND folder=? ");
    query.bindValue(0, url.toString());
    query.bindValue(1, title);
    query.bindValue(2, folder);
    query.exec();
    if (query.next())
        return query.value(0).toInt();
    return -1;
}

BookmarksModel::Bookmark BookmarksModel::getBookmark(int id)
{
    Bookmark bookmark;
    QSqlQuery query;
    query.prepare("SELECT url, title, folder, icon FROM bookmarks WHERE id=?");
    query.bindValue(0, id);
    query.exec();
    if (query.next()) {
        bookmark.id = id;
        bookmark.url = query.value(0).toUrl();
        bookmark.title = query.value(1).toString();
        bookmark.folder = query.value(2).toString();
        bookmark.icon = IconProvider::iconFromBase64(query.value(3).toByteArray());
    }
    return bookmark;
}

bool BookmarksModel::saveBookmark(const QUrl &url, const QString &title, const QIcon &icon, const QString &folder)
{
    if (url.isEmpty() || title.isEmpty() || folder.isEmpty())
        return false;

    QSqlQuery query;
    query.prepare("INSERT INTO bookmarks (url, title, folder, icon) VALUES (?,?,?,?)");
    query.bindValue(0, url.toString());
    query.bindValue(1, title);
    query.bindValue(2, folder);
    query.bindValue(3, IconProvider::iconToBase64(icon));

    if (!query.exec())
        return false;

    Bookmark bookmark;
    bookmark.id = query.lastInsertId().toInt();
    bookmark.url = url;
    bookmark.title = title;
    bookmark.folder = folder;
    bookmark.icon = icon;
    emit bookmarkAdded(bookmark);
    mApp->sendMessages(MainApplication::BookmarksChanged, true);
    return true;
}

bool BookmarksModel::saveBookmark(WebView *view, const QString &folder)
{
    return saveBookmark(view->url(), view->title(), view->siteIcon(), folder);
}

bool BookmarksModel::removeBookmark(int id)
{
    QSqlQuery query;
    query.prepare("SELECT url, title, folder FROM bookmarks WHERE id = ?");
    query.bindValue(0, id);
    query.exec();
    if (!query.next())
        return false;

    Bookmark bookmark;
    bookmark.id = id;
    bookmark.url = query.value(0).toUrl();
    bookmark.title = query.value(1).toString();
    bookmark.folder = query.value(2).toString();
    bookmark.icon = IconProvider::iconFromBase64(query.value(3).toByteArray());

    if (!query.exec("DELETE FROM bookmarks WHERE id = " + QString::number(id)))
        return false;

    emit bookmarkDeleted(bookmark);
    mApp->sendMessages(MainApplication::BookmarksChanged, true);
    return true;
}

bool BookmarksModel::removeBookmark(const QUrl &url)
{
    return removeBookmark(bookmarkId(url));
}

bool BookmarksModel::removeBookmark(WebView* view)
{
    return removeBookmark(bookmarkId(view->url()));
}

//bool BookmarksModel::editBookmark(int id, const QString &title, const QString &folder)
//{
//    QSqlQuery query;
//    query.prepare("UPDATE bookmarks SET title=?, folder=? WHERE id=?");
//    query.bindValue(0, title);
//    query.bindValue(1, folder);
//    query.bindValue(2, id);
//    return query.exec();
//}

//bool BookmarksModel::editBookmark(int id, const QUrl &url, const QString &title)
//{
//    QSqlQuery query;
//    query.prepare("UPDATE bookmarks SET title=?, url=? WHERE id=?");
//    query.bindValue(0, title);
//    query.bindValue(1, url.toString());
//    query.bindValue(2, id);
//    return query.exec();
//}

bool BookmarksModel::editBookmark(int id, const QString &title, const QUrl &url, const QString &folder)
{
    if (title.isEmpty() && url.isEmpty() && folder.isEmpty())
        return false;
    QSqlQuery query;
    if (!query.exec("SELECT title, url, folder, icon FROM bookmarks WHERE id = "+QString::number(id)))
        return false;

    query.next();

    Bookmark before;
    before.id = id;
    before.title = query.value(0).toString();
    before.url = query.value(1).toUrl();
    before.folder = query.value(2).toString();
    before.icon = IconProvider::iconFromBase64(query.value(3).toByteArray());

    Bookmark after;
    after.id = id;
    after.title = title.isEmpty() ? before.title : title;
    after.url = url.isEmpty() ? before.url : url;
    after.folder = folder.isEmpty() ? before.folder : folder;
    after.icon = before.icon;

    query.prepare("UPDATE bookmarks SET title=?, url=?, folder=? WHERE id = ?");
    query.bindValue(0, after.title);
    query.bindValue(1, after.url.toString());
    query.bindValue(2, after.folder);
    query.bindValue(3, id);

    if (!query.exec())
        return false;

    emit bookmarkEdited(before, after);
    mApp->sendMessages(MainApplication::BookmarksChanged, true);
    return true;
}

bool BookmarksModel::createFolder(const QString &name)
{
    QSqlQuery query;
    query.prepare("INSERT INTO folders (name) VALUES (?)");
    query.bindValue(0, name);
    if (!query.exec())
        return false;

    emit folderAdded(name);
    mApp->sendMessages(MainApplication::BookmarksChanged, true);
    return true;
}

bool BookmarksModel::removeFolder(const QString &name)
{
    if (name == tr("Bookmarks In Menu") || name == tr("Bookmarks In ToolBar"))
        return false;

    QSqlQuery query;
    query.prepare("SELECT id FROM bookmarks WHERE folder = ? ");
    query.bindValue(0, name);
    if (!query.exec())
        return false;
    while (query.next())
        removeBookmark(query.value(0).toInt());

    query.prepare("DELETE FROM folders WHERE name=?");
    query.bindValue(0, name);
    if (!query.exec())
        return false;

    query.prepare("DELETE FROM bookmarks WHERE folder=?");
    query.bindValue(0, name);
    if (!query.exec())
        return false;

    emit folderDeleted(name);
    mApp->sendMessages(MainApplication::BookmarksChanged, true);
    return true;
}

bool BookmarksModel::bookmarksEqual(const Bookmark &one, const Bookmark &two)
{
    if (one.id != two.id)
        return false;
    if (one.title != two.title)
        return false;
    if (one.folder != two.folder)
        return false;
    if (one.url != two.url)
        return false;
    return true;
}

QString BookmarksModel::toTranslatedFolder(const QString &name)
{
    QString trFolder;
    if (name == "bookmarksMenu")
        trFolder = tr("Bookmarks In Menu");
    else if (name == "bookmarksToolbar")
        trFolder = tr("Bookmarks In ToolBar");
    else if (name == "unsorted")
        trFolder = tr("Unsorted Bookmarks");
    else
        trFolder = name;
    return trFolder;
}

QString BookmarksModel::fromTranslatedFolder(const QString &name)
{
    QString folder;
    if (name == tr("Bookmarks In Menu"))
        folder = "bookmarksMenu";
    else if (name == tr("Bookmarks In ToolBar"))
        folder = "bookmarksToolbar";
    else if (name == tr("Unsorted Bookmarks"))
        folder = "unsorted";
    else
        folder = name;
    return folder;
}
