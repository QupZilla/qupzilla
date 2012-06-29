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
#include "bookmarkicon.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "tabbedwebview.h"
#include "locationbar.h"
#include "bookmarksmodel.h"
#include "bookmarkswidget.h"
#include "pluginproxy.h"
#include "speeddial.h"

#include <QStyle>

BookmarkIcon::BookmarkIcon(QupZilla* mainClass, QWidget* parent)
    : ClickableLabel(parent)
    , p_QupZilla(mainClass)
    , m_bookmarksModel(0)
    , m_speedDial(mApp->plugins()->speedDial())
{
    setObjectName("locationbar-bookmarkicon");
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("Bookmark this Page"));
    setFocusPolicy(Qt::ClickFocus);

    m_bookmarksModel = mApp->bookmarksModel();
    connect(this, SIGNAL(clicked(QPoint)), this, SLOT(iconClicked()));
    connect(m_bookmarksModel, SIGNAL(bookmarkAdded(BookmarksModel::Bookmark)), this, SLOT(bookmarkAdded(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkDeleted(BookmarksModel::Bookmark)), this, SLOT(bookmarkDeleted(BookmarksModel::Bookmark)));
    connect(m_speedDial, SIGNAL(pagesChanged()), this, SLOT(speedDialChanged()));
}

void BookmarkIcon::iconClicked()
{
    BookmarksWidget* menu = new BookmarksWidget(p_QupZilla->weView(), p_QupZilla->locationBar());
    menu->showAt(this);
}

void BookmarkIcon::checkBookmark(const QUrl &url, bool forceCheck)
{
    if (!forceCheck && m_lastUrl == url) {
        return;
    }

    if (m_bookmarksModel->isBookmarked(url) || !m_speedDial->pageForUrl(url).url.isEmpty()) {
        setBookmarkSaved();
    }
    else {
        setBookmarkDisabled();
    }

    m_lastUrl = url;
}

void BookmarkIcon::bookmarkDeleted(const BookmarksModel::Bookmark &bookmark)
{
    if (bookmark.url == m_lastUrl) {
        checkBookmark(m_lastUrl, true);
    }
}

void BookmarkIcon::bookmarkAdded(const BookmarksModel::Bookmark &bookmark)
{
    if (bookmark.url == m_lastUrl) {
        checkBookmark(m_lastUrl, true);
    }
}

void BookmarkIcon::speedDialChanged()
{
    checkBookmark(m_lastUrl, true);
}

void BookmarkIcon::setBookmarkSaved()
{
    setProperty("bookmarked", QVariant(true));
    style()->unpolish(this);
    style()->polish(this);
    setToolTip(tr("Edit this bookmark"));
}

void BookmarkIcon::setBookmarkDisabled()
{
    setProperty("bookmarked", QVariant(false));
    style()->unpolish(this);
    style()->polish(this);
    setToolTip(tr("Bookmark this Page"));
}

void BookmarkIcon::contextMenuEvent(QContextMenuEvent* ev)
{
    // Prevent propagating to LocationBar
    ev->accept();
}

void BookmarkIcon::mousePressEvent(QMouseEvent* ev)
{
    ClickableLabel::mousePressEvent(ev);

    // Prevent propagating to LocationBar
    ev->accept();
}
