/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include "bookmarksmodel.h"
#include "mainapplication.h"

BookmarksWidget::BookmarksWidget(int bookmarkId, QWidget* parent) :
    QMenu(parent)
    , ui(new Ui::BookmarksWidget)
    , m_bookmarkId(bookmarkId)
    , m_bookmarksModel(0)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(ui->close, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->removeBookmark, SIGNAL(clicked()), this, SLOT(removeBookmark()));
    connect(ui->save, SIGNAL(clicked()), this, SLOT(saveBookmark()));

    m_bookmarksModel = mApp->bookmarksModel();
    loadBookmark();
}

void BookmarksWidget::loadBookmark()
{
    BookmarksModel::Bookmark bookmark = m_bookmarksModel->getBookmark(m_bookmarkId);
    ui->name->setText(bookmark.title);

    // Bookmark folders
    ui->folder->addItem(QIcon(":icons/other/unsortedbookmarks.png"), tr("Unsorted Bookmarks"), "unsorted");
    ui->folder->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In Menu"), "bookmarksMenu");
    ui->folder->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In ToolBar"), "bookmarksToolbar");
    QSqlQuery query;
    query.exec("SELECT name FROM folders");
    while (query.next()) {
        ui->folder->addItem(style()->standardIcon(QStyle::SP_DirIcon), query.value(0).toString(), query.value(0).toString());
    }

    ui->folder->setCurrentIndex(ui->folder->findData(bookmark.folder));
    ui->name->setCursorPosition(0);
}

void BookmarksWidget::removeBookmark()
{
    m_bookmarksModel->removeBookmark(m_bookmarkId);
    emit bookmarkDeleted();
    close();
}

void BookmarksWidget::saveBookmark()
{
    m_bookmarksModel->editBookmark(m_bookmarkId, ui->name->text(), QUrl(), ui->folder->itemData(ui->folder->currentIndex()).toString());
    close();
}

void BookmarksWidget::showAt(QWidget* _parent)
{
    QPoint p = _parent->mapToGlobal(QPoint(0, 0));
    move((p.x() + _parent->width()) - width(), p.y() + _parent->height());
    show();
}

BookmarksWidget::~BookmarksWidget()
{
    delete ui;
}
