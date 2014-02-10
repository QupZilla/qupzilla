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

#include <QObject>
#include <QVariant>

#include "qz_namespace.h"

class QUrl;

class BookmarkItem;
class BookmarksModel;

class QT_QUPZILLA_EXPORT Bookmarks : public QObject
{
    Q_OBJECT
public:
    explicit Bookmarks(QObject* parent = 0);
    ~Bookmarks();

    void loadSettings();
    void saveSettings();

    bool showOnlyIconsInToolbar() const;
    void exportToHtml(const QString &fileName);

    BookmarkItem* rootItem() const;
    BookmarkItem* toolbarFolder() const;
    BookmarkItem* menuFolder() const;
    BookmarkItem* unsortedFolder() const;
    BookmarkItem* lastUsedFolder() const;

    BookmarksModel* model() const;

    bool isBookmarked(const QUrl &url);
    bool canBeModified(BookmarkItem* item) const;

    // Search bookmarks (urls only) for exact url match
    QList<BookmarkItem*> searchBookmarks(const QUrl &url) const;
    // Search bookmarks (urls only) for contains match through all properties
    QList<BookmarkItem*> searchBookmarks(const QString &string, Qt::CaseSensitivity sensitive = Qt::CaseInsensitive) const;

    void addBookmark(BookmarkItem* parent, BookmarkItem* item);
    void insertBookmark(BookmarkItem* parent, int row, BookmarkItem* item);
    bool removeBookmark(BookmarkItem* item);

    void notifyBookmarkChanged(BookmarkItem* item);

public slots:
    void setShowOnlyIconsInToolbar(bool state);

signals:
    // Item was added to bookmarks
    void bookmarkAdded(BookmarkItem* item);
    // Item was removed from bookmarks
    void bookmarkRemoved(BookmarkItem* item);
    // Item data has changed
    void bookmarkChanged(BookmarkItem* item);

    void showOnlyIconsInToolbarChanged(bool show);

private:
    void init();
    void saveBookmarks();

    void loadBookmarksFromMap(const QVariantMap &map);
    void readBookmarks(const QVariantList &list, BookmarkItem* parent);
    QVariantList writeBookmarks(BookmarkItem* parent);

    void search(QList<BookmarkItem*>* items, BookmarkItem* parent, const QUrl &url) const;
    void search(QList<BookmarkItem*>* items, BookmarkItem* parent, const QString &string, Qt::CaseSensitivity sensitive) const;

    BookmarkItem* m_root;
    BookmarkItem* m_folderToolbar;
    BookmarkItem* m_folderMenu;
    BookmarkItem* m_folderUnsorted;

    BookmarkItem* m_lastFolder;
    BookmarksModel* m_model;

    bool m_showOnlyIconsInToolbar;
};

#endif // BOOKMARKS_H
