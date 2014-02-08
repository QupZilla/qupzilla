/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "bookmarks.h"
#include "iconprovider.h"
#include "browsinglibrary.h"
#include "qztools.h"
#include "bookmarksimportdialog.h"
#include "iconchooser.h"
#include "webtab.h"
#include "qzsettings.h"
#include "bookmarkstree.h"
#include "bookmarkitem.h"

#include <QInputDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QShortcut>
#include <QMenu>
#include <QSqlQuery>
#include <QLabel>

#include <QDebug>

BookmarksManager::BookmarksManager(QupZilla* mainClass, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::BookmarksManager)
    , p_QupZilla(mainClass)
    , m_bookmarks(mApp->bookmarks())
    , m_blockDescriptionChangedSignal(false)
{
    ui->setupUi(this);

    connect(ui->tree, SIGNAL(bookmarkActivated(BookmarkItem*)), this, SLOT(bookmarkActivated(BookmarkItem*)));
    connect(ui->tree, SIGNAL(bookmarkCtrlActivated(BookmarkItem*)), this, SLOT(bookmarkCtrlActivated(BookmarkItem*)));
    connect(ui->tree, SIGNAL(bookmarkShiftActivated(BookmarkItem*)), this, SLOT(bookmarkShiftActivated(BookmarkItem*)));
    connect(ui->tree, SIGNAL(bookmarksSelected(QList<BookmarkItem*>)), this, SLOT(bookmarksSelected(QList<BookmarkItem*>)));

    // Disable edit box
    updateEditBox(0);

    connect(ui->title, SIGNAL(textEdited(QString)), this, SLOT(bookmarkEdited()));
    connect(ui->address, SIGNAL(textEdited(QString)), this, SLOT(bookmarkEdited()));
    connect(ui->keyword, SIGNAL(textEdited(QString)), this, SLOT(bookmarkEdited()));
    connect(ui->description, SIGNAL(textChanged()), this, SLOT(descriptionEdited()));

#if 0
    ui->bookmarksTree->setViewType(BookmarksTree::ManagerView);

    ui->bookmarksTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->bookmarksTree->setDragDropReceiver(true, m_bookmarks);
    ui->bookmarksTree->setMimeType(QLatin1String("application/qupzilla.treewidgetitem.bookmarks"));

    connect(ui->bookmarksTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemChanged(QTreeWidgetItem*)));
    connect(ui->addFolder, SIGNAL(clicked()), this, SLOT(addFolder()));
    connect(ui->bookmarksTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));
    connect(ui->bookmarksTree, SIGNAL(itemControlClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));
    connect(ui->bookmarksTree, SIGNAL(itemMiddleButtonClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));
    connect(ui->collapseAll, SIGNAL(clicked()), ui->bookmarksTree, SLOT(collapseAll()));
    connect(ui->expandAll, SIGNAL(clicked()), ui->bookmarksTree, SLOT(expandAll()));

    connect(m_bookmarks, SIGNAL(bookmarkAdded(Bookmarks::Bookmark)), this, SLOT(addBookmark(Bookmarks::Bookmark)));
    connect(m_bookmarks, SIGNAL(bookmarkDeleted(Bookmarks::Bookmark)), this, SLOT(removeBookmark(Bookmarks::Bookmark)));
    connect(m_bookmarks, SIGNAL(bookmarkEdited(Bookmarks::Bookmark,Bookmarks::Bookmark)), this, SLOT(bookmarkEdited(Bookmarks::Bookmark,Bookmarks::Bookmark)));
    connect(m_bookmarks, SIGNAL(subfolderAdded(QString)), this, SLOT(addSubfolder(QString)));
    connect(m_bookmarks, SIGNAL(folderAdded(QString)), this, SLOT(addFolder(QString)));
    connect(m_bookmarks, SIGNAL(folderDeleted(QString)), this, SLOT(removeFolder(QString)));
    connect(m_bookmarks, SIGNAL(folderRenamed(QString,QString)), this, SLOT(renameFolder(QString,QString)));
    connect(m_bookmarks, SIGNAL(folderParentChanged(QString,bool)), this, SLOT(changeFolderParent(QString,bool)));
    connect(m_bookmarks, SIGNAL(bookmarkParentChanged(QString,QByteArray,int,QUrl,QString,QString)), this, SLOT(changeBookmarkParent(QString,QByteArray,int,QUrl,QString,QString)));

    QMenu* menu = new QMenu;
    menu->addAction(tr("Import Bookmarks..."), this, SLOT(importBookmarks()));
    menu->addAction(tr("Export Bookmarks to HTML..."), this, SLOT(exportBookmarks()));

    ui->importExport->setMenu(menu);

    QShortcut* deleteAction = new QShortcut(QKeySequence("Del"), ui->bookmarksTree);
    connect(deleteAction, SIGNAL(activated()), this, SLOT(deleteItem()));

    ui->bookmarksTree->setDefaultItemShowMode(TreeWidget::ItemsExpanded);
    ui->bookmarksTree->sortByColumn(-1);
#endif
}

BookmarksManager::~BookmarksManager()
{
    delete ui;
}

void BookmarksManager::bookmarkActivated(BookmarkItem* item)
{
    // TODO: Open all children in tabs for folder?
    if (!item->isUrl()) {
        return;
    }

    getQupZilla()->loadAddress(item->url());
}

void BookmarksManager::bookmarkCtrlActivated(BookmarkItem* item)
{
    if (!item->isUrl()) {
        return;
    }

    getQupZilla()->tabWidget()->addView(item->url(), item->title(), qzSettings->newTabPosition);
}

void BookmarksManager::bookmarkShiftActivated(BookmarkItem* item)
{
    if (!item->isUrl()) {
        return;
    }

    mApp->makeNewWindow(Qz::BW_NewWindow, item->url());
}

void BookmarksManager::bookmarksSelected(const QList<BookmarkItem*> &items)
{
    if (items.size() != 1) {
        updateEditBox(0);
    }
    else {
        updateEditBox(items.first());
    }
}

void BookmarksManager::bookmarkEdited()
{
    Q_ASSERT(ui->tree->selectedBookmarks().count() == 1);

    BookmarkItem* item = ui->tree->selectedBookmarks().first();
    item->setTitle(ui->title->text());
    item->setUrl(QUrl::fromEncoded(ui->address->text().toUtf8()));
    item->setKeyword(ui->keyword->text());
    item->setDescription(ui->description->toPlainText());

    m_bookmarks->notifyBookmarkChanged(item);
}

void BookmarksManager::descriptionEdited()
{
    // There is no textEdited() signal in QPlainTextEdit
    // textChanged() is emitted also when text is changed programatically
    if (!m_blockDescriptionChangedSignal) {
        bookmarkEdited();
    }
}

void BookmarksManager::importBookmarks()
{
    BookmarksImportDialog* b = new BookmarksImportDialog(this);
    b->show();
}

void BookmarksManager::exportBookmarks()
{
    QString file = QzTools::getSaveFileName("BookmarksManager-Export", this, tr("Export to HTML..."), QDir::homePath() + "/bookmarks.html");

    if (!file.isEmpty()) {
        m_bookmarks->exportToHtml(file);
    }
}

void BookmarksManager::updateEditBox(BookmarkItem* item)
{
    setUpdatesEnabled(false);
    m_blockDescriptionChangedSignal = true;

    bool editable = item && !item->isSeparator() && m_bookmarks->canBeModified(item);
    bool showAddressAndKeyword = item && item->isUrl();
    bool clearBox = !item;

    if (clearBox) {
        ui->title->clear();
        ui->address->clear();
        ui->keyword->clear();
        ui->description->clear();

        ui->title->setReadOnly(true);
        ui->address->setReadOnly(true);
        ui->keyword->setReadOnly(true);
        ui->description->setReadOnly(true);
    }
    else {
        ui->title->setText(item->title());
        ui->address->setText(item->url().toEncoded());
        ui->keyword->setText(item->keyword());
        ui->description->setPlainText(item->description());
    }

    ui->title->setReadOnly(!editable);
    ui->address->setReadOnly(!editable);
    ui->keyword->setReadOnly(!editable);
    ui->description->setReadOnly(!editable);

    ui->labelAddress->setVisible(showAddressAndKeyword);
    ui->address->setVisible(showAddressAndKeyword);
    ui->labelKeyword->setVisible(showAddressAndKeyword);
    ui->keyword->setVisible(showAddressAndKeyword);

    // Without removing widgets from layout, there is unwanted extra spacing
    QFormLayout* l = static_cast<QFormLayout*>(ui->editBox->layout());

    if (showAddressAndKeyword) {
        // Show Address + Keyword
        l->insertRow(1, ui->labelAddress, ui->address);
        l->insertRow(2, ui->labelKeyword, ui->keyword);
    }
    else {
        // Hide Address + Keyword
        l->removeWidget(ui->labelAddress);
        l->removeWidget(ui->labelKeyword);
        l->removeWidget(ui->address);
        l->removeWidget(ui->keyword);
    }

    m_blockDescriptionChangedSignal = false;
    setUpdatesEnabled(true);
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

#if 0
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
    moveMenu.addAction(QIcon(":icons/theme/unsortedbookmarks.png"), _bookmarksUnsorted, this, SLOT(moveBookmark()))->setData("unsorted");
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
#endif

// OLD

void BookmarksManager::addBookmark(WebView* view)
{
    insertBookmark(view->url(), view->title(), view->icon());
}

void BookmarksManager::insertBookmark(const QUrl &url, const QString &title, const QIcon &icon, const QString &folder)
{
    if (url.isEmpty() || title.isEmpty()) {
        return;
    }
    QDialog* dialog = new QDialog(getQupZilla());
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, dialog);
    QLabel* label = new QLabel(dialog);
    QLineEdit* edit = new QLineEdit(dialog);
    QComboBox* combo = new QComboBox(dialog);
    BookmarksTree* bookmarksTree = new BookmarksTree(dialog);
    connect(bookmarksTree, SIGNAL(requestNewFolder(QWidget*,QString*,bool,QString,WebView*)),
            this, SLOT(addFolder(QWidget*,QString*,bool,QString,WebView*)));
    bookmarksTree->setViewType(BookmarksTree::ComboFolderView);
    bookmarksTree->header()->hide();
    bookmarksTree->setColumnCount(1);
    combo->setModel(bookmarksTree->model());
    combo->setView(bookmarksTree);
    bookmarksTree->refreshTree();

    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
    layout->addWidget(label);
    layout->addWidget(edit);
    layout->addWidget(combo);
    if (m_bookmarks->isBookmarked(url)) {
        layout->addWidget(new QLabel(tr("<b>Warning: </b>You already have bookmarked this page!")));
    }
    layout->addWidget(box);

    int index = combo->findText(Bookmarks::toTranslatedFolder(folder.isEmpty() ? m_bookmarks->lastFolder() : folder));
    // QComboBox::find() returns index related to the item's parent
    if (index == -1) { // subfolder
        QModelIndex rootIndex = combo->rootModelIndex();
        combo->setRootModelIndex(combo->model()->index(combo->findText(_bookmarksToolbar), 0));
        index = combo->findText(Bookmarks::toTranslatedFolder(folder.isEmpty() ? m_bookmarks->lastFolder() : folder));
        combo->setCurrentIndex(index);
        combo->setRootModelIndex(rootIndex);
    }
    else {
        combo->setCurrentIndex(index);
    }
    connect(combo, SIGNAL(currentIndexChanged(int)), bookmarksTree, SLOT(activeItemChange(int)));

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
        delete dialog;
        return;
    }
    if (edit->text().isEmpty()) {
        delete dialog;
        return;
    }

    m_bookmarks->saveBookmark(url, edit->text(), icon, Bookmarks::fromTranslatedFolder(combo->currentText()));
    delete dialog;
}

void BookmarksManager::search(const QString &string)
{
    Q_UNUSED(string)
    //ui->bookmarksTree->filterString(string);
}

void BookmarksManager::insertAllTabs()
{
    QDialog* dialog = new QDialog(getQupZilla());
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, dialog);
    QLabel* label = new QLabel(dialog);
    QComboBox* combo = new QComboBox(dialog);
    BookmarksTree* bookmarksTree = new BookmarksTree(dialog);
    connect(bookmarksTree, SIGNAL(requestNewFolder(QWidget*,QString*,bool,QString,WebView*)),
            this, SLOT(addFolder(QWidget*,QString*,bool,QString,WebView*)));
    bookmarksTree->setViewType(BookmarksTree::ComboFolderView);
    bookmarksTree->header()->hide();
    bookmarksTree->setColumnCount(1);
    combo->setModel(bookmarksTree->model());
    combo->setView(bookmarksTree);
    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
    layout->addWidget(label);
    layout->addWidget(combo);
    layout->addWidget(box);

    bookmarksTree->refreshTree();


    int index = combo->findText(Bookmarks::toTranslatedFolder(m_bookmarks->lastFolder()));
    // QComboBox::find() returns index related to the item's parent
    if (index == -1) { // subfolder
        QModelIndex rootIndex = combo->rootModelIndex();
        combo->setRootModelIndex(combo->model()->index(combo->findText(_bookmarksToolbar), 0));
        index = combo->findText(Bookmarks::toTranslatedFolder(m_bookmarks->lastFolder()));
        combo->setCurrentIndex(index);
        combo->setRootModelIndex(rootIndex);
    }
    else {
        combo->setCurrentIndex(index);
    }
    connect(combo, SIGNAL(currentIndexChanged(int)), bookmarksTree, SLOT(activeItemChange(int)));

    label->setText(tr("Choose folder for bookmarks:"));
    dialog->setWindowTitle(tr("Bookmark All Tabs"));

    QSize size = dialog->size();
    size.setWidth(350);
    dialog->resize(size);
    dialog->exec();
    if (dialog->result() == QDialog::Rejected) {
        return;
    }

    foreach (WebTab* tab, getQupZilla()->tabWidget()->allTabs(false)) {
        if (tab->url().isEmpty()) {
            continue;
        }

        m_bookmarks->saveBookmark(tab->url(), tab->title(), tab->icon(), Bookmarks::fromTranslatedFolder(combo->currentText()));
    }

    delete dialog;
}

