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
#include "bookmarkswidget.h"
#include "ui_bookmarkswidget.h"
#include "bookmarksmodel.h"
#include "mainapplication.h"
#include "pluginproxy.h"
#include "speeddial.h"
#include "webview.h"
#include "qupzilla.h"

#include <QToolTip>
#include <QSqlQuery>
#include <QTimer>

BookmarksWidget::BookmarksWidget(QupZilla* mainClass, WebView* view, QWidget* parent)
    : LocationBarPopup(parent)
    , ui(new Ui::BookmarksWidget)
    , p_QupZilla(mainClass)
    , m_url(view->url())
    , m_view(view)
    , m_bookmarksModel(mApp->bookmarksModel())
    , m_speedDial(mApp->plugins()->speedDial())
    , m_edited(false)
{
    ui->setupUi(this);

    // The locationbar's direction is direction of its text,
    // it dynamically changes and so, it's not good choice for this widget.
    setLayoutDirection(QApplication::layoutDirection());

    connect(ui->speeddialButton, SIGNAL(clicked(QPoint)), this, SLOT(toggleSpeedDial()));

    const SpeedDial::Page &page = m_speedDial->pageForUrl(m_url);
    ui->speeddialButton->setText(page.url.isEmpty() ?
                                 tr("Add to Speed Dial") :
                                 tr("Remove from Speed Dial"));

    loadBookmark();
}

void BookmarksWidget::loadBookmark()
{
    // Bookmark folders
    ui->folder->addItem(QIcon(":/icons/other/unsortedbookmarks.png"), _bookmarksUnsorted, "unsorted");
    ui->folder->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), _bookmarksMenu, "bookmarksMenu");
    ui->folder->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), _bookmarksToolbar, "bookmarksToolbar");
    QSqlQuery query;
    query.exec("SELECT name FROM folders");
    while (query.next()) {
        ui->folder->addItem(style()->standardIcon(QStyle::SP_DirIcon), query.value(0).toString(), query.value(0).toString());
    }

    m_bookmarkId = m_bookmarksModel->bookmarkId(m_url);

    if (m_bookmarkId > 0) {
        BookmarksModel::Bookmark bookmark = m_bookmarksModel->getBookmark(m_bookmarkId);
        ui->name->setText(bookmark.title);
        ui->folder->setCurrentIndex(ui->folder->findData(bookmark.folder));
        ui->saveRemove->setText(tr("Remove"));
        connect(ui->name, SIGNAL(textEdited(QString)), SLOT(bookmarkEdited()));
        connect(ui->folder, SIGNAL(currentIndexChanged(int)), SLOT(bookmarkEdited()));
    }
    else {
        ui->name->setText(m_view->title());
        ui->folder->setCurrentIndex(0);
    }

    ui->name->setCursorPosition(0);
}

namespace
{
const int hideDelay = 270;
}

void BookmarksWidget::toggleSpeedDial()
{
    const SpeedDial::Page &page = m_speedDial->pageForUrl(m_url);

    QString const dialText("<a href='toggle_dial'>%1</a>");
    if (page.url.isEmpty()) {
        m_speedDial->addPage(m_url, m_view->title());

    }
    else {
        m_speedDial->removePage(page);
    }
    QTimer::singleShot(hideDelay, this, SLOT(close()));
}

void BookmarksWidget::bookmarkEdited()
{
    if (m_edited) {
        return;
    }

    m_edited = true;
    ui->saveRemove->setText(tr("Save"));
}

void BookmarksWidget::on_saveRemove_clicked(bool)
{
    if (m_bookmarkId > 0) {
        if (m_edited) {
            m_bookmarksModel->editBookmark(m_bookmarkId, ui->name->text(), QUrl(), ui->folder->itemData(ui->folder->currentIndex()).toString());
        }
        else {
            m_bookmarksModel->removeBookmark(m_url);
            emit bookmarkDeleted();
        }
    }
    else {
        m_bookmarksModel->saveBookmark(m_url, ui->name->text(), m_view->icon(), ui->folder->currentText());
    }
    QTimer::singleShot(hideDelay, this, SLOT(close()));
}

BookmarksWidget::~BookmarksWidget()
{
    delete ui;
}

