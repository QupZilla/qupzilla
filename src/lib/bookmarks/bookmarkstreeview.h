/* ============================================================
* QupZilla - WebKit based browser
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

#include "qz_namespace.h"

class Bookmarks;
class BookmarkItem;
class BookmarksModel;

class QT_QUPZILLA_EXPORT BookmarksTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit BookmarksTreeView(QWidget* parent = 0);

    QList<BookmarkItem*> selectedBookmarks() const;

signals:
    // Open bookmark in current tab
    void bookmarkActivated(BookmarkItem* item);
    // Open bookmark in new tab
    void bookmarkCtrlActivated(BookmarkItem* item);
    // Open bookmark in new window
    void bookmarkShiftActivated(BookmarkItem* item);

    void contextMenuRequested(BookmarkItem* item);
    void bookmarksSelected(QList<BookmarkItem*> items);

private slots:
    void indexExpanded(const QModelIndex &parent);
    void indexCollapsed(const QModelIndex &parent);

    void selectionChanged();
    void createContextMenu(const QPoint &point);

private:
    void restoreExpandedState(const QModelIndex &parent);
    void rowsInserted(const QModelIndex &parent, int start, int end);

    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);

    Bookmarks* m_bookmarks;
    BookmarksModel* m_model;

};

#endif // BOOKMARKSTREEVIEW_H
