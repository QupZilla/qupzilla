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
    , m_selectedBookmark(0)
    , m_blockDescriptionChangedSignal(false)
    , m_adjustHeaderSizesOnShow(true)
{
    ui->setupUi(this);

    connect(ui->tree, SIGNAL(bookmarkActivated(BookmarkItem*)), this, SLOT(bookmarkActivated(BookmarkItem*)));
    connect(ui->tree, SIGNAL(bookmarkCtrlActivated(BookmarkItem*)), this, SLOT(bookmarkCtrlActivated(BookmarkItem*)));
    connect(ui->tree, SIGNAL(bookmarkShiftActivated(BookmarkItem*)), this, SLOT(bookmarkShiftActivated(BookmarkItem*)));
    connect(ui->tree, SIGNAL(bookmarksSelected(QList<BookmarkItem*>)), this, SLOT(bookmarksSelected(QList<BookmarkItem*>)));
    connect(ui->tree, SIGNAL(contextMenuRequested(QPoint)), this, SLOT(createContextMenu(QPoint)));

    // Box for editing bookmarks
    updateEditBox(0);
    connect(ui->title, SIGNAL(textEdited(QString)), this, SLOT(bookmarkEdited()));
    connect(ui->address, SIGNAL(textEdited(QString)), this, SLOT(bookmarkEdited()));
    connect(ui->keyword, SIGNAL(textEdited(QString)), this, SLOT(bookmarkEdited()));
    connect(ui->description, SIGNAL(textChanged()), this, SLOT(descriptionEdited()));
}

BookmarksManager::~BookmarksManager()
{
    delete ui;
}

void BookmarksManager::bookmarkActivated(BookmarkItem* item)
{
    openBookmark(item);
}

void BookmarksManager::bookmarkCtrlActivated(BookmarkItem* item)
{
    openBookmarkInNewTab(item);
}

void BookmarksManager::bookmarkShiftActivated(BookmarkItem* item)
{
    openBookmarkInNewWindow(item);
}

void BookmarksManager::bookmarksSelected(const QList<BookmarkItem*> &items)
{
    if (items.size() != 1) {
        m_selectedBookmark = 0;
        updateEditBox(0);
    }
    else {
        m_selectedBookmark = items.first();
        updateEditBox(m_selectedBookmark);
    }
}

void BookmarksManager::createContextMenu(const QPoint &pos)
{
    QMenu menu;
    QAction* actNewTab = menu.addAction(QIcon::fromTheme("tab-new", QIcon(":/icons/menu/tab-new.png")), tr("Open in new tab"));
    QAction* actNewWindow = menu.addAction(QIcon::fromTheme("window-new"), tr("Open in new window"));
    menu.addSeparator();
    menu.addAction(tr("New Bookmark"), this, SLOT(addBookmark()));
    menu.addAction(tr("New Folder"), this, SLOT(addFolder()));
    menu.addAction(tr("New Separator"), this, SLOT(addSeparator()));
    menu.addSeparator();
    QAction* actDelete = menu.addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    connect(actNewTab, SIGNAL(triggered()), this, SLOT(openBookmarkInNewTab()));
    connect(actNewWindow, SIGNAL(triggered()), this, SLOT(openBookmarkInNewWindow()));
    connect(actDelete, SIGNAL(triggered()), this, SLOT(deleteBookmarks()));

    bool canBeDeleted = false;
    QList<BookmarkItem*> items = ui->tree->selectedBookmarks();

    foreach (BookmarkItem* item, items) {
        if (m_bookmarks->canBeModified(item)) {
            canBeDeleted = true;
            break;
        }
    }

    if (!canBeDeleted) {
        actDelete->setDisabled(true);
    }

    if (!m_selectedBookmark || !m_selectedBookmark->isUrl()) {
        actNewTab->setDisabled(true);
        actNewWindow->setDisabled(true);
    }

    menu.exec(pos);
}

void BookmarksManager::openBookmark(BookmarkItem* item)
{
    item = item ? item : m_selectedBookmark;

    // TODO: Open all children in tabs for folder?
    if (!item || !item->isUrl()) {
        return;
    }

    getQupZilla()->loadAddress(item->url());
}

void BookmarksManager::openBookmarkInNewTab(BookmarkItem* item)
{
    item = item ? item : m_selectedBookmark;

    // TODO: Open all children in tabs for folder?
    if (!item || !item->isUrl()) {
        return;
    }

    getQupZilla()->tabWidget()->addView(item->url(), item->title(), qzSettings->newTabPosition);
}

void BookmarksManager::openBookmarkInNewWindow(BookmarkItem* item)
{
    item = item ? item : m_selectedBookmark;

    if (!item || !item->isUrl()) {
        return;
    }

    mApp->makeNewWindow(Qz::BW_NewWindow, item->url());
}

void BookmarksManager::addBookmark()
{
    BookmarkItem* item = new BookmarkItem(BookmarkItem::Url);
    item->setTitle(tr("New Bookmark"));
    item->setUrl(QUrl("http://"));

    addBookmark(item);
}

void BookmarksManager::addFolder()
{
    BookmarkItem* item = new BookmarkItem(BookmarkItem::Folder);
    item->setTitle(tr("New Folder"));

    addBookmark(item);
}

void BookmarksManager::addSeparator()
{
    BookmarkItem* item = new BookmarkItem(BookmarkItem::Separator);

    addBookmark(item);
}

void BookmarksManager::deleteBookmarks()
{
    QList<BookmarkItem*> items = ui->tree->selectedBookmarks();

    foreach (BookmarkItem* item, items) {
        if (m_bookmarks->canBeModified(item)) {
            m_bookmarks->removeBookmark(item);
        }
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

    bool editable = bookmarkEditable(item);
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

bool BookmarksManager::bookmarkEditable(BookmarkItem* item) const
{
    return item && (item->isFolder() || item->isUrl()) && m_bookmarks->canBeModified(item);
}

void BookmarksManager::addBookmark(BookmarkItem* item)
{
    BookmarkItem* parent = parentForNewBookmark();
    Q_ASSERT(parent);

    // TODO: Make sure parent is expanded
    m_bookmarks->addBookmark(parent, item);
}

BookmarkItem* BookmarksManager::parentForNewBookmark() const
{
    if (m_selectedBookmark && m_selectedBookmark->isFolder()) {
        return m_selectedBookmark;
    }

    if (!m_selectedBookmark || m_selectedBookmark->parent() == m_bookmarks->rootItem()) {
        return m_bookmarks->unsortedFolder();
    }

    return m_selectedBookmark->parent();
}

void BookmarksManager::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Delete:
        deleteBookmarks();
        break;
    }

    QWidget::keyPressEvent(event);
}

QupZilla* BookmarksManager::getQupZilla()
{
    if (!p_QupZilla) {
        p_QupZilla = mApp->getWindow();
    }
    return p_QupZilla.data();
}

void BookmarksManager::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (m_adjustHeaderSizesOnShow) {
        ui->tree->header()->resizeSection(0, ui->tree->header()->width() / 1.9);
        m_adjustHeaderSizesOnShow = false;
    }
}

void BookmarksManager::setMainWindow(QupZilla* window)
{
    if (window) {
        p_QupZilla = window;
    }
}

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

