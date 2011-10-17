/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include "bookmarksmodel.h"
#include "iconprovider.h"
#include "historymodel.h"
#include "toolbutton.h"

BookmarksToolbar::BookmarksToolbar(QupZilla* mainClass, QWidget* parent) :
    QWidget(parent)
    ,p_QupZilla(mainClass)
    ,m_bookmarksModel(mApp->bookmarksModel())
    ,m_historyModel(mApp->history())
{
    setObjectName("bookmarksbar");
    m_layout = new QHBoxLayout();
    m_layout->setContentsMargins(9, 3, 9, 3);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));

    connect(m_bookmarksModel, SIGNAL(bookmarkAdded(BookmarksModel::Bookmark)), this, SLOT(addBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkDeleted(BookmarksModel::Bookmark)), this, SLOT(removeBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkEdited(BookmarksModel::Bookmark,BookmarksModel::Bookmark)), this, SLOT(bookmarkEdited(BookmarksModel::Bookmark,BookmarksModel::Bookmark)));

//    QTimer::singleShot(0, this, SLOT(refreshBookmarks()));
    refreshBookmarks();
}

void BookmarksToolbar::customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos)

    QMenu menu;
    menu.addAction(tr("&Bookmark Current Page"), p_QupZilla, SLOT(bookmarkPage()));
    menu.addAction(tr("Bookmark &All Tabs"), p_QupZilla, SLOT(bookmarkAllTabs()));
    menu.addAction(IconProvider::fromTheme("user-bookmarks"), tr("&Organize Bookmarks"), p_QupZilla, SLOT(showBookmarksManager()));
    menu.addSeparator();
    menu.addAction(m_bookmarksModel->isShowingMostVisited() ? tr("Hide Most &Visited") : tr("Show Most &Visited"), this, SLOT(showMostVisited()));
    menu.addAction(tr("&Hide Toolbar"), this, SLOT(hidePanel()));

    //Prevent choosing first option with double rightclick
    QPoint position = QCursor::pos();
    QPoint p(position.x(), position.y()+1);
    menu.exec(p);
}

void BookmarksToolbar::hidePanel()
{
    p_QupZilla->showBookmarksToolbar();
}

void BookmarksToolbar::loadClickedBookmark()
{
    ToolButton* button = qobject_cast<ToolButton*>(sender());
    if (!button)
        return;

    p_QupZilla->loadAddress(button->data().toUrl());
}

void BookmarksToolbar::showMostVisited()
{
    m_bookmarksModel->setShowingMostVisited(!m_bookmarksModel->isShowingMostVisited());
    m_mostVis->setVisible(!m_mostVis->isVisible());
}

void BookmarksToolbar::addBookmark(const BookmarksModel::Bookmark &bookmark)
{
    if (bookmark.folder != "bookmarksToolbar")
        return;
    QString title = bookmark.title;
    if (title.length()>15) {
        title.truncate(13);
        title+="..";
    }

    ToolButton* button = new ToolButton(this);
    button->setText(title);
    button->setData(bookmark.url);
    button->setIcon(bookmark.icon);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setToolTip(bookmark.url.toEncoded());
    button->setAutoRaise(true);
    button->setWhatsThis(bookmark.title);

    connect(button, SIGNAL(clicked()), this, SLOT(loadClickedBookmark()));
    m_layout->insertWidget(m_layout->count() - 2, button);
}

void BookmarksToolbar::removeBookmark(const BookmarksModel::Bookmark &bookmark)
{
    for (int i = 0; i < m_layout->count(); i++) {
        ToolButton* button = qobject_cast<ToolButton*>(m_layout->itemAt(i)->widget());
        if (!button)
            continue;

        if (button->data().toUrl() == bookmark.url) {
            delete button;
            return;
        }
    }
}

void BookmarksToolbar::bookmarkEdited(const BookmarksModel::Bookmark &before, const BookmarksModel::Bookmark &after)
{
    if (before.folder == "bookmarksToolbar" && after.folder != "bookmarksToolbar") //Editing from toolbar folder to other folder -> Remove bookmark
        removeBookmark(before);
    else if (before.folder != "bookmarksToolbar" && after.folder == "bookmarksToolbar") //Editing from other folder to toolbar folder -> Add bookmark
        addBookmark(after);
    else { //Editing bookmark already in toolbar
        for (int i = 0; i < m_layout->count(); i++) {
            ToolButton* button = qobject_cast<ToolButton*>(m_layout->itemAt(i)->widget());
            if (!button)
                continue;

            if (button->data().toUrl() == before.url && button->whatsThis() == before.title) {
                QString title = after.title;
                if (title.length()>15) {
                    title.truncate(13);
                    title+="..";
                }

                button->setText(title);
                button->setData(after.url);
                button->setIcon(after.icon);
                button->setToolTip(after.url.toEncoded());
                button->setWhatsThis(after.title);
            }
        }
    }
}

void BookmarksToolbar::refreshBookmarks()
{
    QSqlQuery query;
    query.exec("SELECT title, url, icon FROM bookmarks WHERE folder='bookmarksToolbar'");
    while(query.next()) {
        QString title = query.value(0).toString();
        QUrl url = query.value(1).toUrl();
        QIcon icon = IconProvider::iconFromBase64(query.value(2).toByteArray());
        QString title_ = title;
        if (title.length()>15) {
            title.truncate(13);
            title+="..";
        }

        ToolButton* button = new ToolButton(this);
        button->setText(title);
        button->setData(url);
        button->setIcon(icon);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setToolTip(url.toEncoded());
        button->setWhatsThis(title_);
        button->setAutoRaise(true);

        connect(button, SIGNAL(clicked()), this, SLOT(loadClickedBookmark()));
        m_layout->addWidget(button);
    }

    m_mostVis = new ToolButton(this);
    m_mostVis->setPopupMode(QToolButton::InstantPopup);
    m_mostVis->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_mostVis->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    m_mostVis->setText(tr("Most visited"));
    m_mostVis->setToolTip(tr("Sites You visited the most"));

    m_menuMostVisited = new QMenu();
    m_mostVis->setMenu(m_menuMostVisited);
    connect(m_menuMostVisited, SIGNAL(aboutToShow()), this, SLOT(refreshMostVisited()));

    m_layout->addWidget(m_mostVis);
    m_layout->addStretch();

    m_mostVis->setVisible(m_bookmarksModel->isShowingMostVisited());
}

void BookmarksToolbar::refreshMostVisited()
{
    m_menuMostVisited->clear();

    QList<HistoryModel::HistoryEntry> mostList = m_historyModel->mostVisited(10);
    foreach (HistoryModel::HistoryEntry entry, mostList) {
        if (entry.title.length()>40) {
            entry.title.truncate(40);
            entry.title+="..";
        }
        m_menuMostVisited->addAction(_iconForUrl(entry.url), entry.title, p_QupZilla, SLOT(loadActionUrl()))->setData(entry.url);
    }
}
