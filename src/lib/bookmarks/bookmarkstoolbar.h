/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "bookmarksmodel.h"

class QHBoxLayout;

class QupZilla;
class BookmarksModel;
class History;
class ToolButton;
class Menu;

class QT_QUPZILLA_EXPORT BookmarksToolbar : public QWidget
{
    Q_OBJECT
public:
    explicit BookmarksToolbar(QupZilla* mainClass, QWidget* parent = 0);

signals:

public slots:
    void refreshBookmarks();
    void refreshMostVisited();
    void showMostVisited();

private slots:
    void loadClickedBookmark();
    void loadClickedBookmarkInNewTab();
    void loadFolderBookmarksInTabs();

    void aboutToShowFolderMenu();
    void showBookmarkContextMenu(const QPoint &pos);
    void customContextMenuRequested(const QPoint &pos);

    void moveRight();
    void moveLeft();
    void editBookmark();
    void removeButton();

    void hidePanel();
    void toggleShowOnlyIcons();

    void addBookmark(const BookmarksModel::Bookmark &bookmark);
    void removeBookmark(const BookmarksModel::Bookmark &bookmark);
    void bookmarkEdited(const BookmarksModel::Bookmark &before, const BookmarksModel::Bookmark &after);
    void subfolderAdded(const QString &name);
    void folderDeleted(const QString &name);
    void folderRenamed(const QString &before, const QString &after);

    void changeBookmarkParent(const QString &name, const QByteArray &imageData, int id,
                              const QUrl &url, const QString &oldParent, const QString &newParent);
    void changeFolderParent(const QString &name, bool isSubfolder);

private:
    void dropEvent(QDropEvent* e);
    void dragEnterEvent(QDragEnterEvent* e);

    void showOnlyIconsChanged();
    int indexOfLastBookmark();

    QupZilla* p_QupZilla;
    BookmarksModel* m_bookmarksModel;
    History* m_historyModel;
    Menu* m_menuMostVisited;
    ToolButton* m_mostVis;
    QHBoxLayout* m_layout;

    Qt::ToolButtonStyle m_toolButtonStyle;
};

#endif // BOOKMARKSTOOLBAR_H
