/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "bookmarkstree.h"
#include "browsinglibrary.h"
#include "bookmarksmanager.h"

#include <QToolTip>
#include <QSqlQuery>
#include <QTimer>

#define HIDE_DELAY 270

BookmarksWidget::BookmarksWidget(WebView* view, QWidget* parent)
    : LocationBarPopup(parent)
    , ui(new Ui::BookmarksWidget)
    , m_url(view->url())
    , m_view(view)
    , m_bookmarksModel(mApp->bookmarksModel())
    , m_speedDial(mApp->plugins()->speedDial())
    , m_edited(false)
{
    ui->setupUi(this);
    m_bookmarksTree = new BookmarksTree(this);
    m_bookmarksTree->setViewType(BookmarksTree::ComboFolderView);
    m_bookmarksTree->header()->hide();
    m_bookmarksTree->setColumnCount(1);
    ui->folder->setModel(m_bookmarksTree->model());
    ui->folder->setView(m_bookmarksTree);

    // The locationbar's direction is direction of its text,
    // it dynamically changes and so, it's not good choice for this widget.
    setLayoutDirection(QApplication::layoutDirection());

    connect(ui->speeddialButton, SIGNAL(clicked(QPoint)), this, SLOT(toggleSpeedDial()));

    const SpeedDial::Page page = m_speedDial->pageForUrl(m_url);
    ui->speeddialButton->setText(page.url.isEmpty() ?
                                 tr("Add to Speed Dial") :
                                 tr("Remove from Speed Dial"));

    loadBookmark();

    connect(ui->folder, SIGNAL(activated(int)), this, SLOT(comboItemActive(int)));
    connect(m_bookmarksTree, SIGNAL(requestNewFolder(QWidget*, QString*, bool, QString, WebView*)),
            mApp->browsingLibrary()->bookmarksManager(), SLOT(addFolder(QWidget*, QString*, bool, QString, WebView*)));
}

void BookmarksWidget::loadBookmark()
{
    // Bookmark folders
    m_bookmarksTree->refreshTree();

    m_bookmarkId = m_bookmarksModel->bookmarkId(m_url);

    if (m_bookmarkId > 0) {
        BookmarksModel::Bookmark bookmark = m_bookmarksModel->getBookmark(m_bookmarkId);
        ui->name->setText(bookmark.title);

        int index = ui->folder->findData(bookmark.folder);
        // QComboBox::findData() returns index related to the item's parent
        if (index == -1) { // subfolder
            QModelIndex rootIndex = ui->folder->rootModelIndex();
            ui->folder->setRootModelIndex(ui->folder->model()->index(ui->folder->findText(_bookmarksToolbar), 0));
            // subfolder's name and its stored data are the same
            index = ui->folder->findText(bookmark.folder);
            ui->folder->setCurrentIndex(index);
            ui->folder->setRootModelIndex(rootIndex);
        }
        else {
            ui->folder->setCurrentIndex(index);
        }

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

void BookmarksWidget::toggleSpeedDial()
{
    const SpeedDial::Page page = m_speedDial->pageForUrl(m_url);

    if (page.url.isEmpty()) {
        m_speedDial->addPage(m_url, m_view->title());

    }
    else {
        m_speedDial->removePage(page);
    }
    QTimer::singleShot(HIDE_DELAY, this, SLOT(close()));
}

void BookmarksWidget::bookmarkEdited()
{
    if (m_edited) {
        return;
    }

    m_edited = true;
    ui->saveRemove->setText(tr("Save"));
}

void BookmarksWidget::comboItemActive(int index)
{
    m_bookmarksTree->activeItemChange(index, ui->folder, ui->name->text(), m_view);
}

void BookmarksWidget::on_saveRemove_clicked(bool)
{
    if (m_bookmarkId > 0) {
        if (m_edited) {
            m_bookmarksModel->editBookmark(m_bookmarkId, ui->name->text(), QUrl(), BookmarksModel::fromTranslatedFolder(ui->folder->currentText()));
        }
        else {
            m_bookmarksModel->removeBookmark(m_url);
            emit bookmarkDeleted();
        }
    }
    else {
        m_bookmarksModel->saveBookmark(m_url, ui->name->text(), m_view->icon(), BookmarksModel::fromTranslatedFolder(ui->folder->currentText()));
    }
    QTimer::singleShot(HIDE_DELAY, this, SLOT(close()));
}

BookmarksWidget::~BookmarksWidget()
{
    delete ui;
}

