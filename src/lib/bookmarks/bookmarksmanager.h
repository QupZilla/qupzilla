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
#ifndef BOOKMARKSMANAGER_H
#define BOOKMARKSMANAGER_H

#include <QWidget>
#include <QPointer>

#include "qzcommon.h"

namespace Ui
{
class BookmarksManager;
}

class QUrl;

class BrowserWindow;
class Bookmarks;
class BookmarkItem;

class QUPZILLA_EXPORT BookmarksManager : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksManager(BrowserWindow* window, QWidget* parent = 0);
    ~BookmarksManager();

    void setMainWindow(BrowserWindow* window);

public slots:
    void search(const QString &string);

private slots:
    void bookmarkActivated(BookmarkItem* item);
    void bookmarkCtrlActivated(BookmarkItem* item);
    void bookmarkShiftActivated(BookmarkItem* item);
    void bookmarksSelected(const QList<BookmarkItem*> &items);
    void createContextMenu(const QPoint &pos);

    void openBookmark(BookmarkItem* item = 0);
    void openBookmarkInNewTab(BookmarkItem* item = 0);
    void openBookmarkInNewWindow(BookmarkItem* item = 0);
    void openBookmarkInNewPrivateWindow(BookmarkItem* item = 0);

    void addBookmark();
    void addFolder();
    void addSeparator();
    void deleteBookmarks();

    void bookmarkEdited();
    void descriptionEdited();
    void enableUpdates();

private:
    void updateEditBox(BookmarkItem* item);
    bool bookmarkEditable(BookmarkItem* item) const;
    void addBookmark(BookmarkItem* item);
    BookmarkItem* parentForNewBookmark() const;
    BrowserWindow* getQupZilla();

    void showEvent(QShowEvent* event);
    void keyPressEvent(QKeyEvent* event);

    Ui::BookmarksManager* ui;
    QPointer<BrowserWindow> m_window;

    Bookmarks* m_bookmarks;
    BookmarkItem* m_selectedBookmark;
    bool m_blockDescriptionChangedSignal;
    bool m_adjustHeaderSizesOnShow;

};

#endif // BOOKMARKSMANAGER_H
