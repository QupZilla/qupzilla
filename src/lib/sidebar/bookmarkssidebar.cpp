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
#include "bookmarkssidebar.h"
#include "mainapplication.h"
#include "ui_bookmarkssidebar.h"
#include "qupzilla.h"
#include "iconprovider.h"
#include "tabbedwebview.h"
#include "bookmarkstoolbar.h"
#include "tabwidget.h"
#include "bookmarksmodel.h"
#include "qzsettings.h"

#include <QMenu>
#include <QTimer>
#include <QClipboard>
#include <QSqlQuery>
#include <QKeyEvent>

BookmarksSideBar::BookmarksSideBar(QupZilla* mainClass, QWidget* parent)
    : QWidget(parent)
    , m_isRefreshing(false)
    , ui(new Ui::BookmarksSideBar)
    , p_QupZilla(mainClass)
    , m_bookmarksModel(mApp->bookmarksModel())
{
    ui->setupUi(this);
    ui->bookmarksTree->setViewType(BookmarksTree::SideBarView);

    ui->bookmarksTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->bookmarksTree->setDragDropReceiver(true, m_bookmarksModel);
    ui->bookmarksTree->setMimeType(QLatin1String("application/qupzilla.treewidgetitem.bookmarks"));

    ui->bookmarksTree->setDefaultItemShowMode(TreeWidget::ItemsExpanded);
    connect(ui->bookmarksTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));
    connect(ui->bookmarksTree, SIGNAL(itemControlClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));
    connect(ui->bookmarksTree, SIGNAL(itemMiddleButtonClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));
    connect(ui->bookmarksTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*)));
    connect(ui->search, SIGNAL(textChanged(QString)), ui->bookmarksTree, SLOT(filterString(QString)));

    connect(m_bookmarksModel, SIGNAL(bookmarkAdded(BookmarksModel::Bookmark)), this, SLOT(addBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkDeleted(BookmarksModel::Bookmark)), this, SLOT(removeBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkEdited(BookmarksModel::Bookmark,BookmarksModel::Bookmark)),
            this, SLOT(bookmarkEdited(BookmarksModel::Bookmark,BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(folderAdded(QString)), this, SLOT(addFolder(QString)));
    connect(m_bookmarksModel, SIGNAL(folderDeleted(QString)), this, SLOT(removeFolder(QString)));
    connect(m_bookmarksModel, SIGNAL(folderRenamed(QString,QString)), this, SLOT(renameFolder(QString,QString)));
    connect(m_bookmarksModel, SIGNAL(folderParentChanged(QString,bool)), this, SLOT(changeFolderParent(QString,bool)));
    connect(m_bookmarksModel, SIGNAL(bookmarkParentChanged(QString,QByteArray,int,QUrl,QString,QString)),
            this, SLOT(changeBookmarkParent(QString,QByteArray,int,QUrl,QString,QString)));

    QTimer::singleShot(0, this, SLOT(refreshTable()));
}

void BookmarksSideBar::itemControlClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty()) {
        return;
    }

    const QUrl url = QUrl::fromEncoded(item->text(1).toUtf8());
    p_QupZilla->tabWidget()->addView(url, item->text(0));
}

void BookmarksSideBar::itemDoubleClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty()) {
        return;
    }

    const QUrl url = QUrl::fromEncoded(item->text(1).toUtf8());
    p_QupZilla->loadAddress(url);
}

void BookmarksSideBar::loadInNewTab()
{
    QTreeWidgetItem* item = ui->bookmarksTree->currentItem();
    QAction* action = qobject_cast<QAction*>(sender());

    if (!item || !action) {
        return;
    }

    p_QupZilla->tabWidget()->addView(action->data().toUrl(), item->text(0), qzSettings->newTabPosition);
}

void BookmarksSideBar::copyAddress()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QApplication::clipboard()->setText(action->data().toUrl().toEncoded());
    }
}

void BookmarksSideBar::deleteItem()
{
    QTreeWidgetItem* item = ui->bookmarksTree->currentItem();
    if (!item) {
        return;
    }

    int id = item->data(0, Qt::UserRole + 10).toInt();
    m_bookmarksModel->removeBookmark(id);
}

void BookmarksSideBar::contextMenuRequested(const QPoint &position)
{
    if (!ui->bookmarksTree->itemAt(position)) {
        return;
    }
    QUrl link = QUrl::fromEncoded(ui->bookmarksTree->itemAt(position)->text(1).toUtf8());
    if (link.isEmpty()) {
        return;
    }

    QMenu menu;
    menu.addAction(tr("Open link in current &tab"), p_QupZilla, SLOT(loadActionUrl()))->setData(link);
    menu.addAction(tr("Open link in &new tab"), this, SLOT(loadInNewTab()))->setData(link);
    menu.addAction(tr("Copy address"), this, SLOT(copyAddress()))->setData(link);
    menu.addSeparator();
    menu.addAction(tr("&Delete"), this, SLOT(deleteItem()));

    //Prevent choosing first option with double rightclick
    QPoint pos = ui->bookmarksTree->viewport()->mapToGlobal(position);
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);
}

void BookmarksSideBar::addBookmark(const BookmarksModel::Bookmark &bookmark)
{
    QString translatedFolder = BookmarksModel::toTranslatedFolder(bookmark.folder);
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, bookmark.title);
    item->setText(1, bookmark.url.toEncoded());
    item->setData(0, Qt::UserRole + 10, bookmark.id);
    item->setIcon(0, qIconProvider->iconFromImage(bookmark.image));
    item->setToolTip(0, bookmark.url.toEncoded());
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

    if (bookmark.folder != QLatin1String("unsorted")) {
        ui->bookmarksTree->appendToParentItem(translatedFolder, item);
    }
    else {
        ui->bookmarksTree->addTopLevelItem(item);
    }

//    if (!ui->search->text().isEmpty())
    item->setHidden(!bookmark.title.contains(ui->search->text(), Qt::CaseInsensitive));
}

void BookmarksSideBar::removeBookmark(const BookmarksModel::Bookmark &bookmark)
{
    if (bookmark.folder == QLatin1String("unsorted")) {
        QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(bookmark.title, Qt::MatchExactly);
        if (list.count() == 0) {
            return;
        }
        QTreeWidgetItem* item = list.at(0);
        if (item && item->data(0, Qt::UserRole + 10).toInt() == bookmark.id) {
            ui->bookmarksTree->deleteItem(item);
        }
    }
    else {
        QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(BookmarksModel::toTranslatedFolder(bookmark.folder), Qt::MatchExactly);
        if (list.count() == 0) {
            return;
        }
        QTreeWidgetItem* parentItem = list.at(0);
        if (!parentItem) {
            return;
        }
        for (int i = 0; i < parentItem->childCount(); i++) {
            QTreeWidgetItem* item = parentItem->child(i);
            if (!item) {
                continue;
            }
            if (item->text(0) == bookmark.title  && item->data(0, Qt::UserRole + 10).toInt() == bookmark.id) {
                ui->bookmarksTree->deleteItem(item);
                return;
            }
        }
    }
}

void BookmarksSideBar::bookmarkEdited(const BookmarksModel::Bookmark &before, const BookmarksModel::Bookmark &after)
{
    removeBookmark(before);
    addBookmark(after);
}

void BookmarksSideBar::addFolder(const QString &name)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->bookmarksTree);
    item->setText(0, name);
    item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));

    if (name != _bookmarksToolbar && name != _bookmarksMenu) {
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    }
    else {
        item->setFlags((item->flags() & ~Qt::ItemIsDragEnabled) | Qt::ItemIsDropEnabled);
    }
}

void BookmarksSideBar::removeFolder(const QString &name)
{
    QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(name, Qt::MatchExactly);
    if (list.count() == 0) {
        return;
    }
    QTreeWidgetItem* item = list.at(0);
    if (item) {
        ui->bookmarksTree->deleteItem(item);
    }
}

void BookmarksSideBar::renameFolder(const QString &before, const QString &after)
{
    QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(before, Qt::MatchExactly);
    if (list.count() == 0) {
        return;
    }
    QTreeWidgetItem* item = list.at(0);
    if (!item) {
        return;
    }

    item->setText(0, after);
}

void BookmarksSideBar::changeBookmarkParent(const QString &name, const QByteArray &imageData, int id,
        const QUrl &url, const QString &oldParent, const QString &newParent)
{
    if (!newParent.isEmpty() && newParent != QLatin1String("unsorted")
            && newParent != _bookmarksUnsorted && !oldParent.isEmpty()
            && oldParent != QLatin1String("unsorted")
            && oldParent != _bookmarksUnsorted) { // old and new parent are not invisibleRootItem()
        QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(newParent, Qt::MatchExactly);
        if (list.isEmpty()) { // newParent is a subfolder
            list = ui->bookmarksTree->findItems(oldParent, Qt::MatchExactly);
            if (list.isEmpty()) { // oldParent is a subfolder
                return; // item was reparented from bookmarkToolbar(subfolder) to subfolder(bookmarkToolbar)
            }
        }
    }

    QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(name, Qt::MatchExactly | Qt::MatchRecursive);

    QTreeWidgetItem* item = 0;
    foreach (item, list) {
        if (id == item->data(0, Qt::UserRole + 10).toInt()) {
            break;
        }
    }

    if (!item || id != item->data(0, Qt::UserRole + 10).toInt()) { // list is empty or item with 'id' was not found
        // bookmark's oldParent was bookmarkToolbar(subfolder)
        Bookmark bookmark;
        bookmark.id =  id;
        bookmark.url = url;
        bookmark.title = name;
        bookmark.folder = BookmarksModel::fromTranslatedFolder(newParent.isEmpty() ? QLatin1String("unsorted") : newParent);
        bookmark.image = QImage::fromData(imageData);
        bookmark.inSubfolder = false;
        addBookmark(bookmark);
        return;
    }

    QTreeWidgetItem* parent = 0;
    if (newParent.isEmpty() || newParent == QLatin1String("unsorted") || newParent == _bookmarksUnsorted) {
        parent = ui->bookmarksTree->invisibleRootItem();
    }
    item->parent() ? item->parent()->removeChild(item) : ui->bookmarksTree->invisibleRootItem()->removeChild(item);

    if (!parent) {
        list = ui->bookmarksTree->findItems(newParent, Qt::MatchExactly);
        if (list.count() == 0) {
            return;
        }

        parent = list.at(0);
        if (!parent) {
            return;
        }
    }

    parent->addChild(item);
}

void BookmarksSideBar::changeFolderParent(const QString &name, bool isSubfolder)
{
    if (isSubfolder) { // folder moved to _bookmarksToolbar as a subfolder
        removeFolder(name);
    }
    else {
        addFolder(name);
        QVector<Bookmark> bookmarksList = m_bookmarksModel->folderBookmarks(name);
        foreach (const Bookmark &b, bookmarksList) {
            addBookmark(b);
        }
    }
}

void BookmarksSideBar::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QTreeWidgetItem* item = ui->bookmarksTree->currentItem();

        if (event->modifiers() & Qt::ControlModifier) {
            itemControlClicked(item);
        }
        else {
            itemDoubleClicked(item);
        }

        return;
    }

    QWidget::keyPressEvent(event);
}

void BookmarksSideBar::refreshTable()
{
    m_isRefreshing = true;
    ui->bookmarksTree->refreshTree();
    m_isRefreshing = false;

    ui->search->setFocus();
}

BookmarksSideBar::~BookmarksSideBar()
{
    delete ui;
}
