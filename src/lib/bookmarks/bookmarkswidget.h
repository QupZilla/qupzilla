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
#ifndef BOOKMARKSWIDGET_H
#define BOOKMARKSWIDGET_H

#include <QMenu>

#include "qzcommon.h"
#include "locationbarpopup.h"

namespace Ui
{
class BookmarksWidget;
}

class WebView;
class SpeedDial;
class Bookmarks;
class BookmarkItem;

class QUPZILLA_EXPORT BookmarksWidget : public LocationBarPopup
{
    Q_OBJECT
public:
    explicit BookmarksWidget(WebView* view, BookmarkItem* bookmark, QWidget* parent = 0);
    ~BookmarksWidget();

private slots:
    void toggleSpeedDial();
    void toggleBookmark();
    void bookmarkEdited();

private:
    void init();
    void closePopup();

    Ui::BookmarksWidget* ui;
    WebView* m_view;
    BookmarkItem* m_bookmark;

    Bookmarks* m_bookmarks;
    SpeedDial* m_speedDial;
    bool m_edited;
};

#endif // BOOKMARKSWIDGET_H
