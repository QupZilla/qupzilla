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
#include "tabwidget.h"
#include "qzsettings.h"
#include "qupzilla.h"

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

#define FOLDER_ICON QApplication::style()->standardIcon(QStyle::SP_DirIcon)

void BookmarksFoldersMenu::init()
{
#define ADD_MENU(name) \
    BookmarkItem* f_##name = mApp->bookmarks()->name(); \
    QMenu* m_##name = addMenu(FOLDER_ICON, f_##name->title()); \
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
            QMenu* m = menu->addMenu(FOLDER_ICON, child->title());
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

    if (sender()) {
        emit selectedFolderChanged(folder);
    }
}

void BookmarksFoldersButton::init()
{
    setIcon(FOLDER_ICON);
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
    dialog->setWindowIcon(_iconForUrl(url));
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

void BookmarksTools::openBookmark(QupZilla* window, BookmarkItem* item)
{
    Q_ASSERT(window);

    // TODO: Open all children in tabs for folder?
    if (!item || !item->isUrl()) {
        return;
    }

    window->loadAddress(item->url());
}

void BookmarksTools::openBookmarkInNewTab(QupZilla* window, BookmarkItem* item)
{
    Q_ASSERT(window);

    // TODO: Open all children in tabs for folder?
    if (!item || !item->isUrl()) {
        return;
    }

    window->tabWidget()->addView(item->url(), item->title(), qzSettings->newTabPosition);
}

void BookmarksTools::openBookmarkInNewWindow(BookmarkItem* item)
{
    mApp->makeNewWindow(Qz::BW_NewWindow, item->url());
}
