/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#ifndef BOOKMARKSTREEVIEW_H
#define BOOKMARKSTREEVIEW_H

#include <QTreeView>

#include "qzcommon.h"

class Bookmarks;
class BookmarkItem;
class BookmarksModel;
class BookmarksFilterModel;

class QUPZILLA_EXPORT BookmarksTreeView : public QTreeView
{
    Q_OBJECT

public:
    enum ViewType {
        BookmarksManagerViewType,
        BookmarksSidebarViewType
    };

    explicit BookmarksTreeView(QWidget* parent = 0);

    ViewType viewType() const;
    void setViewType(ViewType type);

    // Returns null if more than one (or zero) bookmarks are selected
    BookmarkItem* selectedBookmark() const;
    // Returns all selected bookmarks
    QList<BookmarkItem*> selectedBookmarks() const;

    void selectBookmark(BookmarkItem* item);
    // Expand up to root item
    void ensureBookmarkVisible(BookmarkItem* item);

public slots:
    void search(const QString &string);

signals:
    // Open bookmark in current tab
    void bookmarkActivated(BookmarkItem* item);
    // Open bookmark in new tab
    void bookmarkCtrlActivated(BookmarkItem* item);
    // Open bookmark in new window
    void bookmarkShiftActivated(BookmarkItem* item);
    // Context menu signal with point mapped to global
    void contextMenuRequested(const QPoint &point);
    // If all bookmarks have been deselected, items is empty
    void bookmarksSelected(QList<BookmarkItem*> items);

private slots:
    void indexExpanded(const QModelIndex &parent);
    void indexCollapsed(const QModelIndex &parent);

private:
    void restoreExpandedState(const QModelIndex &parent);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void contextMenuEvent(QContextMenuEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);

    Bookmarks* m_bookmarks;
    BookmarksModel* m_model;
    BookmarksFilterModel* m_filter;
    ViewType m_type;
};

#endif // BOOKMARKSTREEVIEW_H
