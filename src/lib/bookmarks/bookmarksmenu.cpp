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
#include "bookmarksmenu.h"
#include "bookmarkstools.h"
#include "bookmarkitem.h"
#include "bookmarks.h"
#include "mainapplication.h"
#include "browsinglibrary.h"
#include "browserwindow.h"
#include "qzsettings.h"
#include "tabwidget.h"

BookmarksMenu::BookmarksMenu(QWidget* parent)
    : Menu(parent)
    , m_window(0)
    , m_changed(true)
{
    init();

    connect(mApp->bookmarks(), SIGNAL(bookmarkAdded(BookmarkItem*)), this, SLOT(bookmarksChanged()));
    connect(mApp->bookmarks(), SIGNAL(bookmarkRemoved(BookmarkItem*)), this, SLOT(bookmarksChanged()));
    connect(mApp->bookmarks(), SIGNAL(bookmarkChanged(BookmarkItem*)), this, SLOT(bookmarksChanged()));
}

void BookmarksMenu::setMainWindow(BrowserWindow* window)
{
    Q_ASSERT(window);

    m_window = window;
}

void BookmarksMenu::bookmarkPage()
{
    if (m_window) {
        m_window->bookmarkPage();
    }
}

void BookmarksMenu::bookmarkAllTabs()
{
    if (m_window) {
        BookmarksTools::bookmarkAllTabsDialog(m_window, m_window->tabWidget());
    }
}

void BookmarksMenu::showBookmarksManager()
{
    if (m_window) {
        mApp->browsingLibrary()->showBookmarks(m_window);
    }
}

void BookmarksMenu::bookmarksChanged()
{
    m_changed = true;
}

void BookmarksMenu::aboutToShow()
{
    Menu* menu = qobject_cast<Menu*>(sender());
    if (!menu) {
        return;
    }

    // root menu
    if (menu == this) {
        if (mApp->bookmarks()->rootItem()->isChanged() || mApp->bookmarks()->menuFolder()->isChanged()) {
            refresh();
            mApp->bookmarks()->menuFolder()->setChanged(false);
            mApp->bookmarks()->rootItem()->setChanged(false);
        }
        return;
    }

    // sub menus
    BookmarkItem* folder = static_cast<BookmarkItem*>(menu->menuAction()->data().value<void*>());

    if (!folder || !folder->isFolder() || !folder->isChanged()) {
        return;
    }

    menu->clear();
    foreach(BookmarkItem* child, folder->children()) {
        BookmarksTools::addActionToMenu(this, menu, child);
        if (child->isFolder()) {
            child->setChanged(true);
        }
    }

    if (menu->isEmpty()) {
        menu->addAction(Bookmarks::tr("Empty"))->setDisabled(true);
    }

    folder->setChanged(false);
}

void BookmarksMenu::menuMiddleClicked(Menu* menu)
{
    BookmarkItem* item = static_cast<BookmarkItem*>(menu->menuAction()->data().value<void*>());
    Q_ASSERT(item);
    openFolder(item);
}

void BookmarksMenu::bookmarkActivated()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        BookmarkItem* item = static_cast<BookmarkItem*>(action->data().value<void*>());
        Q_ASSERT(item);
        openBookmark(item);
    }
}

void BookmarksMenu::bookmarkCtrlActivated()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        BookmarkItem* item = static_cast<BookmarkItem*>(action->data().value<void*>());
        Q_ASSERT(item);
        openBookmarkInNewTab(item);
    }
}

void BookmarksMenu::bookmarkShiftActivated()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        BookmarkItem* item = static_cast<BookmarkItem*>(action->data().value<void*>());
        Q_ASSERT(item);
        openBookmarkInNewWindow(item);
    }
}

void BookmarksMenu::openFolder(BookmarkItem* item)
{
    Q_ASSERT(item->isFolder());

    if (m_window) {
        BookmarksTools::openFolderInTabs(m_window, item);
    }
}

void BookmarksMenu::openBookmark(BookmarkItem* item)
{
    Q_ASSERT(item->isUrl());

    if (m_window) {
        BookmarksTools::openBookmark(m_window, item);
    }
}

void BookmarksMenu::openBookmarkInNewTab(BookmarkItem* item)
{
    Q_ASSERT(item->isUrl());

    if (m_window) {
        BookmarksTools::openBookmarkInNewTab(m_window, item);
    }
}

void BookmarksMenu::openBookmarkInNewWindow(BookmarkItem* item)
{
    Q_ASSERT(item->isUrl());

    BookmarksTools::openBookmarkInNewWindow(item);
}

void BookmarksMenu::init()
{
    setTitle(tr("&Bookmarks"));

    addAction(QIcon::fromTheme("bookmark-new"), tr("Bookmark &This Page"), this, SLOT(bookmarkPage()))->setShortcut(QKeySequence("Ctrl+D"));
    addAction(QIcon::fromTheme("bookmark-new-list"), tr("Bookmark &All Tabs"), this, SLOT(bookmarkAllTabs()));
    addAction(QIcon::fromTheme("bookmarks-organize"), tr("Organize &Bookmarks"), this, SLOT(showBookmarksManager()))->setShortcut(QKeySequence("Ctrl+Shift+O"));
    addSeparator();

    connect(this, SIGNAL(aboutToShow()), this, SLOT(aboutToShow()));
    connect(this, SIGNAL(menuMiddleClicked(Menu*)), this, SLOT(menuMiddleClicked(Menu*)));
}

void BookmarksMenu::refresh()
{
    while (actions().count() != 4) {
        QAction* act = actions().at(4);
        if (act->menu()) {
            act->menu()->clear();
        }
        removeAction(act);
        delete act;
    }

    BookmarksTools::addActionToMenu(this, this, mApp->bookmarks()->toolbarFolder());
    mApp->bookmarks()->toolbarFolder()->setChanged(true);
    addSeparator();

    foreach (BookmarkItem* child, mApp->bookmarks()->menuFolder()->children()) {
        BookmarksTools::addActionToMenu(this, this, child);
    }

    addSeparator();
    BookmarksTools::addActionToMenu(this, this, mApp->bookmarks()->unsortedFolder());
    mApp->bookmarks()->unsortedFolder()->setChanged(true);
}
