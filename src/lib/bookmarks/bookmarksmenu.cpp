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
#include "iconprovider.h"
#include "qupzilla.h"
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

void BookmarksMenu::setMainWindow(QupZilla* window)
{
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
    if (m_changed) {
        refresh();
        m_changed = false;
    }
}

void BookmarksMenu::loadMenu(Menu* menu)
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

    foreach (BookmarkItem* child, item->children()) {
        if (child->isUrl()) {
            openBookmarkInNewTab(child);
        }
        else if (child->isFolder()) {
            openFolder(child);
        }
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
    addAction(qIconProvider->fromTheme("bookmarks-organize"), tr("Organize &Bookmarks"), this, SLOT(showBookmarksManager()))->setShortcut(QKeySequence("Ctrl+Shift+O"));
    addSeparator();

    connect(this, SIGNAL(aboutToShow()), this, SLOT(aboutToShow()));
    connect(this, SIGNAL(menuMiddleClicked(Menu*)), this, SLOT(loadMenu(Menu*)));
}

#define FOLDER_ICON QApplication::style()->standardIcon(QStyle::SP_DirIcon)

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

    addItem(this, mApp->bookmarks()->toolbarFolder());
    addSeparator();

    foreach (BookmarkItem* child, mApp->bookmarks()->menuFolder()->children()) {
        addItem(this, child);
    }

    addSeparator();
    addItem(this, mApp->bookmarks()->unsortedFolder());
}

void BookmarksMenu::addItem(Menu* menu, BookmarkItem* item)
{
    Q_ASSERT(menu);
    Q_ASSERT(item);

    switch (item->type()) {
    case BookmarkItem::Url:
        addBookmark(menu, item);
        break;
    case BookmarkItem::Folder:
        addFolder(menu, item);
        break;
    case BookmarkItem::Separator:
        menu->addSeparator();
        break;
    default:
        break;
    }
}

void BookmarksMenu::addFolder(Menu* menu, BookmarkItem* folder)
{
    Menu* m = new Menu(folder->title());
    m->setIcon(FOLDER_ICON);
    connect(m, SIGNAL(menuMiddleClicked(Menu*)), this, SLOT(loadMenu(Menu*)));

    QAction* act = menu->addMenu(m);
    act->setData(QVariant::fromValue<void*>(static_cast<void*>(folder)));

    foreach (BookmarkItem* child, folder->children()) {
        addItem(m, child);
    }
}

void BookmarksMenu::addBookmark(Menu* menu, BookmarkItem* bookmark)
{
    Action* act = new Action(_iconForUrl(bookmark->url()), bookmark->title());
    act->setData(QVariant::fromValue<void*>(static_cast<void*>(bookmark)));

    connect(act, SIGNAL(triggered()), this, SLOT(bookmarkActivated()));
    connect(act, SIGNAL(ctrlTriggered()), this, SLOT(bookmarkCtrlActivated()));
    connect(act, SIGNAL(shiftTriggered()), this, SLOT(bookmarkShiftActivated()));

    menu->addAction(act);
}
