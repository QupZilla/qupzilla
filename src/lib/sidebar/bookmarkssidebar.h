/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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

#include "qz_namespace.h"
#include "bookmarksmodel.h"

namespace Ui
{
class BookmarksSideBar;
}

class QTreeWidgetItem;

class QupZilla;
class BookmarksModel;

class QT_QUPZILLA_EXPORT BookmarksSideBar : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksSideBar(QupZilla* mainClass, QWidget* parent = 0);
    ~BookmarksSideBar();
    void setMainWindow(QupZilla* window);

public slots:
    void refreshTable();

private slots:
    void deleteItem();
    void contextMenuRequested(const QPoint &position);
    void loadInNewTab();
    void itemControlClicked(QTreeWidgetItem* item);
    void itemDoubleClicked(QTreeWidgetItem* item);

    void copyAddress();

    void addBookmark(const BookmarksModel::Bookmark &bookmark);
    void removeBookmark(const BookmarksModel::Bookmark &bookmark);
    void bookmarkEdited(const BookmarksModel::Bookmark &before, const BookmarksModel::Bookmark &after);
    void addFolder(const QString &name);
    void removeFolder(const QString &name);
    void renameFolder(const QString &before, const QString &after);

    void changeBookmarkParent(const QString &name, const QByteArray &imageData, int id,
                              const QUrl &url, const QString &oldParent, const QString &newParent);
    void changeFolderParent(const QString &name, bool isSubfolder);

private:
    void keyPressEvent(QKeyEvent* event);

    QupZilla* getQupZilla();

    bool m_isRefreshing;
    Ui::BookmarksSideBar* ui;
    QupZilla* p_QupZilla;
    BookmarksModel* m_bookmarksModel;
};

#endif // BOOKMARKSSIDEBAR_H
