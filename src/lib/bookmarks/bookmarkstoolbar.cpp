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
#include "bookmarkstoolbar.h"
#include "bookmarkstoolbarbutton.h"
#include "bookmarkstools.h"
#include "bookmarkitem.h"
#include "mainapplication.h"
#include "bookmarks.h"

#include <QDragEnterEvent>
#include <QHBoxLayout>
#include <QMimeData>
#include <QTimer>
#include <QFrame>

BookmarksToolbar::BookmarksToolbar(QupZilla* mainClass, QWidget* parent)
    : QWidget(parent)
    , m_window(mainClass)
    , m_bookmarks(mApp->bookmarks())
    , m_clickedBookmark(0)
{
    setObjectName("bookmarksbar");
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);

    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(1);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(300);
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(refresh()));

    connect(m_bookmarks, SIGNAL(bookmarkAdded(BookmarkItem*)), this, SLOT(bookmarksChanged()));
    connect(m_bookmarks, SIGNAL(bookmarkRemoved(BookmarkItem*)), this, SLOT(bookmarksChanged()));
    connect(m_bookmarks, SIGNAL(bookmarkChanged(BookmarkItem*)), this, SLOT(bookmarksChanged()));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));

    refresh();
}

void BookmarksToolbar::contextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = mapToGlobal(pos);
    BookmarksToolbarButton* button = qobject_cast<BookmarksToolbarButton*>(QApplication::widgetAt(globalPos));
    m_clickedBookmark = button ? button->bookmark() : 0;

    QMenu menu;
    QAction* actNewTab = menu.addAction(QIcon::fromTheme("tab-new", QIcon(":/icons/menu/tab-new.png")), tr("Open in new tab"));
    QAction* actNewWindow = menu.addAction(QIcon::fromTheme("window-new"), tr("Open in new window"));
    menu.addSeparator();
    QAction* actDelete = menu.addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));
    menu.addSeparator();
    QAction* act = menu.addAction(tr("Show Only Icons"));
    act->setCheckable(true);
    act->setChecked(m_bookmarks->isShowingOnlyIconsInToolbar());
    connect(act, SIGNAL(toggled(bool)), this, SLOT(setShowOnlyIcons(bool)));

    connect(actNewTab, SIGNAL(triggered()), this, SLOT(openBookmarkInNewTab()));
    connect(actNewWindow, SIGNAL(triggered()), this, SLOT(openBookmarkInNewWindow()));
    connect(actDelete, SIGNAL(triggered()), this, SLOT(deleteBookmark()));

    actDelete->setEnabled(m_clickedBookmark && (m_clickedBookmark->isUrl() || m_clickedBookmark->isFolder()));
    actNewTab->setEnabled(m_clickedBookmark && m_clickedBookmark->isUrl());
    actNewWindow->setEnabled(m_clickedBookmark && m_clickedBookmark->isUrl());

    menu.exec(globalPos);

    m_clickedBookmark = 0;
}

void BookmarksToolbar::setShowOnlyIcons(bool show)
{
    m_bookmarks->setShowingOnlyIconsInToolbar(show);

    for (int i = 0; i < m_layout->count(); ++i) {
        BookmarksToolbarButton* b = qobject_cast<BookmarksToolbarButton*>(m_layout->itemAt(i)->widget());

        if (b) {
            b->setShowOnlyIcon(show);
        }
    }
}

void BookmarksToolbar::addItem(BookmarkItem* item)
{
    Q_ASSERT(item);

    switch (item->type()) {
    case BookmarkItem::Folder:
    case BookmarkItem::Url: {
        BookmarksToolbarButton* button = new BookmarksToolbarButton(item, this);
        button->setMainWindow(m_window);
        button->setShowOnlyIcon(m_bookmarks->isShowingOnlyIconsInToolbar());
        m_layout->addWidget(button);
        break;
    }

    case BookmarkItem::Separator: {
        QFrame* separator = new QFrame(this);
        separator->setFrameShape(QFrame::VLine);
        m_layout->addWidget(separator);
        break;
    }

    default:
        break;
    }
}

void BookmarksToolbar::clear()
{
    int count = m_layout->count();

    for (int i = 0; i < count; ++i) {
        QLayoutItem* item = m_layout->takeAt(0);
        delete item->widget();
        delete item;
    }

    Q_ASSERT(m_layout->isEmpty());
}

void BookmarksToolbar::refresh()
{
    clear();

    BookmarkItem* folder = mApp->bookmarks()->toolbarFolder();

    foreach (BookmarkItem* child, folder->children()) {
        addItem(child);
    }

    m_layout->addStretch();
}

void BookmarksToolbar::bookmarksChanged()
{
    m_updateTimer->stop();
    m_updateTimer->start();
}

void BookmarksToolbar::openBookmarkInNewTab()
{
    if (m_clickedBookmark) {
        BookmarksTools::openBookmarkInNewTab(m_window, m_clickedBookmark);
    }
}

void BookmarksToolbar::openBookmarkInNewWindow()
{
    if (m_clickedBookmark) {
        BookmarksTools::openBookmarkInNewWindow(m_clickedBookmark);
    }
}

void BookmarksToolbar::deleteBookmark()
{
    if (m_clickedBookmark) {
        m_bookmarks->removeBookmark(m_clickedBookmark);
    }
}

void BookmarksToolbar::dropEvent(QDropEvent* e)
{
    const QMimeData* mime = e->mimeData();

    if (!mime->hasUrls() || !mime->hasText()) {
        QWidget::dropEvent(e);
        return;
    }

    QString title = mime->text();
    QUrl url = mime->urls().at(0);

    BookmarkItem* bookmark = new BookmarkItem(BookmarkItem::Url);
    bookmark->setTitle(title);
    bookmark->setUrl(url);
    m_bookmarks->addBookmark(m_bookmarks->toolbarFolder(), bookmark);
}

void BookmarksToolbar::dragEnterEvent(QDragEnterEvent* e)
{
    const QMimeData* mime = e->mimeData();

    if (mime->hasUrls() && mime->hasText()) {
        e->acceptProposedAction();
        return;
    }

    QWidget::dragEnterEvent(e);
}
