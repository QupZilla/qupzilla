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
#include "mainapplication.h"
#include "qztools.h"
#include "webview.h"
#include "settings.h"
#include "json.h"

#include <QSqlQuery>
#include <QTextStream>
#include <QDebug>
#include <QFile>

Bookmarks::Bookmarks(QObject* parent)
    : QObject(parent)
{
    init();
    loadSettings();
}

void Bookmarks::init()
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

    const QString bookmarksFile = mApp->currentProfilePath() + QLatin1String("/bookmarks.json");
    const QString backupFile = bookmarksFile + QLatin1String(".old");

    QFile file(bookmarksFile);
    file.open(QFile::ReadOnly);
    QByteArray data = file.readAll();
    file.close();

    bool ok;
    const QVariant res = Json::parse(data, &ok);

    if (!ok || res.type() != QVariant::Map) {
        qWarning() << "Bookmarks::init() Error parsing bookmarks! Using default bookmarks!";
        qWarning() << "Bookmarks::init() Your bookmarks have been backed up in" << backupFile;

        // Backup the user bookmarks
        QFile::remove(backupFile);
        QFile::copy(bookmarksFile, backupFile);

        // Load default bookmarks
        const QVariant data = Json::parse(QzTools::readAllFileByteContents(":data/bookmarks.json"), &ok);

        Q_ASSERT(ok);
        Q_ASSERT(data.type() == QVariant::Map);

        loadBookmarksFromMap(data.toMap().value("roots").toMap());
    }
    else {
        loadBookmarksFromMap(res.toMap().value("roots").toMap());
    }

    m_lastFolder = m_folderUnsorted;
    m_model = new BookmarksModel(this, this);
}

void Bookmarks::loadBookmarksFromMap(const QVariantMap &map)
{
#define READ_FOLDER(name, folder) \
    readBookmarks(map.value(name).toMap().value("children").toList(), folder); \
    folder->setExpanded(map.value(name).toMap().value("expanded").toBool()); \
    folder->setSidebarExpanded(map.value(name).toMap().value("expanded_sidebar").toBool());

    READ_FOLDER("bookmark_bar", m_folderToolbar)
    READ_FOLDER("bookmark_menu", m_folderMenu)
    READ_FOLDER("other", m_folderUnsorted)
#undef READ_FOLDER
}

void Bookmarks::saveBookmarks()
{
    QVariantMap bookmarksMap;

#define WRITE_FOLDER(name, mapName, folder) \
    QVariantMap mapName; \
    mapName.insert("children", writeBookmarks(folder)); \
    mapName.insert("expanded", folder->isExpanded()); \
    mapName.insert("expanded_sidebar", folder->isSidebarExpanded()); \
    mapName.insert("name", folder->title()); \
    mapName.insert("description", folder->description()); \
    mapName.insert("type", "folder"); \
    bookmarksMap.insert(name, mapName);

    WRITE_FOLDER("bookmark_bar", toolbarMap, m_folderToolbar)
    WRITE_FOLDER("bookmark_menu", menuMap, m_folderMenu)
    WRITE_FOLDER("other", unsortedMap, m_folderUnsorted)
#undef WRITE_FOLDER

    QVariantMap map;
    map.insert("version", Qz::bookmarksVersion);
    map.insert("roots", bookmarksMap);

    bool ok;
    const QByteArray data = Json::serialize(map, &ok);

    if (!ok || data.isEmpty()) {
        qWarning() << "Bookmarks::saveBookmarks() Error serializing bookmarks!";
        return;
    }

    QFile file(mApp->currentProfilePath() + QLatin1String("/bookmarks.json"));

    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "Bookmarks::saveBookmarks() Error opening bookmarks file for writing!";
    }

    file.write(data);
    file.close();
}

void Bookmarks::readBookmarks(const QVariantList &list, BookmarkItem* parent)
{
    Q_ASSERT(parent);

    foreach (const QVariant &entry, list) {
        const QVariantMap map = entry.toMap();
        BookmarkItem::Type type = BookmarkItem::typeFromString(map.value("type").toString());

        if (type == BookmarkItem::Invalid) {
            continue;
        }

        BookmarkItem* item = new BookmarkItem(type, parent);

        switch (type) {
        case BookmarkItem::Url:
            item->setUrl(QUrl::fromEncoded(map.value("url").toByteArray()));
            item->setTitle(map.value("name").toString());
            item->setDescription(map.value("description").toString());
            item->setKeyword(map.value("keyword").toString());
            item->setVisitCount(map.value("visit_count").toInt());
            break;

        case BookmarkItem::Folder:
            item->setTitle(map.value("name").toString());
            item->setDescription(map.value("description").toString());
            item->setExpanded(map.value("expanded").toBool());
            item->setSidebarExpanded(map.value("expanded_sidebar").toBool());
            break;

        default:
            break;
        }

        if (map.contains("children")) {
            readBookmarks(map.value("children").toList(), item);
        }
    }
}

QVariantList Bookmarks::writeBookmarks(BookmarkItem* parent)
{
    Q_ASSERT(parent);

    QVariantList list;

    foreach (BookmarkItem* child, parent->children()) {
        QVariantMap map;
        map.insert("type", BookmarkItem::typeToString(child->type()));

        switch (child->type()) {
        case BookmarkItem::Url:
            map.insert("url", child->url().toEncoded());
            map.insert("name", child->title());
            map.insert("description", child->description());
            map.insert("keyword", child->keyword());
            map.insert("visit_count", child->visitCount());
            break;

        case BookmarkItem::Folder:
            map.insert("name", child->title());
            map.insert("description", child->description());
            map.insert("expanded", child->isExpanded());
            map.insert("expanded_sidebar", child->isSidebarExpanded());
            break;

        default:
            break;
        }

        if (!child->children().isEmpty()) {
            map.insert("children", writeBookmarks(child));
        }

        list.append(map);
    }

    return list;
}

void Bookmarks::search(QList<BookmarkItem*>* items, BookmarkItem* parent, const QUrl &url) const
{
    Q_ASSERT(items);
    Q_ASSERT(parent);

    switch (parent->type()) {
    case BookmarkItem::Root:
    case BookmarkItem::Folder:
        foreach (BookmarkItem* child, parent->children()) {
            search(items, child, url);
        }
        break;

    case BookmarkItem::Url:
        if (parent->url() == url) {
            items->append(parent);
        }
        break;

    default:
        break;
    }
}

void Bookmarks::search(QList<BookmarkItem*>* items, BookmarkItem* parent, const QString &string, Qt::CaseSensitivity sensitive) const
{
    Q_ASSERT(items);
    Q_ASSERT(parent);

    switch (parent->type()) {
    case BookmarkItem::Root:
    case BookmarkItem::Folder:
        foreach (BookmarkItem* child, parent->children()) {
            search(items, child, string, sensitive);
        }
        break;

    case BookmarkItem::Url:
        if (parent->title().contains(string, sensitive) ||
                parent->urlString().contains(string, sensitive) ||
                parent->description().contains(string, sensitive) ||
                parent->keyword().compare(string, sensitive) == 0) {
            items->append(parent);
        }
        break;

    default:
        break;
    }
}

void Bookmarks::loadSettings()
{
    Settings settings;
    settings.beginGroup("Bookmarks");
    m_showMostVisited = settings.value("showMostVisited", true).toBool();
    m_showOnlyIconsInToolbar = settings.value("showOnlyIconsInToolbar", false).toBool();
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

void Bookmarks::setLastFolder(const QString &folder)
{
    Settings settings;
    settings.beginGroup("Bookmarks");
    settings.setValue("lastFolder", folder);
    settings.endGroup();
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

BookmarkItem* Bookmarks::lastUsedFolder() const
{
    return m_lastFolder;
}

bool Bookmarks::isBookmarked(const QUrl &url)
{
    return !searchBookmarks(url).isEmpty();
}

QList<BookmarkItem*> Bookmarks::searchBookmarks(const QUrl &url) const
{
    QList<BookmarkItem*> items;
    search(&items, m_root, url);
    return items;
}

QList<BookmarkItem*> Bookmarks::searchBookmarks(const QString &string, Qt::CaseSensitivity sensitive) const
{
    QList<BookmarkItem*> items;
    search(&items, m_root, string, sensitive);
    return items;
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
    Q_ASSERT(item);

    emit bookmarkChanged(item);
}

bool Bookmarks::canBeModified(BookmarkItem* item) const
{
    Q_ASSERT(item);

    return item != m_root &&
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

    m_lastFolder = parent;
    m_model->addBookmark(parent, row, item);
    emit bookmarkAdded(item);
}
