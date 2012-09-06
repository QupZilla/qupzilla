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

BookmarksWidget::BookmarksWidget(QupZilla* mainClass, WebView* view, QWidget* parent)
    : QMenu(parent)
    , ui(new Ui::BookmarksWidget)
    , p_QupZilla(mainClass)
    , m_url(view->url())
    , m_view(view)
    , m_bookmarksModel(mApp->bookmarksModel())
    , m_speedDial(mApp->plugins()->speedDial())
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // The locationbar's direction is direction of its text,
    // it dynamically changes and so, it's not good choice for this widget.
    setLayoutDirection(QApplication::layoutDirection());

    connect(ui->removeBookmark, SIGNAL(clicked()), this, SLOT(removeBookmark()));
    connect(ui->save, SIGNAL(clicked()), this, SLOT(saveBookmark()));
    connect(ui->speeddialButton, SIGNAL(clicked()), this, SLOT(toggleSpeedDial()));

    const SpeedDial::Page &page = m_speedDial->pageForUrl(m_url);
    ui->speeddialButton->setText(page.url.isEmpty() ? tr("Add to Speed Dial") : tr("Remove from Speed Dial"));

#ifndef KDE
    // Use light color for QLabels even with Ubuntu Ambiance theme
    QPalette pal = palette();
    pal.setColor(QPalette::WindowText, QToolTip::palette().color(QPalette::ToolTipText));
    ui->label_2->setPalette(pal);
    ui->label_3->setPalette(pal);
#endif

    addBookmark();
}

void BookmarksWidget::loadBookmark()
{
    if (m_bookmarksModel->isBookmarked(m_url)) {
        m_bookmarkId = m_bookmarksModel->bookmarkId(m_url);
        BookmarksModel::Bookmark bookmark = m_bookmarksModel->getBookmark(m_bookmarkId);
        ui->name->setText(bookmark.title);

        // Bookmark folders
        ui->folder->addItem(QIcon(":icons/other/unsortedbookmarks.png"), _bookmarksUnsorted, "unsorted");
        ui->folder->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), _bookmarksMenu, "bookmarksMenu");
        ui->folder->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), _bookmarksToolbar, "bookmarksToolbar");
        QSqlQuery query;
        query.exec("SELECT name FROM folders");
        while (query.next()) {
            ui->folder->addItem(style()->standardIcon(QStyle::SP_DirIcon), query.value(0).toString(), query.value(0).toString());
        }

        ui->folder->setCurrentIndex(ui->folder->findData(bookmark.folder));
        ui->name->setCursorPosition(0);
    }
}

void BookmarksWidget::removeBookmark()
{
    m_bookmarksModel->removeBookmark(m_url);
    emit bookmarkDeleted();
    close();
}

void BookmarksWidget::saveBookmark()
{
    m_bookmarksModel->editBookmark(m_bookmarkId, ui->name->text(), QUrl(), ui->folder->itemData(ui->folder->currentIndex()).toString());
    close();
}

void BookmarksWidget::addBookmark()
{
    if (!m_bookmarksModel->isBookmarked(m_url)) {
        m_bookmarksModel->saveBookmark(m_view);
    }

    loadBookmark();
}

void BookmarksWidget::toggleSpeedDial()
{
    const SpeedDial::Page &page = m_speedDial->pageForUrl(m_url);

    if (page.url.isEmpty()) {
        m_speedDial->addPage(m_url, m_view->title());
        ui->speeddialButton->setText(tr("Remove from Speed Dial"));

    }
    else {
        m_speedDial->removePage(page);
        ui->speeddialButton->setText(tr("Add to Speed Dial"));

    }
}

void BookmarksWidget::showAt(QWidget* _parent)
{
    layout()->invalidate();
    layout()->activate();

    QPoint p = _parent->mapToGlobal(QPoint(0, 0));
    move((p.x() + _parent->width()) - width(), p.y() + _parent->height());

    show();
}

BookmarksWidget::~BookmarksWidget()
{
    delete ui;
}
