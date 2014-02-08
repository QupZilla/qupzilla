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
#include "bookmarks.h"
#include "bookmarkitem.h"
#include "bookmarksmodel.h"
#include "tabbedwebview.h"
#include "iconprovider.h"
#include "mainapplication.h"
#include "settings.h"
#include "json.h"

#include <QSqlQuery>
#include <QTextStream>
#include <QBuffer>
#include <QDebug>
#include <QFile>

// SQLite DB -> table bookmarks + folders
// Unique in bookmarks table is id
// However from bookmark icon, it is not possible to add more than one bookmark
// Only from Ctrl+D dialog it is possible

Bookmarks::Bookmarks(QObject* parent)
    : QObject(parent)
{
    loadBookmarks();
    loadSettings();
}

void Bookmarks::loadBookmarks()
{
    m_root = new BookmarkItem(BookmarkItem::Root);

    m_folderToolbar = new BookmarkItem(BookmarkItem::Folder, m_root);
    m_folderToolbar->setTitle(tr("Bookmarks Toolbar"));
    m_folderToolbar->setDescription(tr("Bookmarks located in Bookmarks Toolbar"));

    m_folderMenu = new BookmarkItem(BookmarkItem::Folder, m_root);
    m_folderMenu->setTitle(tr("Bookmarks Menu"));
    m_folderMenu->setDescription(tr("Bookmarks located in Bookmarks Menu"));

    m_folderUnsorted = new BookmarkItem(BookmarkItem::Folder, m_root);
    m_folderUnsorted->setTitle(tr("Unsorted Bookmarks"));
    m_folderUnsorted->setDescription(tr("All other bookmarks"));

    // TODO: Make sure bookmarks are loaded correctly even on error

    QFile bFile(mApp->currentProfilePath() + QLatin1String("/bookmarks.json"));
    bFile.open(QFile::ReadOnly);
    QByteArray data = bFile.readAll();
    bFile.close();

    bool ok;
    const QVariant res = Json::parse(data, &ok);

    if (!ok || res.type() != QVariant::Map) {
        qWarning() << "Bookmarks::loadBookmarks() Error parsing bookmarks!";
        return;
    }

    const QVariantMap map = res.toMap();
    const QVariantMap bookmarksMap = map.value("roots").toMap();

    readBookmarks(bookmarksMap.value("bookmark_bar").toMap().value("children").toList(), m_folderToolbar);
    readBookmarks(bookmarksMap.value("bookmark_menu").toMap().value("children").toList(), m_folderMenu);
    readBookmarks(bookmarksMap.value("other").toMap().value("children").toList(), m_folderUnsorted);

    m_model = new BookmarksModel(this, this);
}

void Bookmarks::saveBookmarks()
{
    QVariantMap toolbarMap;
    toolbarMap.insert("children", writeBookmarks(m_folderToolbar));

    QVariantMap menuMap;
    menuMap.insert("children", writeBookmarks(m_folderMenu));

    QVariantMap unsortedMap;
    unsortedMap.insert("children", writeBookmarks(m_folderUnsorted));

    QVariantMap bookmarksMap;
    bookmarksMap.insert("bookmark_bar", toolbarMap);
    bookmarksMap.insert("bookmark_menu", menuMap);
    bookmarksMap.insert("other", unsortedMap);

    QVariantMap map;
    map.insert("version", Qz::bookmarksVersion);
    map.insert("roots", bookmarksMap);

    bool ok;
    const QByteArray data = Json::serialize(map, &ok);

    if (!ok || data.isEmpty()) {
        qWarning() << "Bookmarks::saveBookmarks() Error serializing bookmarks!";
        return;
    }

    QFile bFile(mApp->currentProfilePath() + QLatin1String("/bookmarks.json"));

    if (!bFile.open(QFile::WriteOnly)) {
        qWarning() << "Bookmarks::saveBookmarks() Error opening bookmarks file for writing!";
    }

    bFile.write(data);
    bFile.close();
}

void Bookmarks::readBookmarks(const QVariantList &list, BookmarkItem* parent)
{
    if (!parent) {
        return;
    }

    foreach (const QVariant &entry, list) {
        const QVariantMap map = entry.toMap();
        BookmarkItem::Type type = BookmarkItem::typeFromString(map.value("type").toString());

        if (type == BookmarkItem::Invalid) {
            continue;
        }

        BookmarkItem* item = new BookmarkItem(type, parent);
        item->setUrl(map.value("url").toUrl());
        item->setTitle(map.value("name").toString());
        item->setDescription(map.value("description").toString());
        item->setKeyword(map.value("keyword").toString());
        item->setExpanded(map.value("expanded").toBool());

        if (map.contains("children")) {
            readBookmarks(map.value("children").toList(), item);
        }
    }
}

QVariantList Bookmarks::writeBookmarks(BookmarkItem* parent)
{
    QVariantList list;

    if (!parent) {
        return list;
    }

    foreach (BookmarkItem* child, parent->children()) {
        QVariantMap map;
        map.insert("type", BookmarkItem::typeToString(child->type()));
        map.insert("url", child->url());
        map.insert("name", child->title());
        map.insert("description", child->description());
        map.insert("keyword", child->keyword());
        map.insert("expanded", child->isExpanded());

        if (!child->children().isEmpty()) {
            map.insert("children", writeBookmarks(child));
        }

        list.append(map);
    }

    return list;
}

void Bookmarks::writeChildren(BookmarkItem* parent)
{
    if (!parent) {
        return;
    }

    foreach (BookmarkItem* child, parent->children()) {
        if (child->isUrl()) {
            qDebug() << child->title() << child->url();
        }
        else {
            qDebug() << "folder" << child->title();
        }
    }
}

void Bookmarks::loadSettings()
{
    Settings settings;
    settings.beginGroup("Bookmarks");
    m_showMostVisited = settings.value("showMostVisited", true).toBool();
    m_showOnlyIconsInToolbar = settings.value("showOnlyIconsInToolbar", false).toBool();
    m_lastFolder = settings.value("LastFolder", "unsorted").toString();
    settings.endGroup();
}

bool Bookmarks::isShowingMostVisited() const
{
    return m_showMostVisited;
}

void Bookmarks::setShowingMostVisited(bool state)
{
    Settings settings;
    settings.beginGroup("Bookmarks");
    settings.setValue("showMostVisited", state);
    settings.endGroup();
    m_showMostVisited = state;
}

bool Bookmarks::isShowingOnlyIconsInToolbar() const
{
    return m_showOnlyIconsInToolbar;
}

void Bookmarks::setShowingOnlyIconsInToolbar(bool state)
{
    Settings settings;
    settings.beginGroup("Bookmarks");
    settings.setValue("showOnlyIconsInToolbar", state);
    settings.endGroup();
    m_showOnlyIconsInToolbar = state;
}

bool Bookmarks::isFolder(const QString &name)
{
    if (name == QLatin1String("bookmarksToolbar") || name == QLatin1String("bookmarksMenu")
            || name == QLatin1String("unsorted") || name == _bookmarksToolbar
            || name == _bookmarksMenu || name == _bookmarksUnsorted) {
        return true;
    }

    QSqlQuery query;
    query.prepare("SELECT name FROM folders WHERE name = ?");
    query.bindValue(0, name);
    query.exec();

    return query.next();
}

QString Bookmarks::lastFolder() const
{
    return m_lastFolder;
}

void Bookmarks::setLastFolder(const QString &folder)
{
    Settings settings;
    settings.beginGroup("Bookmarks");
    settings.setValue("lastFolder", folder);
    settings.endGroup();
    m_lastFolder = folder;
}

bool Bookmarks::isBookmarked(const QUrl &url)
{
    QSqlQuery query;
    query.prepare("SELECT count(id) FROM bookmarks WHERE url=?");
    query.bindValue(0, url.toString());

    if (!query.exec() || !query.next()) {
        return false;
    }

    return query.value(0).toInt() > 0;
}

// Bookmark search priority:
// Bookmarks in menu > bookmarks in toolbar -> user folders and unsorted
Bookmarks::Bookmark Bookmarks::getBookmark(const QUrl &url)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM bookmarks WHERE url=? AND folder='bookmarksMenu' ");
    query.bindValue(0, url.toString());
    query.exec();

    if (query.next()) {
        return getBookmark(query.value(0).toInt());
    }

    query.prepare("SELECT id FROM bookmarks WHERE url=? AND folder='bookmarksToolbar' ");
    query.bindValue(0, url.toString());
    query.exec();

    if (query.next()) {
        return getBookmark(query.value(0).toInt());
    }

    query.prepare("SELECT id FROM bookmarks WHERE url=? ");
    query.bindValue(0, url.toString());
    query.exec();

    if (query.next()) {
        return getBookmark(query.value(0).toInt());
    }

    return Bookmark();
}

Bookmarks::Bookmark Bookmarks::getBookmark(int id)
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
        bookmark.image = QImage::fromData(query.value(3).toByteArray());
        bookmark.inSubfolder = isSubfolder(bookmark.folder);
    }
    return bookmark;
}

bool Bookmarks::saveBookmark(const QUrl &url, const QString &title, const QIcon &icon, const QString &folder)
{
    if (url.isEmpty() || title.isEmpty() || folder.isEmpty()) {
        return false;
    }

    QImage image = icon.pixmap(16, 16).toImage();
    if (image.isNull()) {
        image = qIconProvider->emptyWebImage();
    }

    // createFolder() calls isFolder()
    createFolder(folder);

    QSqlQuery query;
    query.prepare("INSERT INTO bookmarks (url, title, folder, icon) VALUES (?,?,?,?)");
    query.bindValue(0, url.toString());
    query.bindValue(1, title);
    query.bindValue(2, folder);
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    query.bindValue(3, buffer.data());
    query.exec();

    Bookmark bookmark;
    bookmark.id = query.lastInsertId().toInt();
    bookmark.url = url;
    bookmark.title = title;
    bookmark.folder = folder;
    bookmark.image = image;
    bookmark.inSubfolder = isSubfolder(bookmark.folder);

    setLastFolder(folder);

    emit bookmarkAdded(bookmark);
    mApp->sendMessages(Qz::AM_BookmarksChanged, true);
    return true;
}

bool Bookmarks::saveBookmark(WebView* view, QString folder)
{
    if (folder.isEmpty()) {
        folder = m_lastFolder;
    }

    return saveBookmark(view->url(), view->title(), view->icon(), folder);
}

void Bookmarks::removeBookmark(int id)
{
    QList<int> list;
    list.append(id);

    return removeBookmark(list);
}

void Bookmarks::removeBookmark(const QList<int> list)
{
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    foreach (int id, list) {
        QSqlQuery query;
        query.prepare("SELECT url, title, folder FROM bookmarks WHERE id = ?");
        query.bindValue(0, id);
        query.exec();
        if (!query.next()) {
            continue;
        }

        Bookmark bookmark;
        bookmark.id = id;
        bookmark.url = query.value(0).toUrl();
        bookmark.title = query.value(1).toString();
        bookmark.folder = query.value(2).toString();
        bookmark.image = QImage::fromData(query.value(3).toByteArray());
        bookmark.inSubfolder = isSubfolder(bookmark.folder);

        query.prepare("DELETE FROM bookmarks WHERE id=?");
        query.addBindValue(id);
        query.exec();

        if (!query.exec()) {
            continue;
        }

        emit bookmarkDeleted(bookmark);
    }

    db.commit();
    mApp->sendMessages(Qz::AM_BookmarksChanged, true);
}

bool Bookmarks::editBookmark(int id, const QString &title, const QUrl &url, const QString &folder)
{
    if (title.isEmpty() && url.isEmpty() && folder.isEmpty()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT title, url, folder, icon FROM bookmarks WHERE id=?");
    query.addBindValue(id);
    query.exec();

    if (!query.next()) {
        return false;
    }

    Bookmark before;
    before.id = id;
    before.title = query.value(0).toString();
    before.url = query.value(1).toUrl();
    before.folder = query.value(2).toString();
    before.image = QImage::fromData(query.value(3).toByteArray());
    before.inSubfolder = isSubfolder(before.folder);

    Bookmark after;
    after.id = id;
    after.title = title.isEmpty() ? before.title : title;
    after.url = url.isEmpty() ? before.url : url;
    after.folder = folder.isEmpty() ? before.folder : folder;
    after.image = before.image;
    after.inSubfolder = isSubfolder(after.folder);

    query.prepare("UPDATE bookmarks SET title=?, url=?, folder=? WHERE id = ?");
    query.bindValue(0, after.title);
    query.bindValue(1, after.url.toString());
    query.bindValue(2, after.folder);
    query.bindValue(3, id);

    if (!query.exec()) {
        return false;
    }

    emit bookmarkEdited(before, after);
    mApp->sendMessages(Qz::AM_BookmarksChanged, true);
    return true;
}

bool Bookmarks::changeIcon(int id, const QIcon &icon)
{
    QSqlQuery query;
    query.prepare("SELECT title, url, folder, icon FROM bookmarks WHERE id=?");
    query.addBindValue(id);
    query.exec();

    if (!query.next()) {
        return false;
    }

    Bookmark before;
    before.id = id;
    before.title = query.value(0).toString();
    before.url = query.value(1).toUrl();
    before.folder = query.value(2).toString();
    before.image = QImage::fromData(query.value(3).toByteArray());
    before.inSubfolder = isSubfolder(before.folder);

    Bookmark after = before;
    after.image = icon.pixmap(16).toImage();

    query.prepare("UPDATE bookmarks SET icon = ? WHERE id = ?");
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    after.image.save(&buffer, "PNG");
    query.bindValue(0, buffer.data());
    query.bindValue(1, id);

    if (!query.exec()) {
        return false;
    }

    emit bookmarkEdited(before, after);
    mApp->sendMessages(Qz::AM_BookmarksChanged, true);
    return true;
}

bool Bookmarks::createFolder(const QString &name)
{
    if (isFolder(name)) {
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO folders (name, subfolder) VALUES (?, 'no')");
    query.bindValue(0, name);
    if (!query.exec()) {
        return false;
    }

    emit folderAdded(name);
    mApp->sendMessages(Qz::AM_BookmarksChanged, true);
    return true;
}

void Bookmarks::removeFolder(const QString &name)
{
    if (name == _bookmarksMenu || name == _bookmarksToolbar) {
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT id FROM bookmarks WHERE folder = ? ");
    query.bindValue(0, name);
    if (!query.exec()) {
        return;
    }

    QList<int> list;
    while (query.next()) {
        list.append(query.value(0).toInt());
    }
    removeBookmark(list);

    query.prepare("DELETE FROM folders WHERE name=?");
    query.bindValue(0, name);
    query.exec();

    if (name == m_lastFolder) {
        setLastFolder("unsorted");
    }

    emit folderDeleted(name);

    mApp->sendMessages(Qz::AM_BookmarksChanged, true);
}

bool Bookmarks::renameFolder(const QString &before, const QString &after)
{
    QSqlQuery query;
    query.prepare("SELECT name FROM folders WHERE name = ?");
    query.bindValue(0, after);
    query.exec();
    if (query.next()) {
        return false;
    }

    query.prepare("UPDATE folders SET name=? WHERE name=?");
    query.bindValue(0, after);
    query.bindValue(1, before);
    if (!query.exec()) {
        return false;
    }

    query.prepare("UPDATE bookmarks SET folder=? WHERE folder=?");
    query.bindValue(0, after);
    query.bindValue(1, before);
    if (!query.exec()) {
        return false;
    }

    emit folderRenamed(before, after);
    return true;
}

void Bookmarks::exportToHtml(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        qWarning() << "Bookmarks::exportHtml Cannot open file for writing!" << file.errorString();
    }

    QTextStream out(&file);

    out << "<!DOCTYPE NETSCAPE-Bookmark-file-1>" << endl;
    out << "<!-- This is an automatically generated file." << endl;
    out << "     It will be read and overwritten." << endl;
    out << "     DO NOT EDIT! -->" << endl;
    out << "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">" << endl;
    out << "<TITLE>Bookmarks</TITLE>" << endl;
    out << "<H1>Bookmarks</H1>" << endl;

    out << "<DL><p>" << endl;

    QString indent = "    ";
    QList<QPair<QString, bool> > allFolders;

    QPair<QString, bool> menu;
    menu.first = "bookmarksMenu";
    menu.second = false;

    QPair<QString, bool> toolbar;
    toolbar.first = "bookmarksToolbar";
    toolbar.second = false;

    allFolders.append(menu);
    allFolders.append(toolbar);

    QSqlQuery query;
    query.exec("SELECT name, subfolder FROM folders");

    while (query.next()) {
        QPair<QString, bool> pair;
        pair.first = query.value(0).toString();
        pair.second = query.value(1).toString() == QLatin1String("yes");

        allFolders.append(pair);
    }

    for (int i = 0; i < allFolders.size(); ++i) {
        QPair<QString, bool> pair = allFolders.at(i);

        out << indent << "<DT><H3 TOOLBAR_SUBFOLDER=\"" << (pair.second ? "yes" : "no") << "\">" << pair.first << "</H3>" << endl;
        out << indent << "<DL><p>" << endl;

        QSqlQuery q;
        q.prepare("SELECT title, url FROM bookmarks WHERE folder = ?");
        q.addBindValue(pair.first);
        q.exec();

        while (q.next()) {
            QString title = q.value(0).toString();
            QString url = q.value(1).toString();

            out << indent << indent << "<DT><A HREF=\"" << url << "\">" << title << "</A>" << endl;
        }

        out << indent << "</DL><p>" << endl;
    }

    query.exec("SELECT title, url FROM bookmarks WHERE folder='' OR folder='unsorted'");

    while (query.next()) {
        QString title = query.value(0).toString();
        QString url = query.value(1).toString();

        out << indent << "<DT><A HREF=\"" << url << "\">" << title << "</A>" << endl;
    }

    out << "</DL><p>" << endl;
}

QVector<Bookmark> Bookmarks::getFolderBookmarks(const QString &name)
{
    QVector<Bookmark> list;

    QSqlQuery query;
    query.prepare("SELECT id, url, title, folder, icon FROM bookmarks WHERE folder=?");
    query.addBindValue(name);
    query.exec();
    while (query.next()) {
        Bookmark bookmark;
        bookmark.id = query.value(0).toInt();
        bookmark.url = query.value(1).toUrl();
        bookmark.title = query.value(2).toString();
        bookmark.folder = query.value(3).toString();
        bookmark.image = QImage::fromData(query.value(4).toByteArray());
        bookmark.inSubfolder = isSubfolder(bookmark.folder);

        list.append(bookmark);
    }

    return list;
}

bool Bookmarks::createSubfolder(const QString &name)
{
    if (isFolder(name)) {
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO folders (name, subfolder) VALUES (?, 'yes')");
    query.bindValue(0, name);
    if (!query.exec()) {
        return false;
    }

    emit subfolderAdded(name);
    mApp->sendMessages(Qz::AM_BookmarksChanged, true);
    return true;
}

bool Bookmarks::isSubfolder(const QString &name)
{
    QSqlQuery query;
    query.prepare("SELECT subfolder FROM folders WHERE name = ?");
    query.bindValue(0, name);
    query.exec();
    if (!query.next()) {
        return false;
    }

    return query.value(0).toString() == QLatin1String("yes");
}

QString Bookmarks::toTranslatedFolder(const QString &name)
{
    QString trFolder;
    if (name == QLatin1String("bookmarksMenu")) {
        trFolder = tr("Bookmarks In Menu");
    }
    else if (name == QLatin1String("bookmarksToolbar")) {
        trFolder = tr("Bookmarks In ToolBar");
    }
    else if (name == QLatin1String("unsorted")) {
        trFolder = tr("Unsorted Bookmarks");
    }
    else {
        trFolder = name;
    }
    return trFolder;
}

QString Bookmarks::fromTranslatedFolder(const QString &name)
{
    QString folder;
    if (name == tr("Bookmarks In Menu")) {
        folder = "bookmarksMenu";
    }
    else if (name == tr("Bookmarks In ToolBar")) {
        folder = "bookmarksToolbar";
    }
    else if (name == tr("Unsorted Bookmarks")) {
        folder = "unsorted";
    }
    else {
        folder = name;
    }
    return folder;
}

BookmarksModel* Bookmarks::model() const
{
    return m_model;
}

BookmarkItem* Bookmarks::rootItem() const
{
    return m_root;
}

BookmarkItem* Bookmarks::toolbarFolder() const
{
    return m_folderToolbar;
}

BookmarkItem* Bookmarks::menuFolder() const
{
    return m_folderMenu;
}

BookmarkItem* Bookmarks::unsortedFolder() const
{
    return m_folderUnsorted;
}

bool Bookmarks::removeBookmark(BookmarkItem* item)
{
    if (!canBeModified(item)) {
        return false;
    }

    m_model->removeBookmark(item);
    emit bookmarkRemoved(item);

    return true;
}

void Bookmarks::notifyBookmarkChanged(BookmarkItem* item)
{
    emit bookmarkChanged(item);
}

bool Bookmarks::canBeModified(BookmarkItem* item) const
{
    return item &&
           item != m_root &&
           item != m_folderToolbar &&
           item != m_folderMenu &&
           item != m_folderUnsorted;
}

void Bookmarks::addBookmark(BookmarkItem* parent, BookmarkItem* item)
{
    Q_ASSERT(parent);
    Q_ASSERT(parent->isFolder());
    Q_ASSERT(item);

    insertBookmark(parent, 0, item);
}

void Bookmarks::insertBookmark(BookmarkItem* parent, int row, BookmarkItem* item)
{
    Q_ASSERT(parent);
    Q_ASSERT(parent->isFolder());
    Q_ASSERT(item);

    m_model->addBookmark(parent, row, item);
    emit bookmarkAdded(item);
}

void Bookmarks::changeBookmarkParent(int id, const QString &newParent, const QString &oldParent, bool* ok)
{
    QSqlQuery query;
    query.prepare("SELECT title, url, icon FROM bookmarks WHERE id=?");
    query.addBindValue(id);
    query.exec();

    if (!query.next()) {
        if (ok) {
            *ok = false;
        }
        return;
    }

    QString title = query.value(0).toString();
    QUrl url = query.value(1).toUrl();
    QByteArray imageData = query.value(2).toByteArray();

    query.prepare("UPDATE bookmarks SET folder = ? WHERE id = ?");
    query.bindValue(0, Bookmarks::fromTranslatedFolder(newParent));
    query.bindValue(1, id);

    if (!query.exec()) {
        if (ok) {
            *ok = false;
        }
        return;
    }

    emit bookmarkParentChanged(title, imageData, id, url, oldParent, newParent);
    mApp->sendMessages(Qz::AM_BookmarksChanged, true);

    if (ok) {
        *ok = true;
    }
}

void Bookmarks::changeFolderParent(const QString &name, bool isSubfolder, bool* ok)
{
    if (name.isEmpty()) {
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE folders SET subfolder=? WHERE name=?");
    query.bindValue(0, isSubfolder ? "yes" : "no");
    query.bindValue(1, Bookmarks::fromTranslatedFolder(name));
    if (!query.exec()) {
        if (ok) {
            *ok = false;
        }
        return;
    }

    emit folderParentChanged(name, isSubfolder);
    mApp->sendMessages(Qz::AM_BookmarksChanged, true);

    if (ok) {
        *ok = true;
    }
}

void Bookmarks::bookmarkDropedLink(const QUrl &url, const QString &title, const QVariant &imageVariant, const QString &folder, bool* ok)
{
    QIcon icon = qIconProvider->iconFromImage(qvariant_cast<QImage>(imageVariant));
    bool result = saveBookmark(url, title, icon, Bookmarks::fromTranslatedFolder(folder));

    if (ok) {
        *ok = result;
    }
}
