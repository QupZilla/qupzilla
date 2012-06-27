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
#include "mainapplication.h"
#include "bookmarksmanager.h"
#include "ui_bookmarksmanager.h"
#include "qupzilla.h"
#include "tabbedwebview.h"
#include "bookmarkstoolbar.h"
#include "tabwidget.h"
#include "bookmarksmodel.h"
#include "iconprovider.h"
#include "browsinglibrary.h"
#include "globalfunctions.h"
#include "bookmarksimportdialog.h"
#include "iconchooser.h"
#include "webtab.h"
#include "websettings.h"

#include <QInputDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QShortcut>
#include <QMenu>
#include <QSqlQuery>

BookmarksManager::BookmarksManager(QupZilla* mainClass, QWidget* parent)
    : QWidget(parent)
    , m_isRefreshing(false)
    , ui(new Ui::BookmarksManager)
    , p_QupZilla(mainClass)
    , m_bookmarksModel(mApp->bookmarksModel())
{
    ui->setupUi(this);

    connect(ui->bookmarksTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(itemChanged(QTreeWidgetItem*)));
    connect(ui->addFolder, SIGNAL(clicked()), this, SLOT(addFolder()));
    connect(ui->bookmarksTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));
    connect(ui->bookmarksTree, SIGNAL(itemControlClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));
    connect(ui->bookmarksTree, SIGNAL(itemMiddleButtonClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));

    connect(m_bookmarksModel, SIGNAL(bookmarkAdded(BookmarksModel::Bookmark)), this, SLOT(addBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkDeleted(BookmarksModel::Bookmark)), this, SLOT(removeBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkEdited(BookmarksModel::Bookmark, BookmarksModel::Bookmark)), this, SLOT(bookmarkEdited(BookmarksModel::Bookmark, BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(subfolderAdded(QString)), this, SLOT(addSubfolder(QString)));
    connect(m_bookmarksModel, SIGNAL(folderAdded(QString)), this, SLOT(addFolder(QString)));
    connect(m_bookmarksModel, SIGNAL(folderDeleted(QString)), this, SLOT(removeFolder(QString)));
    connect(m_bookmarksModel, SIGNAL(folderRenamed(QString, QString)), this, SLOT(renameFolder(QString, QString)));

    connect(ui->optimizeDb, SIGNAL(clicked(QPoint)), this, SLOT(optimizeDb()));
    connect(ui->importBookmarks, SIGNAL(clicked(QPoint)), this, SLOT(importBookmarks()));

    QShortcut* deleteAction = new QShortcut(QKeySequence("Del"), ui->bookmarksTree);
    connect(deleteAction, SIGNAL(activated()), this, SLOT(deleteItem()));

    ui->bookmarksTree->setDefaultItemShowMode(TreeWidget::ItemsExpanded);
}

void BookmarksManager::importBookmarks()
{
    BookmarksImportDialog* b = new BookmarksImportDialog(this);
    b->show();
}

void BookmarksManager::search(const QString &string)
{
    ui->bookmarksTree->filterString(string);
}

QupZilla* BookmarksManager::getQupZilla()
{
    if (!p_QupZilla) {
        p_QupZilla = mApp->getWindow();
    }
    return p_QupZilla.data();
}

void BookmarksManager::setMainWindow(QupZilla* window)
{
    if (window) {
        p_QupZilla = window;
    }
}

void BookmarksManager::addFolder()
{
    QString text = QInputDialog::getText(this, tr("Add new folder"), tr("Choose name for new bookmark folder: "));
    if (text.isEmpty()) {
        return;
    }

    m_bookmarksModel->createFolder(text);
}

void BookmarksManager::addSubfolder()
{
    QString text = QInputDialog::getText(this, tr("Add new subfolder"), tr("Choose name for new subfolder in bookmarks toolbar: "));
    if (text.isEmpty()) {
        return;
    }

    m_bookmarksModel->createSubfolder(text);
}

void BookmarksManager::renameFolder()
{
    QTreeWidgetItem* item = ui->bookmarksTree->currentItem();
    if (!item) {
        return;
    }

    if (!item->text(1).isEmpty()) {
        return;
    }

    QString folder = item->text(0);

    if (folder == _bookmarksMenu || folder == _bookmarksToolbar) {
        return;
    }

    QString text = QInputDialog::getText(this, tr("Rename Folder"), tr("Choose name for folder: "), QLineEdit::Normal, folder);
    if (text.isEmpty()) {
        return;
    }

    m_bookmarksModel->renameFolder(folder, text);
}

void BookmarksManager::renameBookmark()
{
    QTreeWidgetItem* item = ui->bookmarksTree->currentItem();
    if (!item) {
        return;
    }

    ui->bookmarksTree->editItem(item, 0);
}

void BookmarksManager::changeIcon()
{
    QTreeWidgetItem* item = ui->bookmarksTree->currentItem();
    if (!item) {
        return;
    }

    int id = item->data(0, Qt::UserRole + 10).toInt();
    QIcon icon;

    IconChooser chooser(this);
    icon = chooser.exec();

    if (!icon.isNull()) {
        m_bookmarksModel->changeIcon(id, icon);
    }
}

void BookmarksManager::itemChanged(QTreeWidgetItem* item)
{
    if (!item || m_isRefreshing || item->text(1).isEmpty()) {
        return;
    }

    QString name = item->text(0);
    QUrl url = QUrl::fromEncoded(item->text(1).toUtf8());
    int id = item->data(0, Qt::UserRole + 10).toInt();

    ui->bookmarksTree->deleteItem(item);
    m_bookmarksModel->editBookmark(id, name, url, "");
}

void BookmarksManager::itemControlClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty()) {
        return;
    }
    getQupZilla()->tabWidget()->addView(QUrl(item->text(1)), item->text(0));
}

void BookmarksManager::loadInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        getQupZilla()->tabWidget()->addView(action->data().toUrl(), WebSettings::newTabPosition);
    }
}

void BookmarksManager::deleteItem()
{
    QTreeWidgetItem* item = ui->bookmarksTree->currentItem();
    if (!item) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (item->text(1).isEmpty()) { // Delete folder
        QString folder = item->text(0);
        m_bookmarksModel->removeFolder(folder);

        QApplication::restoreOverrideCursor();
        return;
    }

    int id = item->data(0, Qt::UserRole + 10).toInt();
    m_bookmarksModel->removeBookmark(id);
    QApplication::restoreOverrideCursor();
}

void BookmarksManager::addBookmark(WebView* view)
{
    insertBookmark(view->url(), view->title(), view->icon());
}

void BookmarksManager::moveBookmark()
{
    QTreeWidgetItem* item = ui->bookmarksTree->currentItem();
    if (!item) {
        return;
    }

    if (QAction* action = qobject_cast<QAction*>(sender())) {
        int id = item->data(0, Qt::UserRole + 10).toInt();

        m_bookmarksModel->editBookmark(id, item->text(0), QUrl(), action->data().toString());
    }
}

void BookmarksManager::contextMenuRequested(const QPoint &position)
{
    if (!ui->bookmarksTree->itemAt(position)) {
        return;
    }

    QUrl link = ui->bookmarksTree->itemAt(position)->data(0, Qt::UserRole + 11).toUrl();
    if (link.isEmpty()) {
        QString folderName = ui->bookmarksTree->itemAt(position)->text(0);
        QMenu menu;
        if (folderName == _bookmarksToolbar) {
            menu.addAction(tr("Add Subfolder"), this, SLOT(addSubfolder()));
            menu.addSeparator();
        }

        if (folderName != _bookmarksToolbar && folderName != _bookmarksMenu) {
            menu.addAction(tr("Rename folder"), this, SLOT(renameFolder()));
            menu.addAction(tr("Remove folder"), this, SLOT(deleteItem()));
        }

        if (menu.actions().count() == 0) {
            return;
        }

        //Prevent choosing first option with double rightclick
        QPoint pos = ui->bookmarksTree->viewport()->mapToGlobal(position);
        QPoint p(pos.x(), pos.y() + 1);
        menu.exec(p);
        return;
    }

    QMenu menu;
    menu.addAction(tr("Open link in current &tab"), getQupZilla(), SLOT(loadActionUrl()))->setData(link);
    menu.addAction(tr("Open link in &new tab"), this, SLOT(loadInNewTab()))->setData(link);
    menu.addSeparator();

    QMenu moveMenu;
    moveMenu.setTitle(tr("Move bookmark to &folder"));
    moveMenu.addAction(QIcon(":icons/other/unsortedbookmarks.png"), _bookmarksUnsorted, this, SLOT(moveBookmark()))->setData("unsorted");
    moveMenu.addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), _bookmarksMenu, this, SLOT(moveBookmark()))->setData("bookmarksMenu");
    moveMenu.addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), _bookmarksToolbar, this, SLOT(moveBookmark()))->setData("bookmarksToolbar");
    QSqlQuery query;
    query.exec("SELECT name FROM folders");
    while (query.next()) {
        moveMenu.addAction(style()->standardIcon(QStyle::SP_DirIcon), query.value(0).toString(), this, SLOT(moveBookmark()))->setData(query.value(0).toString());
    }
    menu.addMenu(&moveMenu);

    menu.addSeparator();
    menu.addAction(tr("Change icon"), this, SLOT(changeIcon()));
    menu.addAction(tr("Rename bookmark"), this, SLOT(renameBookmark()));
    menu.addAction(tr("Remove bookmark"), this, SLOT(deleteItem()));

    //Prevent choosing first option with double rightclick
    QPoint pos = ui->bookmarksTree->viewport()->mapToGlobal(position);
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);
}

void BookmarksManager::refreshTable()
{
    m_isRefreshing = true;
    ui->bookmarksTree->setUpdatesEnabled(false);
    ui->bookmarksTree->clear();

    QSqlQuery query;
    QTreeWidgetItem* newItem = new QTreeWidgetItem(ui->bookmarksTree);
    newItem->setText(0, _bookmarksMenu);
    newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    ui->bookmarksTree->addTopLevelItem(newItem);

    QTreeWidgetItem* bookmarksToolbar = new QTreeWidgetItem(ui->bookmarksTree);
    bookmarksToolbar->setText(0, _bookmarksToolbar);
    bookmarksToolbar->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    ui->bookmarksTree->addTopLevelItem(bookmarksToolbar);

    query.exec("SELECT name FROM folders WHERE subfolder!='yes'");
    while (query.next()) {
        newItem = new QTreeWidgetItem(ui->bookmarksTree);
        newItem->setText(0, query.value(0).toString());
        newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        ui->bookmarksTree->addTopLevelItem(newItem);
    }

    query.exec("SELECT title, url, id, folder, icon FROM bookmarks");
    while (query.next()) {
        QString title = query.value(0).toString();
        QUrl url = query.value(1).toUrl();
        int id = query.value(2).toInt();
        QString folder = query.value(3).toString();
        QIcon icon = qIconProvider->iconFromImage(QImage::fromData(query.value(4).toByteArray()));
        QTreeWidgetItem* item = new QTreeWidgetItem();
        if (folder == "bookmarksMenu") {
            folder = _bookmarksMenu;
        }
        if (folder == "bookmarksToolbar") {
            folder = _bookmarksToolbar;
        }

        if (folder != "unsorted") {
            QList<QTreeWidgetItem*> findParent = ui->bookmarksTree->findItems(folder, 0);
            if (findParent.count() != 1) {
                continue;
            }
            findParent.at(0)->addChild(item);
        }

        item->setText(0, title);
        item->setText(1, url.toEncoded());
        item->setToolTip(0, title);
        item->setToolTip(1, url.toEncoded());

        item->setData(0, Qt::UserRole + 10, id);
        item->setData(0, Qt::UserRole + 11, url);
        item->setIcon(0, icon);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->bookmarksTree->addTopLevelItem(item);
    }

    query.exec("SELECT name FROM folders WHERE subfolder='yes'");
    while (query.next()) {
        newItem = new QTreeWidgetItem(bookmarksToolbar);
        newItem->setText(0, query.value(0).toString());
        newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));

        QSqlQuery query2;
        query2.prepare("SELECT title, url, id, icon FROM bookmarks WHERE folder=?");
        query2.addBindValue(query.value(0).toString());
        query2.exec();
        while (query2.next()) {
            QString title = query2.value(0).toString();
            QUrl url = query2.value(1).toUrl();
            int id = query2.value(2).toInt();
            QIcon icon = qIconProvider->iconFromImage(QImage::fromData(query2.value(3).toByteArray()));
            QTreeWidgetItem* item = new QTreeWidgetItem(newItem);

            item->setText(0, title);
            item->setText(1, url.toEncoded());
            item->setToolTip(0, title);
            item->setToolTip(1, url.toEncoded());

            item->setData(0, Qt::UserRole + 10, id);
            item->setData(0, Qt::UserRole + 11, url);
            item->setIcon(0, icon);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
    }


    ui->bookmarksTree->expandAll();
    ui->bookmarksTree->setUpdatesEnabled(true);
    m_isRefreshing = false;
}

void BookmarksManager::addBookmark(const BookmarksModel::Bookmark &bookmark)
{
    m_isRefreshing = true;
    QString translatedFolder = BookmarksModel::toTranslatedFolder(bookmark.folder);
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, bookmark.title);
    item->setText(1, bookmark.url.toEncoded());
    item->setData(0, Qt::UserRole + 10, bookmark.id);
    item->setData(0, Qt::UserRole + 11, bookmark.url);
    item->setIcon(0, qIconProvider->iconFromImage(bookmark.image));
    item->setToolTip(0, bookmark.title);
    item->setToolTip(1, bookmark.url.toEncoded());
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    if (bookmark.inSubfolder) {
        QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(bookmark.folder, Qt::MatchExactly | Qt::MatchRecursive);
        if (list.count() != 0) {
            foreach(QTreeWidgetItem * it, list) {
                if (it->text(1).isEmpty()) {
                    it->addChild(item);
                    break;
                }
            }
        }
    }
    else if (bookmark.folder != "unsorted") {
        ui->bookmarksTree->appendToParentItem(translatedFolder, item);
    }
    else {
        ui->bookmarksTree->addTopLevelItem(item);
    }
    m_isRefreshing = false;
}

void BookmarksManager::removeBookmark(const BookmarksModel::Bookmark &bookmark)
{
    m_isRefreshing = true;

    if (bookmark.inSubfolder) {
        QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(bookmark.folder, Qt::MatchExactly | Qt::MatchRecursive);
        QTreeWidgetItem* subfolderItem = 0;
        if (list.count() != 0) {
            foreach(QTreeWidgetItem * it, list) {
                if (it->text(1).isEmpty()) {
                    subfolderItem = it;
                    break;
                }
            }
        }
        if (!subfolderItem) {
            return;
        }

        for (int i = 0; i < subfolderItem->childCount(); i++) {
            QTreeWidgetItem* item = subfolderItem->child(i);
            if (!item) {
                continue;
            }

            int id = item->data(0, Qt::UserRole + 10).toInt();

            if (item->text(0) == bookmark.title  && id == bookmark.id) {
                ui->bookmarksTree->deleteItem(item);
                return;
            }
        }
    }
    else if (bookmark.folder == "unsorted") {
        QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(bookmark.title, Qt::MatchExactly);
        if (list.count() == 0) {
            return;
        }
        QTreeWidgetItem* item = list.at(0);
        int id = item->data(0, Qt::UserRole + 10).toInt();

        if (id == bookmark.id) {
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

            int id = item->data(0, Qt::UserRole + 10).toInt();

            if (item->text(0) == bookmark.title  && id == bookmark.id) {
                ui->bookmarksTree->deleteItem(item);
                return;
            }
        }
    }
    m_isRefreshing = false;
}

void BookmarksManager::bookmarkEdited(const BookmarksModel::Bookmark &before, const BookmarksModel::Bookmark &after)
{
    removeBookmark(before);
    addBookmark(after);
}

void BookmarksManager::addFolder(const QString &name)
{
    m_isRefreshing = true;

    QTreeWidgetItem* item = new QTreeWidgetItem(ui->bookmarksTree);
    item->setText(0, name);
    item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));

    m_isRefreshing = false;
}

void BookmarksManager::addSubfolder(const QString &name)
{
    m_isRefreshing = true;

    QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(_bookmarksToolbar, Qt::MatchExactly);
    if (list.count() != 0) {
        QTreeWidgetItem* item = new QTreeWidgetItem(list.at(0));
        item->setText(0, name);
        item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    }

    m_isRefreshing = false;
}

void BookmarksManager::removeFolder(const QString &name)
{
    QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(name, Qt::MatchExactly | Qt::MatchRecursive);
    if (list.count() == 0) {
        return;
    }

    QTreeWidgetItem* folderItem = 0;
    if (list.count() != 0) {
        foreach(QTreeWidgetItem * it, list) {
            if (it->text(1).isEmpty()) {
                folderItem = it;
                break;
            }
        }
    }
    if (!folderItem) {
        return;
    }

    ui->bookmarksTree->deleteItem(folderItem);
}

void BookmarksManager::renameFolder(const QString &before, const QString &after)
{
    QList<QTreeWidgetItem*> list = ui->bookmarksTree->findItems(before, Qt::MatchExactly | Qt::MatchRecursive);
    if (list.count() == 0) {
        return;
    }

    QTreeWidgetItem* folderItem = 0;
    if (list.count() != 0) {
        foreach(QTreeWidgetItem * it, list) {
            if (it->text(0) == before && it->text(1).isEmpty()) {
                folderItem = it;
                break;
            }
        }
    }
    if (!folderItem) {
        return;
    }

    folderItem->setText(0, after);
}

void BookmarksManager::insertBookmark(const QUrl &url, const QString &title, const QIcon &icon)
{
    if (url.isEmpty() || title.isEmpty()) {
        return;
    }
    QDialog* dialog = new QDialog(getQupZilla());
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, dialog);
    QLabel* label = new QLabel(dialog);
    QLineEdit* edit = new QLineEdit(dialog);
    QComboBox* combo = new QComboBox(dialog);
    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
    layout->addWidget(label);
    layout->addWidget(edit);
    layout->addWidget(combo);
    if (m_bookmarksModel->isBookmarked(url)) {
        layout->addWidget(new QLabel(tr("<b>Warning: </b>You already have bookmarked this page!")));
    }
    layout->addWidget(box);

    combo->addItem(QIcon(":icons/other/unsortedbookmarks.png"), _bookmarksUnsorted);
    combo->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), _bookmarksMenu);
    combo->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), _bookmarksToolbar);
    QSqlQuery query;
    query.exec("SELECT name FROM folders");
    while (query.next()) {
        combo->addItem(style()->standardIcon(QStyle::SP_DirIcon), query.value(0).toString());
    }
    combo->setCurrentIndex(combo->findText(BookmarksModel::toTranslatedFolder(m_bookmarksModel->lastFolder())));

    label->setText(tr("Choose name and location of this bookmark."));
    edit->setText(title);
    edit->setCursorPosition(0);
    dialog->setWindowIcon(_iconForUrl(url));
    dialog->setWindowTitle(tr("Add New Bookmark"));

    QSize size = dialog->size();
    size.setWidth(350);
    dialog->resize(size);
    dialog->exec();
    if (dialog->result() == QDialog::Rejected) {
        return;
    }
    if (edit->text().isEmpty()) {
        return;
    }

    m_bookmarksModel->saveBookmark(url, edit->text(), icon, BookmarksModel::fromTranslatedFolder(combo->currentText()));
    delete dialog;
}

void BookmarksManager::insertAllTabs()
{
    QDialog* dialog = new QDialog(getQupZilla());
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, dialog);
    QLabel* label = new QLabel(dialog);
    QComboBox* combo = new QComboBox(dialog);
    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
    layout->addWidget(label);
    layout->addWidget(combo);
    layout->addWidget(box);

    combo->addItem(QIcon(":icons/other/unsortedbookmarks.png"), _bookmarksUnsorted);
    combo->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), _bookmarksMenu);
    combo->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), _bookmarksToolbar);
    QSqlQuery query;
    query.exec("SELECT name FROM folders");
    while (query.next()) {
        combo->addItem(style()->standardIcon(QStyle::SP_DirIcon), query.value(0).toString());
    }
    combo->setCurrentIndex(combo->findText(BookmarksModel::toTranslatedFolder(m_bookmarksModel->lastFolder())));

    label->setText(tr("Choose folder for bookmarks:"));
    dialog->setWindowTitle(tr("Bookmark All Tabs"));

    QSize size = dialog->size();
    size.setWidth(350);
    dialog->resize(size);
    dialog->exec();
    if (dialog->result() == QDialog::Rejected) {
        return;
    }

    foreach(WebTab * tab, getQupZilla()->tabWidget()->allTabs(false)) {
        if (tab->url().isEmpty()) {
            continue;
        }

        m_bookmarksModel->saveBookmark(tab->url(), tab->title(), tab->icon(), BookmarksModel::fromTranslatedFolder(combo->currentText()));
    }

    delete dialog;
}

void BookmarksManager::optimizeDb()
{
    BrowsingLibrary* b = qobject_cast<BrowsingLibrary*>(parentWidget()->parentWidget());
    if (!b) {
        return;
    }
    b->optimizeDatabase();
}


BookmarksManager::~BookmarksManager()
{
    delete ui;
}
