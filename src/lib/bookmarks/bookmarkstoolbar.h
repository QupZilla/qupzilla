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
#ifndef BOOKMARKSTOOLBAR_H
#define BOOKMARKSTOOLBAR_H

#include <QWidget>

#include "qz_namespace.h"

class QHBoxLayout;
class QTimer;

class QupZilla;
class Bookmarks;
class BookmarkItem;

class QT_QUPZILLA_EXPORT BookmarksToolbar : public QWidget
{
    Q_OBJECT
public:
    explicit BookmarksToolbar(QupZilla* mainClass, QWidget* parent = 0);

private slots:
    void contextMenuRequested(const QPoint &pos);
    void setShowOnlyIcons(bool show);

    void refresh();
    void bookmarksChanged();

    void openBookmarkInNewTab();
    void openBookmarkInNewWindow();
    void deleteBookmark();

private:
    void clear();
    void addItem(BookmarkItem* item);

    void dropEvent(QDropEvent* e);
    void dragEnterEvent(QDragEnterEvent* e);

    QupZilla* m_window;
    Bookmarks* m_bookmarks;
    BookmarkItem* m_clickedBookmark;
    QHBoxLayout* m_layout;
    QTimer* m_updateTimer;
};

#endif // BOOKMARKSTOOLBAR_H
