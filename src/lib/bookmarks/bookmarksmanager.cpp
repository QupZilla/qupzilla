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
#include "bookmarksmanager.h"
#include "ui_bookmarksmanager.h"
#include "bookmarksmodel.h"
#include "bookmarkstools.h"
#include "bookmarkitem.h"
#include "bookmarks.h"
#include "mainapplication.h"
#include "browserwindow.h"
#include "iconprovider.h"
#include "qztools.h"

#include <QMenu>
#include <QTimer>

BookmarksManager::BookmarksManager(BrowserWindow* window, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::BookmarksManager)
    , m_window(window)
    , m_bookmarks(mApp->bookmarks())
    , m_selectedBookmark(0)
    , m_blockDescriptionChangedSignal(false)
    , m_adjustHeaderSizesOnShow(true)
{
    ui->setupUi(this);
    ui->tree->setViewType(BookmarksTreeView::BookmarksManagerViewType);

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

void BookmarksManager::setMainWindow(BrowserWindow* window)
{
    if (window) {
        m_window = window;
    }
}

void BookmarksManager::search(const QString &string)
{
    ui->tree->search(string);
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
        m_selectedBookmark = items.at(0);
        updateEditBox(m_selectedBookmark);
    }
}

void BookmarksManager::createContextMenu(const QPoint &pos)
{
    QMenu menu;
    QAction* actNewTab = menu.addAction(IconProvider::newTabIcon(), tr("Open in new tab"));
    QAction* actNewWindow = menu.addAction(IconProvider::newWindowIcon(), tr("Open in new window"));
    QAction* actNewPrivateWindow = menu.addAction(IconProvider::privateBrowsingIcon(), tr("Open in new private window"));

    menu.addSeparator();
    menu.addAction(tr("New Bookmark"), this, SLOT(addBookmark()));
    menu.addAction(tr("New Folder"), this, SLOT(addFolder()));
    menu.addAction(tr("New Separator"), this, SLOT(addSeparator()));
    menu.addSeparator();
    QAction* actDelete = menu.addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    connect(actNewTab, SIGNAL(triggered()), this, SLOT(openBookmarkInNewTab()));
    connect(actNewWindow, SIGNAL(triggered()), this, SLOT(openBookmarkInNewWindow()));
    connect(actNewPrivateWindow, SIGNAL(triggered()), this, SLOT(openBookmarkInNewPrivateWindow()));
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
        actNewPrivateWindow->setDisabled(true);
    }

    menu.exec(pos);
}

void BookmarksManager::openBookmark(BookmarkItem* item)
{
    item = item ? item : m_selectedBookmark;
    BookmarksTools::openBookmark(getQupZilla(), item);
}

void BookmarksManager::openBookmarkInNewTab(BookmarkItem* item)
{
    item = item ? item : m_selectedBookmark;
    BookmarksTools::openBookmarkInNewTab(getQupZilla(), item);
}

void BookmarksManager::openBookmarkInNewWindow(BookmarkItem* item)
{
    item = item ? item : m_selectedBookmark;
    BookmarksTools::openBookmarkInNewWindow(item);
}

void BookmarksManager::openBookmarkInNewPrivateWindow(BookmarkItem* item)
{
    item = item ? item : m_selectedBookmark;
    BookmarksTools::openBookmarkInNewPrivateWindow(item);
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

    BookmarkItem* item = ui->tree->selectedBookmarks().at(0);
    item->setTitle(ui->title->text());
    item->setUrl(QUrl::fromEncoded(ui->address->text().toUtf8()));
    item->setKeyword(ui->keyword->text());
    item->setDescription(ui->description->toPlainText());

    m_bookmarks->changeBookmark(item);
}

void BookmarksManager::descriptionEdited()
{
    // There is no textEdited() signal in QPlainTextEdit
    // textChanged() is emitted also when text is changed programatically
    if (!m_blockDescriptionChangedSignal) {
        bookmarkEdited();
    }
}

void BookmarksManager::enableUpdates()
{
    setUpdatesEnabled(true);
}

void BookmarksManager::updateEditBox(BookmarkItem* item)
{
    setUpdatesEnabled(false);
    m_blockDescriptionChangedSignal = true;

    bool editable = bookmarkEditable(item);
    bool showAddressAndKeyword = item && item->isUrl();

    if (!item) {
        ui->title->clear();
        ui->address->clear();
        ui->keyword->clear();
        ui->description->clear();
    }
    else {
        ui->title->setText(item->title());
        ui->address->setText(item->url().toEncoded());
        ui->keyword->setText(item->keyword());
        ui->description->setPlainText(item->description());

        ui->title->setCursorPosition(0);
        ui->address->setCursorPosition(0);
        ui->keyword->setCursorPosition(0);
        ui->description->moveCursor(QTextCursor::Start);
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

    // Prevent flickering
    QTimer::singleShot(10, this, SLOT(enableUpdates()));
}

bool BookmarksManager::bookmarkEditable(BookmarkItem* item) const
{
    return item && (item->isFolder() || item->isUrl()) && m_bookmarks->canBeModified(item);
}

void BookmarksManager::addBookmark(BookmarkItem* item)
{
    BookmarkItem* parent = parentForNewBookmark();
    Q_ASSERT(parent);

    m_bookmarks->insertBookmark(parent, 0, item);

    // Select newly added bookmark
    ui->tree->selectBookmark(item);
    ui->tree->ensureBookmarkVisible(item);

    // Start editing title
    if (!item->isSeparator()) {
        ui->title->setFocus();
        ui->title->selectAll();
    }
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

BrowserWindow* BookmarksManager::getQupZilla()
{
    if (!m_window) {
        m_window = mApp->getWindow();
    }
    return m_window.data();
}

void BookmarksManager::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (m_adjustHeaderSizesOnShow) {
        ui->tree->header()->resizeSection(0, ui->tree->header()->width() / 1.9);
        m_adjustHeaderSizesOnShow = false;
    }
}
