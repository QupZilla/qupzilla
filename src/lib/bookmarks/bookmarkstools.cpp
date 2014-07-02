/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#include "bookmarkstools.h"
#include "bookmarkitem.h"
#include "bookmarks.h"
#include "mainapplication.h"
#include "iconprovider.h"
#include "enhancedmenu.h"
#include "tabwidget.h"
#include "qzsettings.h"
#include "browserwindow.h"

#include <iostream>
#include <QSqlQuery>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QStyle>
#include <QDialog>

// BookmarksFoldersMenu
BookmarksFoldersMenu::BookmarksFoldersMenu(QWidget* parent)
    : QMenu(parent)
    , m_selectedFolder(0)
{
    init();
}

BookmarkItem* BookmarksFoldersMenu::selectedFolder() const
{
    return m_selectedFolder;
}

void BookmarksFoldersMenu::folderChoosed()
{
    if (QAction* act = qobject_cast<QAction*>(sender())) {
        BookmarkItem* folder = static_cast<BookmarkItem*>(act->data().value<void*>());
        emit folderSelected(folder);
    }
}

void BookmarksFoldersMenu::init()
{
#define ADD_MENU(name) \
    BookmarkItem* f_##name = mApp->bookmarks()->name(); \
    QMenu* m_##name = addMenu(f_##name->icon(), f_##name->title()); \
    createMenu(m_##name, f_##name);

    ADD_MENU(toolbarFolder)
    ADD_MENU(menuFolder)
    ADD_MENU(unsortedFolder)
#undef ADD_MENU
}

void BookmarksFoldersMenu::createMenu(QMenu* menu, BookmarkItem* parent)
{
    QAction* act = menu->addAction(tr("Choose %1").arg(parent->title()));
    act->setData(QVariant::fromValue<void*>(static_cast<void*>(parent)));
    connect(act, SIGNAL(triggered()), this, SLOT(folderChoosed()));

    menu->addSeparator();

    foreach (BookmarkItem* child, parent->children()) {
        if (child->isFolder()) {
            QMenu* m = menu->addMenu(child->icon(), child->title());
            createMenu(m, child);
        }
    }
}


// BookmarksFoldersButton
BookmarksFoldersButton::BookmarksFoldersButton(QWidget* parent, BookmarkItem* folder)
    : QPushButton(parent)
    , m_menu(new BookmarksFoldersMenu(this))
    , m_selectedFolder(folder ? folder : mApp->bookmarks()->lastUsedFolder())
{
    init();

    connect(m_menu, SIGNAL(folderSelected(BookmarkItem*)), this, SLOT(setSelectedFolder(BookmarkItem*)));
}

BookmarkItem* BookmarksFoldersButton::selectedFolder() const
{
    return m_selectedFolder;
}

void BookmarksFoldersButton::setSelectedFolder(BookmarkItem* folder)
{
    Q_ASSERT(folder);
    Q_ASSERT(folder->isFolder());

    m_selectedFolder = folder;
    setText(folder->title());
    setIcon(folder->icon());

    if (sender()) {
        emit selectedFolderChanged(folder);
    }
}

void BookmarksFoldersButton::init()
{
    setMenu(m_menu);
    setSelectedFolder(m_selectedFolder);
}


// BookmarksTools
bool BookmarksTools::addBookmarkDialog(QWidget* parent, const QUrl &url, const QString &title, BookmarkItem* folder)
{
    if (url.isEmpty() || title.isEmpty()) {
        return false;
    }

    QDialog* dialog = new QDialog(parent);
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, dialog);
    QLabel* label = new QLabel(dialog);
    QLineEdit* edit = new QLineEdit(dialog);
    BookmarksFoldersButton* folderButton = new BookmarksFoldersButton(dialog, folder);

    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    QObject::connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    QObject::connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));

    layout->addWidget(label);
    layout->addWidget(edit);
    layout->addWidget(folderButton);
    layout->addWidget(box);

    label->setText(Bookmarks::tr("Choose name and location of this bookmark."));
    edit->setText(title);
    edit->setCursorPosition(0);
    dialog->setWindowIcon(IconProvider::iconForUrl(url));
    dialog->setWindowTitle(Bookmarks::tr("Add New Bookmark"));

    QSize size = dialog->size();
    size.setWidth(350);
    dialog->resize(size);
    dialog->exec();

    if (dialog->result() == QDialog::Rejected || edit->text().isEmpty()) {
        delete dialog;
        return false;
    }

    BookmarkItem* bookmark = new BookmarkItem(BookmarkItem::Url);
    bookmark->setTitle(edit->text());
    bookmark->setUrl(url);
    mApp->bookmarks()->addBookmark(folderButton->selectedFolder(), bookmark);

    delete dialog;
    return true;
}

bool BookmarksTools::bookmarkAllTabsDialog(QWidget* parent, TabWidget* tabWidget, BookmarkItem* folder)
{
    Q_ASSERT(tabWidget);

    QDialog* dialog = new QDialog(parent);
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, dialog);
    QLabel* label = new QLabel(dialog);
    BookmarksFoldersButton* folderButton = new BookmarksFoldersButton(dialog, folder);

    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    QObject::connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    QObject::connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));

    layout->addWidget(label);
    layout->addWidget(folderButton);
    layout->addWidget(box);

    label->setText(Bookmarks::tr("Choose folder for bookmarks:"));
    dialog->setWindowTitle(Bookmarks::tr("Bookmark All Tabs"));

    QSize size = dialog->size();
    size.setWidth(350);
    dialog->resize(size);
    dialog->exec();

    if (dialog->result() == QDialog::Rejected) {
        return false;
    }

    foreach (WebTab* tab, tabWidget->allTabs(false)) {
        if (!tab->url().isEmpty()) {
            BookmarkItem* bookmark = new BookmarkItem(BookmarkItem::Url);
            bookmark->setTitle(tab->title());
            bookmark->setUrl(tab->url());
            mApp->bookmarks()->addBookmark(folderButton->selectedFolder(), bookmark);
        }
    }

    delete dialog;
    return true;
}

void BookmarksTools::openBookmark(BrowserWindow* window, BookmarkItem* item)
{
    Q_ASSERT(window);

    if (!item || !item->isUrl()) {
        return;
    }

    if (item->isFolder()) {
        openFolderInTabs(window, item);
    }
    else if (item->isUrl()) {
        item->updateVisitCount();
        window->loadAddress(item->url());
    }
}

void BookmarksTools::openBookmarkInNewTab(BrowserWindow* window, BookmarkItem* item)
{
    Q_ASSERT(window);

    if (!item) {
        return;
    }

    if (item->isFolder()) {
        openFolderInTabs(window, item);
    }
    else if (item->isUrl()) {
        item->updateVisitCount();
        window->tabWidget()->addView(item->url(), item->title(), qzSettings->newTabPosition);
    }
}

void BookmarksTools::openBookmarkInNewWindow(BookmarkItem* item)
{
    if (!item->isUrl()) {
        return;
    }

    item->updateVisitCount();
    mApp->createWindow(Qz::BW_NewWindow, item->url());
}

void BookmarksTools::openBookmarkInNewPrivateWindow(BookmarkItem* item)
{
    if (!item->isUrl()) {
        return;
    }

    item->updateVisitCount();
    mApp->startPrivateBrowsing(item->url());
}

void BookmarksTools::openFolderInTabs(BrowserWindow* window, BookmarkItem* folder)
{
    Q_ASSERT(window);
    Q_ASSERT(folder->isFolder());

    foreach (BookmarkItem* child, folder->children()) {
        if (child->isUrl()) {
            openBookmarkInNewTab(window, child);
        }
        else if (child->isFolder()) {
            openFolderInTabs(window, child);
        }
    }
}

void BookmarksTools::addActionToMenu(QObject* receiver, Menu* menu, BookmarkItem* item)
{
    Q_ASSERT(menu);
    Q_ASSERT(item);

    switch (item->type()) {
    case BookmarkItem::Url:
        addUrlToMenu(receiver, menu, item);
        break;
    case BookmarkItem::Folder:
        addFolderToMenu(receiver, menu, item);
        break;
    case BookmarkItem::Separator:
        addSeparatorToMenu(menu, item);
        break;
    default:
        break;
    }
}

void BookmarksTools::addFolderToMenu(QObject* receiver, Menu* menu, BookmarkItem* folder)
{
    Q_ASSERT(menu);
    Q_ASSERT(folder);
    Q_ASSERT(folder->isFolder());

    Settings settings;
    bool hideEmptyFoldersInMenu = settings.value("Bookmarks-Settings/hideEmptyFoldersInMenu", false).toBool();
    if (hideEmptyFoldersInMenu && isFolderEmpty(folder)) {
        return;
    }

    Menu* m = new Menu(menu);
    QString title = QFontMetrics(m->font()).elidedText(folder->title(), Qt::ElideRight, 250);
    m->setTitle(title);
    m->setIcon(folder->icon());
    QObject::connect(m, SIGNAL(menuMiddleClicked(Menu*)), receiver, SLOT(menuMiddleClicked(Menu*)));

    QAction* act = menu->addMenu(m);
    act->setData(QVariant::fromValue<void*>(static_cast<void*>(folder)));
    act->setIconVisibleInMenu(true);

    foreach (BookmarkItem* child, folder->children()) {
        addActionToMenu(receiver, m, child);
    }

    if (m->isEmpty()) {
        m->addAction(Bookmarks::tr("Empty"))->setDisabled(true);
    }
}

bool BookmarksTools::isFolderEmpty(const BookmarkItem* folder)
{
    foreach (BookmarkItem* child, folder->children()) {
        if (child->type() == BookmarkItem::Url) {
            return false;
        }
        if (child->type() == BookmarkItem::Folder) {
            if (!isFolderEmpty(child)) {
              return false;
            }
        }
    }
    return true;
}

void BookmarksTools::addUrlToMenu(QObject* receiver, Menu* menu, BookmarkItem* bookmark)
{
    Q_ASSERT(menu);
    Q_ASSERT(bookmark);
    Q_ASSERT(bookmark->isUrl());

    Action* act = new Action(menu);
    QString title = QFontMetrics(act->font()).elidedText(bookmark->title(), Qt::ElideRight, 250);
    act->setText(title);
    act->setIcon(bookmark->icon());

    act->setData(QVariant::fromValue<void*>(static_cast<void*>(bookmark)));
    act->setIconVisibleInMenu(true);

    QObject::connect(act, SIGNAL(triggered()), receiver, SLOT(bookmarkActivated()));
    QObject::connect(act, SIGNAL(ctrlTriggered()), receiver, SLOT(bookmarkCtrlActivated()));
    QObject::connect(act, SIGNAL(shiftTriggered()), receiver, SLOT(bookmarkShiftActivated()));

    menu->addAction(act);
}

void BookmarksTools::addSeparatorToMenu(Menu* menu, BookmarkItem* separator)
{
    Q_UNUSED(separator)

    Q_ASSERT(menu);
    Q_ASSERT(separator->isSeparator());

    menu->addSeparator();
}

bool BookmarksTools::migrateBookmarksIfNecessary(Bookmarks* bookmarks)
{
    QSqlQuery query;
    query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='folders'");

    if (!query.next()) {
        return false;
    }

    std::cout << "Bookmarks: Migrating your bookmarks from SQLite to JSON..." << std::endl;

    QHash<QString, BookmarkItem*> folders;
    folders.insert("bookmarksToolbar", bookmarks->toolbarFolder());
    folders.insert("bookmarksMenu", bookmarks->menuFolder());
    folders.insert("unsorted", bookmarks->unsortedFolder());

    query.exec("SELECT name, subfolder FROM folders");
    while (query.next()) {
        const QString title = query.value(0).toString();
        bool subfolder = query.value(1).toString() == QLatin1String("yes");

        BookmarkItem* parent = subfolder ? bookmarks->toolbarFolder() : bookmarks->unsortedFolder();
        BookmarkItem* folder = new BookmarkItem(BookmarkItem::Folder, parent);
        folder->setTitle(title);
        folders.insert(folder->title(), folder);
    }

    query.exec("SELECT title, folder, url FROM bookmarks ORDER BY position ASC");
    while (query.next()) {
        const QString title = query.value(0).toString();
        const QString folder = query.value(1).toString();
        const QUrl url = query.value(2).toUrl();

        BookmarkItem* parent = folders.value(folder);
        if (!parent) {
            parent = bookmarks->unsortedFolder();
        }
        Q_ASSERT(parent);

        BookmarkItem* bookmark = new BookmarkItem(BookmarkItem::Url, parent);
        bookmark->setTitle(title);
        bookmark->setUrl(url);
    }

    query.exec("DROP TABLE folders");
    query.exec("DROP TABLE bookmarks");
    query.exec("VACUUM");

    std::cout << "Bookmarks: Bookmarks successfully migrated!" << std::endl;
    return true;
}
