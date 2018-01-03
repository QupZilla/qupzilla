/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "bookmarkstools.h"
#include "autosaver.h"
#include "datapaths.h"
#include "settings.h"
#include "qztools.h"

#include <QSaveFile>
#include <QJsonDocument>

Bookmarks::Bookmarks(QObject* parent)
    : QObject(parent)
    , m_autoSaver(0)
{
    m_autoSaver = new AutoSaver(this);
    connect(m_autoSaver, SIGNAL(save()), this, SLOT(saveSettings()));

    init();
    loadSettings();
}

Bookmarks::~Bookmarks()
{
    m_autoSaver->saveIfNecessary();
    delete m_root;
}

void Bookmarks::loadSettings()
{
    Settings settings;
    settings.beginGroup("Bookmarks");
    m_showOnlyIconsInToolbar = settings.value("showOnlyIconsInToolbar", false).toBool();
    m_showOnlyTextInToolbar = settings.value("showOnlyTextInToolbar", false).toBool();
    settings.endGroup();
}

bool Bookmarks::showOnlyIconsInToolbar() const
{
    return m_showOnlyIconsInToolbar;
}

bool Bookmarks::showOnlyTextInToolbar() const
{
    return m_showOnlyTextInToolbar;
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

BookmarksModel* Bookmarks::model() const
{
    return m_model;
}

bool Bookmarks::isBookmarked(const QUrl &url)
{
    return !searchBookmarks(url).isEmpty();
}

bool Bookmarks::canBeModified(BookmarkItem* item) const
{
    Q_ASSERT(item);

    return item != m_root &&
           item != m_folderToolbar &&
           item != m_folderMenu &&
           item != m_folderUnsorted;
}

QList<BookmarkItem*> Bookmarks::searchBookmarks(const QUrl &url) const
{
    QList<BookmarkItem*> items;
    search(&items, m_root, url);
    return items;
}

QList<BookmarkItem*> Bookmarks::searchBookmarks(const QString &string, int limit, Qt::CaseSensitivity sensitive) const
{
    QList<BookmarkItem*> items;
    search(&items, m_root, string, limit, sensitive);
    return items;
}

QList<BookmarkItem*> Bookmarks::searchKeyword(const QString &keyword) const
{
    QList<BookmarkItem*> items;
    searchKeyword(&items, m_root, keyword);
    return items;
}

void Bookmarks::addBookmark(BookmarkItem* parent, BookmarkItem* item)
{
    Q_ASSERT(parent);
    Q_ASSERT(parent->isFolder());
    Q_ASSERT(item);

    insertBookmark(parent, parent->children().count(), item);
}

void Bookmarks::insertBookmark(BookmarkItem* parent, int row, BookmarkItem* item)
{
    Q_ASSERT(parent);
    Q_ASSERT(parent->isFolder());
    Q_ASSERT(item);

    m_lastFolder = parent;
    m_model->addBookmark(parent, row, item);
    emit bookmarkAdded(item);

    m_autoSaver->changeOccurred();
}

bool Bookmarks::removeBookmark(BookmarkItem* item)
{
    if (!canBeModified(item)) {
        return false;
    }

    m_model->removeBookmark(item);
    emit bookmarkRemoved(item);

    m_autoSaver->changeOccurred();
    return true;
}

void Bookmarks::changeBookmark(BookmarkItem* item)
{
    Q_ASSERT(item);
    emit bookmarkChanged(item);

    m_autoSaver->changeOccurred();
}

void Bookmarks::setShowOnlyIconsInToolbar(bool state)
{
    m_showOnlyIconsInToolbar = state;
    emit showOnlyIconsInToolbarChanged(state);
    m_autoSaver->changeOccurred();
}

void Bookmarks::setShowOnlyTextInToolbar(bool state)
{
    m_showOnlyTextInToolbar = state;
    emit showOnlyTextInToolbarChanged(state);
    m_autoSaver->changeOccurred();
}

void Bookmarks::saveSettings()
{
    Settings settings;
    settings.beginGroup("Bookmarks");
    settings.setValue("showOnlyIconsInToolbar", m_showOnlyIconsInToolbar);
    settings.setValue("showOnlyTextInToolbar", m_showOnlyTextInToolbar);
    settings.endGroup();

    saveBookmarks();
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

    if (BookmarksTools::migrateBookmarksIfNecessary(this)) {
        // Bookmarks migrated just now, let's save them ASAP
        saveBookmarks();
    }
    else {
        // Bookmarks don't need to be migrated, just load them as usual
        loadBookmarks();
    }

    m_lastFolder = m_folderUnsorted;
    m_model = new BookmarksModel(m_root, this, this);
}

void Bookmarks::loadBookmarks()
{
    const QString bookmarksFile = DataPaths::currentProfilePath() + QLatin1String("/bookmarks.json");
    const QString backupFile = bookmarksFile + QLatin1String(".old");

    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(QzTools::readAllFileByteContents(bookmarksFile), &err);
    const QVariant res = json.toVariant();

    if (err.error != QJsonParseError::NoError || res.type() != QVariant::Map) {
        if (QFile(bookmarksFile).exists()) {
            qWarning() << "Bookmarks::init() Error parsing bookmarks! Using default bookmarks!";
            qWarning() << "Bookmarks::init() Your bookmarks have been backed up in" << backupFile;

            // Backup the user bookmarks
            QFile::remove(backupFile);
            QFile::copy(bookmarksFile, backupFile);
        }

        // Load default bookmarks
        json = QJsonDocument::fromJson(QzTools::readAllFileByteContents(QSL(":data/bookmarks.json")), &err);
        const QVariant data = json.toVariant();

        Q_ASSERT(err.error == QJsonParseError::NoError);
        Q_ASSERT(data.type() == QVariant::Map);

        loadBookmarksFromMap(data.toMap().value("roots").toMap());

        // Don't forget to save the bookmarks
        m_autoSaver->changeOccurred();
    }
    else {
        loadBookmarksFromMap(res.toMap().value("roots").toMap());
    }
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

    const QJsonDocument json = QJsonDocument::fromVariant(map);
    const QByteArray data = json.toJson();

    if (data.isEmpty()) {
        qWarning() << "Bookmarks::saveBookmarks() Error serializing bookmarks!";
        return;
    }

    QSaveFile file(DataPaths::currentProfilePath() + QLatin1String("/bookmarks.json"));
    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "Bookmarks::saveBookmarks() Error opening bookmarks file for writing!";
        return;
    }

    file.write(data);
    file.commit();
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
            map.insert("url", child->urlString());
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

void Bookmarks::search(QList<BookmarkItem*>* items, BookmarkItem* parent, const QString &string, int limit, Qt::CaseSensitivity sensitive) const
{
    Q_ASSERT(items);
    Q_ASSERT(parent);

    if (limit == items->count()) {
        return;
    }

    switch (parent->type()) {
    case BookmarkItem::Root:
    case BookmarkItem::Folder:
        foreach (BookmarkItem* child, parent->children()) {
            search(items, child, string, limit, sensitive);
        }
        break;

    case BookmarkItem::Url:
        if (parent->title().contains(string, sensitive) ||
            parent->urlString().contains(string, sensitive) ||
            parent->description().contains(string, sensitive) ||
            parent->keyword().compare(string, sensitive) == 0
           ) {
            items->append(parent);
        }
        break;

    default:
        break;
    }
}

void Bookmarks::searchKeyword(QList<BookmarkItem*>* items, BookmarkItem* parent, const QString &keyword) const
{
    Q_ASSERT(items);
    Q_ASSERT(parent);

    switch (parent->type()) {
    case BookmarkItem::Root:
    case BookmarkItem::Folder:
        foreach (BookmarkItem* child, parent->children())
            searchKeyword(items, child, keyword);
        break;

    case BookmarkItem::Url:
        if (parent->keyword() == keyword)
            items->append(parent);
        break;

    default:
        break;
    }
}
