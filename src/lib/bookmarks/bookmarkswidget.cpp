/* ============================================================
* QupZilla - WebKit based browser
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
#include "bookmarkswidget.h"
#include "ui_bookmarkswidget.h"
#include "bookmarks.h"
#include "bookmarkitem.h"
#include "mainapplication.h"
#include "pluginproxy.h"
#include "speeddial.h"
#include "webview.h"
#include "browserwindow.h"

#include <QTimer>

#define HIDE_DELAY 270

BookmarksWidget::BookmarksWidget(WebView* view, BookmarkItem* bookmark, QWidget* parent)
    : LocationBarPopup(parent)
    , ui(new Ui::BookmarksWidget)
    , m_view(view)
    , m_bookmark(bookmark)
    , m_bookmarks(mApp->bookmarks())
    , m_speedDial(mApp->plugins()->speedDial())
    , m_edited(false)
{
    ui->setupUi(this);
    ui->bookmarksButton->setIcon(QIcon::fromTheme(QSL("bookmark-new")));

    init();
}

BookmarksWidget::~BookmarksWidget()
{
    delete ui;
}

void BookmarksWidget::toggleSpeedDial()
{
    const SpeedDial::Page page = m_speedDial->pageForUrl(m_view->url());

    if (page.url.isEmpty()) {
        QString title = m_view->title();
        m_speedDial->addPage(m_view->url(), title);
    }
    else {
        m_speedDial->removePage(page);
    }

    closePopup();
}

void BookmarksWidget::toggleBookmark()
{
    if (m_bookmark) {
        if (m_edited) {
            // Change folder
            m_bookmarks->removeBookmark(m_bookmark);
            m_bookmarks->addBookmark(ui->folderButton->selectedFolder(), m_bookmark);
        }
        else {
            // Remove
            m_bookmarks->removeBookmark(m_bookmark);
        }
    }
    else {
        // Save bookmark
        BookmarkItem* bookmark = new BookmarkItem(BookmarkItem::Url);
        bookmark->setTitle(m_view->title());
        bookmark->setUrl(m_view->url());
        m_bookmarks->addBookmark(ui->folderButton->selectedFolder(), bookmark);
    }

    closePopup();
}

void BookmarksWidget::bookmarkEdited()
{
    if (m_edited) {
        return;
    }

    m_edited = true;
    ui->bookmarksButton->setText(tr("Update Bookmark"));
    ui->bookmarksButton->setFlat(true);
}

void BookmarksWidget::init()
{
    // The locationbar's direction is direction of its text,
    // it dynamically changes and so, it's not good choice for this widget.
    setLayoutDirection(QApplication::layoutDirection());

    // Init SpeedDial button
    const SpeedDial::Page page = m_speedDial->pageForUrl(m_view->url());
    if (page.url.isEmpty()) {
        ui->speeddialButton->setFlat(true);
        ui->speeddialButton->setText(tr("Add to Speed Dial"));
    }
    else {
        ui->speeddialButton->setFlat(false);
        ui->speeddialButton->setText(tr("Remove from Speed Dial"));
    }

    // Init Bookmarks button
    if (m_bookmark) {
        ui->bookmarksButton->setText(tr("Remove from Bookmarks"));
        ui->bookmarksButton->setFlat(false);

        Q_ASSERT(m_bookmark->parent());
        ui->folderButton->setSelectedFolder(m_bookmark->parent());
        connect(ui->folderButton, SIGNAL(selectedFolderChanged(BookmarkItem*)), SLOT(bookmarkEdited()));
    }

    connect(ui->speeddialButton, SIGNAL(clicked()), this, SLOT(toggleSpeedDial()));
    connect(ui->bookmarksButton, SIGNAL(clicked()), this, SLOT(toggleBookmark()));

}

void BookmarksWidget::closePopup()
{
    // Prevent clicking again on buttons while popup is being closed
    disconnect(ui->speeddialButton, SIGNAL(clicked()), this, SLOT(toggleSpeedDial()));
    disconnect(ui->bookmarksButton, SIGNAL(clicked()), this, SLOT(toggleBookmark()));

    QTimer::singleShot(HIDE_DELAY, this, SLOT(close()));
}

