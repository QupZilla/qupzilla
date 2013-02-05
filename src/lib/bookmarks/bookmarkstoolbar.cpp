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
#include "bookmarkstoolbar.h"
#include "qupzilla.h"
#include "mainapplication.h"
#include "bookmarksmodel.h"
#include "iconprovider.h"
#include "history.h"
#include "toolbutton.h"
#include "databasewriter.h"
#include "enhancedmenu.h"
#include "tabwidget.h"

#include <QDialog>
#include <QIcon>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMimeData>
#include <QDragEnterEvent>

BookmarksToolbar::BookmarksToolbar(QupZilla* mainClass, QWidget* parent)
    : QWidget(parent)
    , p_QupZilla(mainClass)
    , m_bookmarksModel(mApp->bookmarksModel())
    , m_historyModel(mApp->history())
    , m_toolButtonStyle(Qt::ToolButtonTextBesideIcon)
{
    setObjectName("bookmarksbar");
    m_layout = new QHBoxLayout();
    m_layout->setMargin(3);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    setAcceptDrops(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));

    connect(m_bookmarksModel, SIGNAL(bookmarkAdded(BookmarksModel::Bookmark)), this, SLOT(addBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkDeleted(BookmarksModel::Bookmark)), this, SLOT(removeBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkEdited(BookmarksModel::Bookmark, BookmarksModel::Bookmark)), this, SLOT(bookmarkEdited(BookmarksModel::Bookmark, BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(subfolderAdded(QString)), this, SLOT(subfolderAdded(QString)));
    connect(m_bookmarksModel, SIGNAL(folderDeleted(QString)), this, SLOT(folderDeleted(QString)));
    connect(m_bookmarksModel, SIGNAL(folderRenamed(QString, QString)), this, SLOT(folderRenamed(QString, QString)));
    connect(m_bookmarksModel, SIGNAL(folderParentChanged(QString, bool)), this, SLOT(changeFolderParent(QString, bool)));
    connect(m_bookmarksModel, SIGNAL(bookmarkParentChanged(QString, QByteArray, int, QUrl, QString, QString)), this, SLOT(changeBookmarkParent(QString, QByteArray, int, QUrl, QString, QString)));

    setMaximumWidth(p_QupZilla->width());

    refreshBookmarks();
    showOnlyIconsChanged();
}

void BookmarksToolbar::customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos)

    QMenu menu;
    menu.addAction(tr("&Bookmark Current Page"), p_QupZilla, SLOT(bookmarkPage()));
    menu.addAction(tr("Bookmark &All Tabs"), p_QupZilla, SLOT(bookmarkAllTabs()));
    menu.addAction(qIconProvider->fromTheme("user-bookmarks"), tr("&Organize Bookmarks"), p_QupZilla, SLOT(showBookmarksManager()));
    menu.addSeparator();
    QAction act(tr("Show Most &Visited"), this);
    act.setCheckable(true);
    act.setChecked(m_bookmarksModel->isShowingMostVisited());
    connect(&act, SIGNAL(triggered()), this, SLOT(showMostVisited()));
    menu.addAction(&act);
    QAction act2(tr("Show Only Icons"), this);
    act2.setCheckable(true);
    act2.setChecked(m_bookmarksModel->isShowingOnlyIconsInToolbar());
    connect(&act2, SIGNAL(triggered()), this, SLOT(toggleShowOnlyIcons()));
    menu.addAction(&act2);
    menu.addSeparator();
    menu.addAction(tr("&Hide Toolbar"), this, SLOT(hidePanel()));

    //Prevent choosing first option with double rightclick
    QPoint position = mapToGlobal(pos);
    QPoint p(position.x(), position.y() + 1);
    menu.exec(p);
}

void BookmarksToolbar::showBookmarkContextMenu(const QPoint &pos)
{
    Q_UNUSED(pos)

    ToolButton* button = qobject_cast<ToolButton*>(sender());
    if (!button) {
        return;
    }

    QVariant buttonPointer = QVariant::fromValue((void*) button);

    QMenu menu;
    menu.addAction(tr("Open bookmark"), this, SLOT(loadClickedBookmark()))->setData(buttonPointer);
    menu.addAction(tr("Open bookmark in new tab"), this, SLOT(loadClickedBookmarkInNewTab()))->setData(buttonPointer);
    menu.addSeparator();
    menu.addAction(qIconProvider->fromTheme("go-next"), tr("Move right"), this, SLOT(moveRight()))->setData(buttonPointer);
    menu.addAction(qIconProvider->fromTheme("go-previous"), tr("Move left"), this, SLOT(moveLeft()))->setData(buttonPointer);
    menu.addAction(tr("Edit bookmark"), this, SLOT(editBookmark()))->setData(buttonPointer);
    menu.addSeparator();
    menu.addAction(qIconProvider->fromTheme("list-remove"), tr("Remove bookmark"), this, SLOT(removeButton()))->setData(buttonPointer);

    //Prevent choosing first option with double rightclick
    QPoint position = button->mapToGlobal(pos);
    QPoint p(position.x(), position.y() + 1);
    menu.exec(p);
}

void BookmarksToolbar::moveRight()
{
    QAction* act = qobject_cast<QAction*> (sender());
    if (!act) {
        return;
    }

    ToolButton* button = static_cast<ToolButton*>(act->data().value<void*>());

    int index = m_layout->indexOf(button);
    if (index == m_layout->count() - 1) {
        return;
    }

    ToolButton* buttonRight = qobject_cast<ToolButton*> (m_layout->itemAt(index + 1)->widget());
    if (!buttonRight || buttonRight->menu()) {
        return;
    }

    Bookmark bookmark = button->data().value<Bookmark>();
    Bookmark bookmarkRight = buttonRight->data().value<Bookmark>();

    QSqlQuery query;
    query.prepare("UPDATE bookmarks SET position=? WHERE id=?");
    query.addBindValue(index + 1);
    query.addBindValue(bookmark.id);
    mApp->dbWriter()->executeQuery(query);

    query.prepare("UPDATE bookmarks SET position=? WHERE id=?");
    query.addBindValue(index);
    query.addBindValue(bookmarkRight.id);
    mApp->dbWriter()->executeQuery(query);

    QWidget* w = m_layout->takeAt(index)->widget();
    m_layout->insertWidget(index + 1, w);
}

void BookmarksToolbar::moveLeft()
{
    QAction* act = qobject_cast<QAction*> (sender());
    if (!act) {
        return;
    }

    ToolButton* button = static_cast<ToolButton*>(act->data().value<void*>());

    int index = m_layout->indexOf(button);
    if (index == 0) {
        return;
    }

    ToolButton* buttonLeft = qobject_cast<ToolButton*> (m_layout->itemAt(index - 1)->widget());
    if (!buttonLeft) {
        return;
    }

    Bookmark bookmark = button->data().value<Bookmark>();
    Bookmark bookmarkLeft = buttonLeft->data().value<Bookmark>();

    QSqlQuery query;
    query.prepare("UPDATE bookmarks SET position=? WHERE id=?");
    query.addBindValue(index - 1);
    query.addBindValue(bookmark.id);
    mApp->dbWriter()->executeQuery(query);

    query.prepare("UPDATE bookmarks SET position=? WHERE id=?");
    query.addBindValue(index);
    query.addBindValue(bookmarkLeft.id);
    mApp->dbWriter()->executeQuery(query);

    QWidget* w = m_layout->takeAt(index)->widget();
    m_layout->insertWidget(index - 1, w);
}

void BookmarksToolbar::editBookmark()
{
    QAction* act = qobject_cast<QAction*> (sender());
    if (!act) {
        return;
    }

    ToolButton* button = static_cast<ToolButton*>(act->data().value<void*>());
    if (!button) {
        return;
    }

    Bookmark b = button->data().value<Bookmark>();

    QDialog* dialog = new QDialog(this);
    QFormLayout* layout = new QFormLayout(dialog);
    QLabel* label = new QLabel(dialog);
    QLineEdit* editUrl = new QLineEdit(dialog);
    QLineEdit* editTitle = new QLineEdit(dialog);
    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));

    label->setText(tr("Edit bookmark: "));
    layout->addRow(label);
    layout->addRow(new QLabel(tr("Title: ")), editTitle);
    layout->addRow(new QLabel(tr("Url: ")), editUrl);
    layout->addRow(box);

    editUrl->setText(b.url.toString());
    editTitle->setText(b.title);
    editUrl->setCursorPosition(0);
    editTitle->setCursorPosition(0);

    dialog->setWindowTitle(tr("Edit Bookmark"));
    dialog->setMinimumSize(400, 100);
    dialog->exec();
    if (dialog->result() == QDialog::Rejected) {
        return;
    }

    QUrl url = QUrl::fromEncoded(editUrl->text().toUtf8());
    QString title = editTitle->text();

    if (url.isEmpty() || title.isEmpty()) {
        return;
    }

    m_bookmarksModel->editBookmark(b.id, title, url, b.folder);
}

void BookmarksToolbar::removeButton()
{
    QAction* act = qobject_cast<QAction*> (sender());
    if (!act) {
        return;
    }

    ToolButton* button = static_cast<ToolButton*>(act->data().value<void*>());
    if (!button) {
        return;
    }

    Bookmark bookmark = button->data().value<Bookmark>();
    m_bookmarksModel->removeBookmark(bookmark.id);
}

void BookmarksToolbar::hidePanel()
{
    p_QupZilla->showBookmarksToolbar();
}

void BookmarksToolbar::toggleShowOnlyIcons()
{
    m_bookmarksModel->setShowingOnlyIconsInToolbar(!m_bookmarksModel->isShowingOnlyIconsInToolbar());
    showOnlyIconsChanged();
}

void BookmarksToolbar::loadClickedBookmark()
{
    ToolButton* button = 0;

    QAction* act = qobject_cast<QAction*> (sender());
    if (act) {
        button = static_cast<ToolButton*>(act->data().value<void*>());
    }

    if (!button) {
        button = qobject_cast<ToolButton*>(sender());
    }

    if (!button) {
        return;
    }

    Bookmark bookmark = button->data().value<Bookmark>();

    p_QupZilla->loadAddress(bookmark.url);
}

void BookmarksToolbar::loadClickedBookmarkInNewTab()
{
    ToolButton* button = 0;

    QAction* act = qobject_cast<QAction*> (sender());
    if (act) {
        button = static_cast<ToolButton*>(act->data().value<void*>());
    }

    if (!button) {
        button = qobject_cast<ToolButton*>(sender());
    }

    if (!button) {
        return;
    }

    Bookmark bookmark = button->data().value<Bookmark>();

    p_QupZilla->tabWidget()->addView(bookmark.url, bookmark.title);
}

void BookmarksToolbar::loadFolderBookmarksInTabs()
{
    ToolButton* b = qobject_cast<ToolButton*>(sender());
    if (!b) {
        return;
    }

    QString folder = b->text();
    if (folder.isEmpty()) {
        return;
    }

    foreach(const Bookmark & b, m_bookmarksModel->folderBookmarks(folder)) {
        p_QupZilla->tabWidget()->addView(b.url, b.title, Qz::NT_NotSelectedTab);
    }
}

void BookmarksToolbar::showMostVisited()
{
    m_bookmarksModel->setShowingMostVisited(!m_bookmarksModel->isShowingMostVisited());
    m_mostVis->setVisible(!m_mostVis->isVisible());
}

int BookmarksToolbar::indexOfLastBookmark()
{
    for (int i = m_layout->count() - 1; i >= 0; i--) {
        ToolButton* button = qobject_cast<ToolButton*>(m_layout->itemAt(i)->widget());
        if (!button) {
            continue;
        }

        if (!button->menu()) {
            return i + 1;
        }
    }

    return 0;
}

void BookmarksToolbar::subfolderAdded(const QString &name)
{
    ToolButton* b = new ToolButton(this);
    b->setPopupMode(QToolButton::InstantPopup);
    b->setToolButtonStyle(m_toolButtonStyle);
    b->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    b->setText(name);
    connect(b, SIGNAL(middleMouseClicked()), this, SLOT(loadFolderBookmarksInTabs()));

    Menu* menu = new Menu(name);
    b->setMenu(menu);
    connect(menu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowFolderMenu()));

    m_layout->insertWidget(m_layout->count() - 2, b);
}

void BookmarksToolbar::folderDeleted(const QString &name)
{
    int index = indexOfLastBookmark();

    for (int i = index; i < m_layout->count(); i++) {
        ToolButton* button = qobject_cast<ToolButton*>(m_layout->itemAt(i)->widget());
        if (!button) {
            continue;
        }

        if (button->text() == name) {
            delete button;
            return;
        }
    }
}

void BookmarksToolbar::folderRenamed(const QString &before, const QString &after)
{
    int index = indexOfLastBookmark();

    for (int i = index; i < m_layout->count(); i++) {
        ToolButton* button = qobject_cast<ToolButton*>(m_layout->itemAt(i)->widget());
        if (!button) {
            continue;
        }

        if (button->text() == before) {
            button->setText(after);
            button->menu()->setTitle(after);
            return;
        }
    }
}

void BookmarksToolbar::changeBookmarkParent(const QString &name, const QByteArray &imageData, int id,
        const QUrl &url, const QString &oldParent, const QString &newParent)
{
    if (oldParent != _bookmarksToolbar && newParent != _bookmarksToolbar) {
        return;
    }

    bool itemIsAboutToRemove = (newParent != _bookmarksToolbar);

    Bookmark bookmark;
    bookmark.id =  id;
    bookmark.url = url;
    bookmark.title = name;
    bookmark.folder = QLatin1String("bookmarksToolbar");
    bookmark.image = QImage::fromData(imageData);
    bookmark.inSubfolder = false;

    if (itemIsAboutToRemove) {
        removeBookmark(bookmark);
    }
    else {
        addBookmark(bookmark);
    }
}

void BookmarksToolbar::changeFolderParent(const QString &name, bool isSubfolder)
{
    if (!isSubfolder) {
        folderDeleted(name);
    }
    else {
        subfolderAdded(name);
    }
}

void BookmarksToolbar::addBookmark(const BookmarksModel::Bookmark &bookmark)
{
    if (bookmark.folder != QLatin1String("bookmarksToolbar")) {
        return;
    }
    QString title = bookmark.title;
    if (title.length() > 15) {
        title.truncate(13);
        title += "..";
    }

    QVariant v;
    v.setValue<Bookmark>(bookmark);

    ToolButton* button = new ToolButton(this);
    button->setText(title);
    button->setData(v);
    button->setIcon(qIconProvider->iconFromImage(bookmark.image));
    button->setToolButtonStyle(m_toolButtonStyle);
    button->setToolTip(bookmark.url.toEncoded());
    button->setAutoRaise(true);
    button->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(button, SIGNAL(clicked()), this, SLOT(loadClickedBookmark()));
    connect(button, SIGNAL(middleMouseClicked()), this, SLOT(loadClickedBookmarkInNewTab()));
    connect(button, SIGNAL(controlClicked()), this, SLOT(loadClickedBookmarkInNewTab()));
    connect(button, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showBookmarkContextMenu(QPoint)));

    int indexForBookmark = indexOfLastBookmark();
    m_layout->insertWidget(indexForBookmark, button);

    QSqlQuery query;
    query.prepare("UPDATE bookmarks SET position=? WHERE id=?");
    query.addBindValue(indexForBookmark);
    query.addBindValue(bookmark.id);
    mApp->dbWriter()->executeQuery(query);
}

void BookmarksToolbar::removeBookmark(const BookmarksModel::Bookmark &bookmark)
{
    for (int i = 0; i < m_layout->count(); i++) {
        ToolButton* button = qobject_cast<ToolButton*>(m_layout->itemAt(i)->widget());
        if (!button) {
            continue;
        }

        Bookmark book = button->data().value<Bookmark>();

        if (book == bookmark) {
            delete button;
            return;
        }
    }
}

void BookmarksToolbar::bookmarkEdited(const BookmarksModel::Bookmark &before, const BookmarksModel::Bookmark &after)
{
    if (before.folder == QLatin1String("bookmarksToolbar") && after.folder != QLatin1String("bookmarksToolbar")) {
        // Editing from toolbar folder to other folder -> Remove bookmark
        removeBookmark(before);
    }
    else if (before.folder != QLatin1String("bookmarksToolbar") && after.folder == QLatin1String("bookmarksToolbar")) {
        // Editing from other folder to toolbar folder -> Add bookmark
        addBookmark(after);
    }
    else {
        // Editing bookmark already in toolbar
        for (int i = 0; i < m_layout->count(); i++) {
            ToolButton* button = qobject_cast<ToolButton*>(m_layout->itemAt(i)->widget());
            if (!button) {
                continue;
            }

            Bookmark book = button->data().value<Bookmark>();

            if (book == before) {
                QString title = after.title;
                if (title.length() > 15) {
                    title.truncate(13);
                    title += "..";
                }

                QVariant v;
                v.setValue<Bookmark>(after);

                button->setText(title);
                button->setData(v);
                button->setIcon(qIconProvider->iconFromImage(after.image));
                button->setToolTip(after.url.toEncoded());
                return;
            }
        }
    }
}

void BookmarksToolbar::refreshBookmarks()
{
    QSqlQuery query;
    query.exec("SELECT id, title, url, icon FROM bookmarks WHERE folder='bookmarksToolbar' ORDER BY position");
    while (query.next()) {
        Bookmark bookmark;
        bookmark.id = query.value(0).toInt();
        bookmark.title = query.value(1).toString();
        bookmark.url = query.value(2).toUrl();
        bookmark.image = QImage::fromData(query.value(3).toByteArray());
        bookmark.folder = "bookmarksToolbar";
        QString title = bookmark.title;
        if (title.length() > 15) {
            title.truncate(13);
            title += "..";
        }

        QVariant v;
        v.setValue<Bookmark>(bookmark);

        ToolButton* button = new ToolButton(this);
        button->setText(title);
        button->setData(v);
        button->setIcon(qIconProvider->iconFromImage(bookmark.image));
        button->setToolButtonStyle(m_toolButtonStyle);
        button->setToolTip(bookmark.url.toEncoded());
        button->setAutoRaise(true);
        button->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(button, SIGNAL(clicked()), this, SLOT(loadClickedBookmark()));
        connect(button, SIGNAL(middleMouseClicked()), this, SLOT(loadClickedBookmarkInNewTab()));
        connect(button, SIGNAL(controlClicked()), this, SLOT(loadClickedBookmarkInNewTab()));
        connect(button, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showBookmarkContextMenu(QPoint)));
        m_layout->addWidget(button);
    }

    query.exec("SELECT name FROM folders WHERE subfolder='yes'");
    while (query.next()) {
        ToolButton* b = new ToolButton(this);
        b->setPopupMode(QToolButton::InstantPopup);
        b->setToolButtonStyle(m_toolButtonStyle);
        b->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
        b->setText(query.value(0).toString());
        connect(b, SIGNAL(middleMouseClicked()), this, SLOT(loadFolderBookmarksInTabs()));

        Menu* menu = new Menu(query.value(0).toString());
        b->setMenu(menu);
        connect(menu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowFolderMenu()));

        m_layout->addWidget(b);
    }

    m_mostVis = new ToolButton(this);
    m_mostVis->setPopupMode(QToolButton::InstantPopup);
    m_mostVis->setToolButtonStyle(m_toolButtonStyle);
    m_mostVis->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    m_mostVis->setText(tr("Most visited"));
    m_mostVis->setToolTip(tr("Sites you visited the most"));

    m_menuMostVisited = new Menu();
    m_mostVis->setMenu(m_menuMostVisited);
    connect(m_menuMostVisited, SIGNAL(aboutToShow()), this, SLOT(refreshMostVisited()));

    m_layout->addWidget(m_mostVis);
    m_layout->addStretch();

    m_mostVis->setVisible(m_bookmarksModel->isShowingMostVisited());
}

void BookmarksToolbar::aboutToShowFolderMenu()
{
    QMenu* menu = qobject_cast<QMenu*> (sender());
    if (!menu) {
        return;
    }

    menu->clear();
    QString folder = menu->title();

    foreach(const Bookmark & b, m_bookmarksModel->folderBookmarks(folder)) {
        QString title = b.title;
        if (title.length() > 40) {
            title.truncate(40);
            title += "..";
        }

        Action* act = new Action(qIconProvider->iconFromImage(b.image), title);
        act->setData(b.url);
        connect(act, SIGNAL(triggered()), p_QupZilla, SLOT(loadActionUrl()));
        connect(act, SIGNAL(middleClicked()), p_QupZilla, SLOT(loadActionUrlInNewNotSelectedTab()));
        menu->addAction(act);
    }

    if (menu->isEmpty()) {
        menu->addAction(tr("Empty"))->setEnabled(false);
    }
}

void BookmarksToolbar::dropEvent(QDropEvent* e)
{
    const QMimeData* mime = e->mimeData();

    if (!mime->hasUrls() || !mime->hasText()) {
        QWidget::dropEvent(e);
        return;
    }

    QString title = mime->text();
    QUrl url = mime->urls().at(0);
    QIcon icon = qIconProvider->iconFromImage(qvariant_cast<QImage>(mime->imageData()));

    m_bookmarksModel->saveBookmark(url, title, icon, "bookmarksToolbar");
}

void BookmarksToolbar::dragEnterEvent(QDragEnterEvent* e)
{
    const QMimeData* mime = e->mimeData();

    if (mime->hasUrls() && mime->hasText()) {
        e->acceptProposedAction();
        return;
    }

    QWidget::dragEnterEvent(e);
}

void BookmarksToolbar::showOnlyIconsChanged()
{
    m_toolButtonStyle = m_bookmarksModel->isShowingOnlyIconsInToolbar() ? Qt::ToolButtonIconOnly : Qt::ToolButtonTextBesideIcon;

    for (int i = 0; i < m_layout->count(); ++i) {
        ToolButton* button = qobject_cast<ToolButton*>(m_layout->itemAt(i)->widget());
        if (!button) {
            continue;
        }

        button->setToolButtonStyle(m_toolButtonStyle);
    }
}

void BookmarksToolbar::refreshMostVisited()
{
    m_menuMostVisited->clear();

    QList<HistoryEntry> mostList = m_historyModel->mostVisited(10);
    foreach(const HistoryEntry & entry, mostList) {
        QString title = entry.title;
        if (title.length() > 40) {
            title.truncate(40);
            title += "..";
        }

        Action* act = new Action(_iconForUrl(entry.url), title);
        act->setData(entry.url);
        connect(act, SIGNAL(triggered()), p_QupZilla, SLOT(loadActionUrl()));
        connect(act, SIGNAL(middleClicked()), p_QupZilla, SLOT(loadActionUrlInNewNotSelectedTab()));
        m_menuMostVisited->addAction(act);
    }

    if (m_menuMostVisited->isEmpty()) {
        m_menuMostVisited->addAction(tr("Empty"))->setEnabled(false);
    }
}
