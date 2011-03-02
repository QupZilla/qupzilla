#include "mainapplication.h"
#include "bookmarksmodel.h"
#include "webview.h"

// SQLite DB -> table bookmarks + folders
// Unique in bookmarks table is id
// However from bookmark icon, it is not possible to add more than one bookmark
// Only from Ctrl+D dialog it is possible

BookmarksModel::BookmarksModel()
{
    loadSettings();
}

void BookmarksModel::loadSettings()
{
    QSettings settings(MainApplication::getInstance()->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    m_showMostVisited = settings.value("showMostVisited",true).toBool();
    settings.endGroup();
}

void BookmarksModel::setShowingMostVisited(bool state)
{
    QSettings settings(MainApplication::getInstance()->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("showMostVisited",state);
    settings.endGroup();
    m_showMostVisited = state;
}

bool BookmarksModel::isBookmarked(QUrl url)
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
int BookmarksModel::bookmarkId(QUrl url)
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

int BookmarksModel::bookmarkId(QUrl url, QString title, QString folder)
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

QStringList BookmarksModel::getBookmark(int id)
{
    QSqlQuery query;
    query.prepare("SELECT url, title, folder FROM bookmarks WHERE id=?");
    query.bindValue(0, id);
    query.exec();
    if (!query.next())
        return QStringList();
    QStringList list;
    list.append(query.value(0).toString());
    list.append(query.value(1).toString());
    list.append(query.value(2).toString());
    return list;
}

bool BookmarksModel::saveBookmark(QUrl url, QString title, QString folder)
{
    if (url.isEmpty() || title.isEmpty() || folder.isEmpty())
        return false;

    QSqlQuery query;
    query.prepare("INSERT INTO bookmarks (url, title, folder) VALUES (?,?,?)");
    query.bindValue(0, url.toString());
    query.bindValue(1, title);
    query.bindValue(2, folder);
    return query.exec();
}

bool BookmarksModel::saveBookmark(WebView *view, QString folder)
{
    return saveBookmark(view->url(), view->title(), folder);
}

bool BookmarksModel::removeBookmark(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM bookmarks WHERE id = ?");
    query.bindValue(0, id);
    return query.exec();
}

bool BookmarksModel::removeBookmark(QUrl url)
{
    return removeBookmark(bookmarkId(url));
}

bool BookmarksModel::removeBookmark(WebView *view)
{
    return removeBookmark(bookmarkId(view->url()));
}

bool BookmarksModel::editBookmark(int id, QString title, QString folder)
{
    QSqlQuery query;
    query.prepare("UPDATE bookmarks SET title=?, folder=? WHERE id=?");
    query.bindValue(0, title);
    query.bindValue(1, folder);
    query.bindValue(2, id);
    return query.exec();
}

bool BookmarksModel::editBookmark(int id, QUrl url, QString title)
{
    QSqlQuery query;
    query.prepare("UPDATE bookmarks SET title=?, url=? WHERE id=?");
    query.bindValue(0, title);
    query.bindValue(1, url.toString());
    query.bindValue(2, id);
    return query.exec();
}
