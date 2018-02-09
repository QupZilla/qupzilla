/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "bookmarks.h"
#include "mainapplication.h"
#include "iconprovider.h"

#include <QDragEnterEvent>
#include <QHBoxLayout>
#include <QMimeData>
#include <QTimer>
#include <QFrame>

BookmarksToolbar::BookmarksToolbar(BrowserWindow* window, QWidget* parent)
    : QWidget(parent)
    , m_window(window)
    , m_bookmarks(mApp->bookmarks())
    , m_clickedBookmark(0)
{
    setObjectName("bookmarksbar");
    setAcceptDrops(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(style()->pixelMetric(QStyle::PM_ToolBarItemMargin, 0, this)
                          + style()->pixelMetric(QStyle::PM_ToolBarFrameWidth, 0, this));
    m_layout->setSpacing(style()->pixelMetric(QStyle::PM_ToolBarItemSpacing, 0, this));
    setLayout(m_layout);

    // Set some sane value
    setMinimumHeight(20);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(300);
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(refresh()));

    connect(m_bookmarks, SIGNAL(bookmarkAdded(BookmarkItem*)), this, SLOT(bookmarksChanged()));
    connect(m_bookmarks, SIGNAL(bookmarkRemoved(BookmarkItem*)), this, SLOT(bookmarksChanged()));
    connect(m_bookmarks, SIGNAL(bookmarkChanged(BookmarkItem*)), this, SLOT(bookmarksChanged()));
    connect(m_bookmarks, SIGNAL(showOnlyIconsInToolbarChanged(bool)), this, SLOT(showOnlyIconsChanged(bool)));
    connect(m_bookmarks, SIGNAL(showOnlyTextInToolbarChanged(bool)), this, SLOT(showOnlyTextChanged(bool)));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));

    refresh();
}

void BookmarksToolbar::contextMenuRequested(const QPoint &pos)
{
    BookmarksToolbarButton* button = buttonAt(pos);
    m_clickedBookmark = button ? button->bookmark() : 0;

    QMenu menu;
    QAction* actNewTab = menu.addAction(IconProvider::newTabIcon(), tr("Open in new tab"));
    QAction* actNewWindow = menu.addAction(IconProvider::newWindowIcon(), tr("Open in new window"));
    QAction* actNewPrivateWindow = menu.addAction(IconProvider::privateBrowsingIcon(), tr("Open in new private window"));
    menu.addSeparator();
    QAction* actEdit = menu.addAction(tr("Edit"));
    QAction* actDelete = menu.addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));
    menu.addSeparator();
    m_actShowOnlyIcons = menu.addAction(tr("Show Only Icons"));
    m_actShowOnlyIcons->setCheckable(true);
    m_actShowOnlyIcons->setChecked(m_bookmarks->showOnlyIconsInToolbar());
    connect(m_actShowOnlyIcons, SIGNAL(toggled(bool)), m_bookmarks, SLOT(setShowOnlyIconsInToolbar(bool)));
    m_actShowOnlyText = menu.addAction(tr("Show Only Text"));
    m_actShowOnlyText->setCheckable(true);
    m_actShowOnlyText->setChecked(m_bookmarks->showOnlyTextInToolbar());
    connect(m_actShowOnlyText, SIGNAL(toggled(bool)), m_bookmarks, SLOT(setShowOnlyTextInToolbar(bool)));

    connect(actNewTab, SIGNAL(triggered()), this, SLOT(openBookmarkInNewTab()));
    connect(actNewWindow, SIGNAL(triggered()), this, SLOT(openBookmarkInNewWindow()));
    connect(actNewPrivateWindow, SIGNAL(triggered()), this, SLOT(openBookmarkInNewPrivateWindow()));
    connect(actEdit, SIGNAL(triggered()), this, SLOT(editBookmark()));
    connect(actDelete, SIGNAL(triggered()), this, SLOT(deleteBookmark()));

    actEdit->setEnabled(m_clickedBookmark && m_bookmarks->canBeModified(m_clickedBookmark));
    actDelete->setEnabled(m_clickedBookmark && m_bookmarks->canBeModified(m_clickedBookmark));
    actNewTab->setEnabled(m_clickedBookmark && m_clickedBookmark->isUrl());
    actNewWindow->setEnabled(m_clickedBookmark && m_clickedBookmark->isUrl());
    actNewPrivateWindow->setEnabled(m_clickedBookmark && m_clickedBookmark->isUrl());

    menu.exec(mapToGlobal(pos));

    if (button) {
        // Clear mouseover state after closing menu
        button->update();
    }

    m_clickedBookmark = nullptr;
    m_actShowOnlyIcons = nullptr;
    m_actShowOnlyText = nullptr;
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
    m_updateTimer->start();
}

void BookmarksToolbar::showOnlyIconsChanged(bool state)
{
    if (state && m_actShowOnlyText) {
        m_actShowOnlyText->setChecked(false);
    }

    for (int i = 0; i < m_layout->count(); ++i) {
        BookmarksToolbarButton* b = qobject_cast<BookmarksToolbarButton*>(m_layout->itemAt(i)->widget());
        if (b) {
            b->setShowOnlyIcon(state);
        }
    }
}

void BookmarksToolbar::showOnlyTextChanged(bool state)
{
    if (state && m_actShowOnlyIcons) {
        m_actShowOnlyIcons->setChecked(false);
    }

    for (int i = 0; i < m_layout->count(); ++i) {
        BookmarksToolbarButton* b = qobject_cast<BookmarksToolbarButton*>(m_layout->itemAt(i)->widget());
        if (b) {
            b->setShowOnlyText(state);
        }
    }
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

void BookmarksToolbar::openBookmarkInNewPrivateWindow()
{
    if (m_clickedBookmark) {
        BookmarksTools::openBookmarkInNewPrivateWindow(m_clickedBookmark);
    }
}

void BookmarksToolbar::editBookmark()
{
    if (m_clickedBookmark) {
        BookmarksTools::editBookmarkDialog(this, m_clickedBookmark);
        m_bookmarks->changeBookmark(m_clickedBookmark);
    }
}

void BookmarksToolbar::deleteBookmark()
{
    if (m_clickedBookmark) {
        m_bookmarks->removeBookmark(m_clickedBookmark);
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

void BookmarksToolbar::addItem(BookmarkItem* item)
{
    Q_ASSERT(item);

    BookmarksToolbarButton* button = new BookmarksToolbarButton(item, this);
    button->setMainWindow(m_window);
    button->setShowOnlyIcon(m_bookmarks->showOnlyIconsInToolbar());
    button->setShowOnlyText(m_bookmarks->showOnlyTextInToolbar());
    m_layout->addWidget(button);

    if (!m_fixedMinHeight) {
        m_fixedMinHeight = true;
        setMinimumHeight(minimumSizeHint().height());
    }
}

BookmarksToolbarButton* BookmarksToolbar::buttonAt(const QPoint &pos)
{
    return qobject_cast<BookmarksToolbarButton*>(QApplication::widgetAt(mapToGlobal(pos)));
}

void BookmarksToolbar::dropEvent(QDropEvent* e)
{
    const QMimeData* mime = e->mimeData();

    if (!mime->hasUrls()) {
        QWidget::dropEvent(e);
        return;
    }

    QUrl url = mime->urls().at(0);
    QString title = mime->hasText() ? mime->text() : url.toEncoded(QUrl::RemoveScheme);

    BookmarkItem* parent = m_bookmarks->toolbarFolder();
    BookmarksToolbarButton* button = buttonAt(e->pos());
    if (button && button->bookmark()->isFolder()) {
        parent = button->bookmark();
    }

    BookmarkItem* bookmark = new BookmarkItem(BookmarkItem::Url);
    bookmark->setTitle(title);
    bookmark->setUrl(url);
    m_bookmarks->addBookmark(parent, bookmark);
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
