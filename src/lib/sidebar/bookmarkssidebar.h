/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#ifndef BOOKMARKSSIDEBAR_H
#define BOOKMARKSSIDEBAR_H

#include <QWidget>

#include "qzcommon.h"

namespace Ui
{
class BookmarksSideBar;
}

class BrowserWindow;
class Bookmarks;
class BookmarkItem;

class QUPZILLA_EXPORT BookmarksSidebar : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksSidebar(BrowserWindow* window, QWidget* parent = 0);
    ~BookmarksSidebar();

private slots:
    void bookmarkActivated(BookmarkItem* item);
    void bookmarkCtrlActivated(BookmarkItem* item);
    void bookmarkShiftActivated(BookmarkItem* item);

    void openBookmark(BookmarkItem* item = 0);
    void openBookmarkInNewTab(BookmarkItem* item = 0);
    void openBookmarkInNewWindow(BookmarkItem* item = 0);
    void openBookmarkInNewPrivateWindow(BookmarkItem* item = 0);

    void deleteBookmarks();
    void createContextMenu(const QPoint &pos);

private:
    Ui::BookmarksSideBar* ui;
    BrowserWindow* m_window;
    Bookmarks* m_bookmarks;
};

#endif // BOOKMARKSSIDEBAR_H
