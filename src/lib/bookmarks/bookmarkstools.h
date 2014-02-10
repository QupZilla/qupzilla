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
#ifndef BOOKMARKSTOOLS_H
#define BOOKMARKSTOOLS_H

#include <QMenu>
#include <QPushButton>

#include "qz_namespace.h"

class BookmarkItem;

class QT_QUPZILLA_EXPORT BookmarksFoldersMenu : public QMenu
{
    Q_OBJECT

public:
    explicit BookmarksFoldersMenu(QWidget* parent = 0);

    BookmarkItem* selectedFolder() const;

signals:
    void folderSelected(BookmarkItem* folder);

private slots:
    void folderChoosed();

private:
    void init();
    void createMenu(QMenu* menu, BookmarkItem* parent);

    BookmarkItem* m_selectedFolder;
};

class QT_QUPZILLA_EXPORT BookmarksFoldersButton : public QPushButton
{
    Q_OBJECT

public:
    explicit BookmarksFoldersButton(QWidget* parent, BookmarkItem* folder = 0);

    BookmarkItem* selectedFolder() const;

signals:
    void selectedFolderChanged(BookmarkItem* folder);

public slots:
    void setSelectedFolder(BookmarkItem* folder);

private:
    void init();

    BookmarksFoldersMenu* m_menu;
    BookmarkItem* m_selectedFolder;
};

class QupZilla;
class TabWidget;
class Action;
class Menu;

class QT_QUPZILLA_EXPORT BookmarksTools
{
public:
    // Add Bookmark Dialogs
    static bool addBookmarkDialog(QWidget* parent, const QUrl &url, const QString &title, BookmarkItem* folder = 0);
    static bool bookmarkAllTabsDialog(QWidget* parent, TabWidget* tabWidget, BookmarkItem* folder = 0);

    // Open Bookmarks
    static void openBookmark(QupZilla* window, BookmarkItem* item);
    static void openBookmarkInNewTab(QupZilla* window, BookmarkItem* item);
    static void openBookmarkInNewWindow(BookmarkItem* item);
    static void openFolderInTabs(QupZilla* window, BookmarkItem* folder);

    // Create Menu
    static void addActionToMenu(QObject* receiver, Menu* menu, BookmarkItem* item);
    static void addFolderToMenu(QObject* receiver, Menu* menu, BookmarkItem* folder);
    static void addUrlToMenu(QObject* receiver, Menu* menu, BookmarkItem* bookmark);
    static void addSeparatorToMenu(Menu* menu, BookmarkItem* separator);
};

#endif // BOOKMARKSTOOLS_H
