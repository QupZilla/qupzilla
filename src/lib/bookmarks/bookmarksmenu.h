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
#ifndef BOOKMARKSMENU_H
#define BOOKMARKSMENU_H

#include <QPointer>

#include "enhancedmenu.h"
#include "qzcommon.h"

class BrowserWindow;
class BookmarkItem;

class QUPZILLA_EXPORT BookmarksMenu : public Menu
{
    Q_OBJECT

public:
    explicit BookmarksMenu(QWidget* parent = 0);

    void setMainWindow(BrowserWindow* window);

private slots:
    void bookmarkPage();
    void bookmarkAllTabs();
    void showBookmarksManager();

    void bookmarksChanged();
    void aboutToShow();
    void menuAboutToShow();
    void menuMiddleClicked(Menu* menu);

    void bookmarkActivated();
    void bookmarkCtrlActivated();
    void bookmarkShiftActivated();

    void openFolder(BookmarkItem* item);
    void openBookmark(BookmarkItem* item);
    void openBookmarkInNewTab(BookmarkItem* item);
    void openBookmarkInNewWindow(BookmarkItem* item);

private:
    void init();
    void refresh();

    QPointer<BrowserWindow> m_window;
    bool m_changed;
};

#endif // BOOKMARKSMENU_H
