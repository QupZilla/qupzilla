/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "webview.h"
#include "bookmarkstoolbar.h"
#include "tabwidget.h"
#include "bookmarksmodel.h"

BookmarksSideBar::BookmarksSideBar(QupZilla* mainClass, QWidget* parent) :
    QWidget(parent)
    ,m_isRefreshing(false)
    ,ui(new Ui::BookmarksSideBar)
    ,p_QupZilla(mainClass)
    ,m_bookmarksModel(mApp->bookmarksModel())
{
    ui->setupUi(this);

    ui->bookmarksTree->setDefaultItemShowMode(TreeWidget::ItemsExpanded);
    connect(ui->bookmarksTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));
    connect(ui->bookmarksTree, SIGNAL(itemControlClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));
    connect(ui->bookmarksTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*)));
    connect(ui->search, SIGNAL(textChanged(QString)), ui->bookmarksTree, SLOT(filterString(QString)));

    connect(m_bookmarksModel, SIGNAL(bookmarkAdded(BookmarksModel::Bookmark)), this, SLOT(addBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkDeleted(BookmarksModel::Bookmark)), this, SLOT(removeBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(folderAdded(QString)), this, SLOT(addFolder(QString)));
    connect(m_bookmarksModel, SIGNAL(folderDeleted(QString)), this, SLOT(removeFolder(QString)));
    connect(m_bookmarksModel, SIGNAL(bookmarkEdited(BookmarksModel::Bookmark,BookmarksModel::Bookmark)), this, SLOT(bookmarkEdited(BookmarksModel::Bookmark,BookmarksModel::Bookmark)));

    QTimer::singleShot(0, this, SLOT(refreshTable()));
}

void BookmarksSideBar::itemControlClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty())
        return;
    p_QupZilla->tabWidget()->addView(QUrl(item->text(1)));
}

void BookmarksSideBar::itemDoubleClicked(QTreeWidgetItem *item)
{
    if (!item || item->text(1).isEmpty())
        return;
    p_QupZilla->loadAddress(QUrl(item->text(1)));
}

void BookmarksSideBar::loadInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender()))
        p_QupZilla->tabWidget()->addView(action->data().toUrl(), tr("New Tab"), TabWidget::NewNotSelectedTab);
}

void BookmarksSideBar::deleteItem()
{
    QTreeWidgetItem* item = ui->bookmarksTree->currentItem();
    if (!item)
        return;

    int id = item->whatsThis(0).toInt();
    m_bookmarksModel->removeBookmark(id);
}

void BookmarksSideBar::moveBookmark()
{
    QTreeWidgetItem* item = ui->bookmarksTree->currentItem();
    if (!item)
        return;
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        m_bookmarksModel->editBookmark(item->whatsThis(0).toInt(), item->text(0), QUrl(), action->data().toString());
    }
}

void BookmarksSideBar::contextMenuRequested(const QPoint &position)
{
    if (!ui->bookmarksTree->itemAt(position))
        return;
    QString link = ui->bookmarksTree->itemAt(position)->text(1);
    if (link.isEmpty())
        return;

    QMenu menu;
    menu.addAction(tr("Open link in actual &tab"), p_QupZilla, SLOT(loadActionUrl()))->setData(link);
    menu.addAction(tr("Open link in &new tab"), this, SLOT(loadInNewTab()))->setData(link);
    menu.addSeparator();

    QMenu moveMenu;
    moveMenu.setTitle(tr("Move bookmark to &folder"));
    moveMenu.addAction(QIcon(":icons/other/unsortedbookmarks.png"), tr("Unsorted Bookmarks"), this, SLOT(moveBookmark()))->setData("unsorted");
    moveMenu.addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In Menu"), this, SLOT(moveBookmark()))->setData("bookmarksMenu");
    moveMenu.addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In ToolBar"), this, SLOT(moveBookmark()))->setData("bookmarksToolbar");
    QSqlQuery query;
    query.exec("SELECT name FROM folders");
    while(query.next())
        moveMenu.addAction(style()->standardIcon(QStyle::SP_DirIcon), query.value(0).toString(), this, SLOT(moveBookmark()))->setData(query.value(0).toString());
    menu.addMenu(&moveMenu);
    menu.addAction(tr("&Delete"), this, SLOT(deleteItem()));
    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y()+1);
    menu.exec(p);
}

void BookmarksSideBar::addBookmark(const BookmarksModel::Bookmark &bookmark)
{
    QString translatedFolder = BookmarksModel::toTranslatedFolder(bookmark.folder);
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, bookmark.title);
    item->setText(1, bookmark.url.toEncoded());
    item->setWhatsThis(0, QString::number(bookmark.id));
    item->setIcon(0, _iconForUrl(bookmark.url));
    item->setToolTip(0, bookmark.url.toEncoded());

    if (bookmark.folder != "unsorted")
        ui->bookmarksTree->appendToParentItem(translatedFolder, item);
    else
        ui->bookmarksTree->addTopLevelItem(item);

//    if (!ui->search->text().isEmpty())
        item->setHidden(!bookmark.title.contains(ui->search->text(), Qt::CaseInsensitive));
}

void BookmarksSideBar::removeBookmark(const BookmarksModel::Bookmark &bookmark)
{
    if (bookmark.folder == "unsorted") {
        QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(bookmark.title, Qt::MatchExactly);
        if (list.count() == 0)
            return;
        QTreeWidgetItem* item = list.at(0);
        if (item && item->whatsThis(0) == QString::number(bookmark.id))
            ui->bookmarksTree->deleteItem(item);
    } else {
        QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(BookmarksModel::toTranslatedFolder(bookmark.folder), Qt::MatchExactly);
        if (list.count() == 0)
            return;
        QTreeWidgetItem* parentItem = list.at(0);
        if (!parentItem)
            return;
        for (int i = 0; i < parentItem->childCount(); i++) {
            QTreeWidgetItem* item = parentItem->child(i);
            if (!item)
                continue;
            if (item->text(0) == bookmark.title  && item->whatsThis(0) == QString::number(bookmark.id)) {
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
}

void BookmarksSideBar::removeFolder(const QString &name)
{
    QTreeWidgetItem* item = ui->bookmarksTree->findItems(name, Qt::MatchExactly).at(0);
    if (item)
        ui->bookmarksTree->deleteItem(item);
}

void BookmarksSideBar::refreshTable()
{
    m_isRefreshing = true;
    ui->bookmarksTree->setUpdatesEnabled(false);
    ui->bookmarksTree->clear();

    QSqlQuery query;
    QTreeWidgetItem* newItem = new QTreeWidgetItem(ui->bookmarksTree);
    newItem->setText(0, tr("Bookmarks In Menu"));
    newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    ui->bookmarksTree->addTopLevelItem(newItem);

    newItem = new QTreeWidgetItem(ui->bookmarksTree);
    newItem->setText(0, tr("Bookmarks In ToolBar"));
    newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    ui->bookmarksTree->addTopLevelItem(newItem);

    query.exec("SELECT name FROM folders");
    while(query.next()) {
        newItem = new QTreeWidgetItem(ui->bookmarksTree);
        newItem->setText(0, query.value(0).toString());
        newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        ui->bookmarksTree->addTopLevelItem(newItem);
    }

    query.exec("SELECT title, url, id, folder FROM bookmarks");
    while(query.next()) {
        QString title = query.value(0).toString();
        QUrl url = query.value(1).toUrl();
        int id = query.value(2).toInt();
        QString folder = query.value(3).toString();
        QTreeWidgetItem* item;
        if (folder == "bookmarksMenu")
            folder = tr("Bookmarks In Menu");
        if (folder == "bookmarksToolbar")
            folder = tr("Bookmarks In ToolBar");

        if (folder != "unsorted") {
            QList<QTreeWidgetItem*> findParent = ui->bookmarksTree->findItems(folder, 0);
            if (findParent.count() == 1) {
                item = new QTreeWidgetItem(findParent.at(0));
            }else{
                QTreeWidgetItem* newParent = new QTreeWidgetItem(ui->bookmarksTree);
                newParent->setText(0, folder);
                newParent->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
                ui->bookmarksTree->addTopLevelItem(newParent);
                item = new QTreeWidgetItem(newParent);
            }
        } else
            item = new QTreeWidgetItem(ui->bookmarksTree);

        item->setText(0, title);
        item->setText(1, url.toEncoded());
        item->setToolTip(0, url.toEncoded());
//        item->setToolTip(1, url.toEncoded());

        item->setWhatsThis(0, QString::number(id));
        item->setIcon(0, _iconForUrl(url));
        ui->bookmarksTree->addTopLevelItem(item);
    }
    ui->bookmarksTree->expandAll();

    ui->bookmarksTree->setUpdatesEnabled(true);
    m_isRefreshing = false;
}

BookmarksSideBar::~BookmarksSideBar()
{
    delete ui;
}
