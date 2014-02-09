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
#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#define _bookmarksToolbar Bookmarks::toTranslatedFolder("bookmarksToolbar")
#define _bookmarksMenu Bookmarks::toTranslatedFolder("bookmarksMenu")
#define _bookmarksUnsorted Bookmarks::toTranslatedFolder("unsorted")

#include <QObject>
#include <QUrl>
#include <QImage>
#include <QVariant>

#include "qz_namespace.h"

class QIcon;

class WebView;
class BookmarkItem;
class BookmarksModel;

class QT_QUPZILLA_EXPORT Bookmarks : public QObject
{
    Q_OBJECT
public:
    explicit Bookmarks(QObject* parent = 0);

    struct Bookmark {
        int id;
        QString title;
        QString folder;
        QUrl url;
        QImage image;
        bool inSubfolder;

        Bookmark() : id(-1) , inSubfolder(false) { }

        bool operator==(const Bookmark &other) const {
            return (this->title == other.title &&
                    this->folder == other.folder &&
                    this->url == other.url &&
                    this->inSubfolder == other.inSubfolder);
        }
    };

    void loadSettings();
    void saveBookmarks();

    bool isShowingMostVisited() const;
    void setShowingMostVisited(bool state);

    bool isShowingOnlyIconsInToolbar() const;
    void setShowingOnlyIconsInToolbar(bool state);

    void setLastFolder(const QString &folder);
    void exportToHtml(const QString &fileName);

    static QString toTranslatedFolder(const QString &name);
    static QString fromTranslatedFolder(const QString &name);

    BookmarksModel* model() const;

    BookmarkItem* rootItem() const;
    BookmarkItem* toolbarFolder() const;
    BookmarkItem* menuFolder() const;
    BookmarkItem* unsortedFolder() const;
    BookmarkItem* lastUsedFolder() const;

    bool isBookmarked(const QUrl &url);

    // Search bookmarks (urls only) for exact url match
    QList<BookmarkItem*> searchBookmarks(const QUrl &url) const;
    // Search bookmarks (urls only) for contains match through all properties
    QList<BookmarkItem*> searchBookmarks(const QString &string, Qt::CaseSensitivity sensitive = Qt::CaseInsensitive) const;

    bool canBeModified(BookmarkItem* item) const;

    void addBookmark(BookmarkItem* parent, BookmarkItem* item);
    void insertBookmark(BookmarkItem* parent, int row, BookmarkItem* item);
    bool removeBookmark(BookmarkItem* item);

    void notifyBookmarkChanged(BookmarkItem* item);

signals:
    // Item was added to bookmarks
    void bookmarkAdded(BookmarkItem* item);
    // Item was removed from bookmarks
    void bookmarkRemoved(BookmarkItem* item);
    // Item data has changed
    void bookmarkChanged(BookmarkItem* item);

private:
    void loadBookmarks();

    void readBookmarks(const QVariantList &list, BookmarkItem* parent);
    QVariantList writeBookmarks(BookmarkItem* parent);

    void search(QList<BookmarkItem*>* items, BookmarkItem* parent, const QUrl &url) const;
    void search(QList<BookmarkItem*>* items, BookmarkItem* parent, const QString &string, Qt::CaseSensitivity sensitive) const;

    bool m_showMostVisited;
    bool m_showOnlyIconsInToolbar;

    BookmarkItem* m_root;
    BookmarkItem* m_folderToolbar;
    BookmarkItem* m_folderMenu;
    BookmarkItem* m_folderUnsorted;

    BookmarksModel* m_model;
};

typedef Bookmarks::Bookmark Bookmark;

// Hint to QVector to use std::realloc on item moving
Q_DECLARE_TYPEINFO(Bookmark, Q_MOVABLE_TYPE);

Q_DECLARE_METATYPE(Bookmark)

#endif // BOOKMARKS_H
