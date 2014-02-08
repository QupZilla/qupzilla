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
#ifndef BOOKMARKSMANAGER_H
#define BOOKMARKSMANAGER_H

#include <QWidget>
#include <QPointer>

#include "qz_namespace.h"

namespace Ui
{
class BookmarksManager;
}

class QUrl;

class WebView;
class QupZilla;
class Bookmarks;
class BookmarkItem;

class QT_QUPZILLA_EXPORT BookmarksManager : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksManager(QupZilla* mainClass, QWidget* parent = 0);
    ~BookmarksManager();

public slots:

private slots:
    void bookmarkActivated(BookmarkItem* item);
    void bookmarkCtrlActivated(BookmarkItem* item);
    void bookmarkShiftActivated(BookmarkItem* item);
    void bookmarksSelected(const QList<BookmarkItem*> &items);

    void bookmarkEdited();
    void descriptionEdited();

    void importBookmarks();
    void exportBookmarks();

private:
    void updateEditBox(BookmarkItem* item);

    QupZilla* getQupZilla();

    Ui::BookmarksManager* ui;
    QPointer<QupZilla> p_QupZilla;

    Bookmarks* m_bookmarks;
    bool m_blockDescriptionChangedSignal;

public:
    void addBookmark(WebView* view);
    void insertBookmark(const QUrl &url, const QString &title, const QIcon &icon, const QString &folder = QString());
    void setMainWindow(QupZilla* window);
    void search(const QString &string);
    void insertAllTabs();
    void refreshTable() {}
};
#endif // BOOKMARKSMANAGER_H
