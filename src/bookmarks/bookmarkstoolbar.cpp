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
#include "bookmarkstoolbar.h"
#include "qupzilla.h"
#include "bookmarksmodel.h"
#include "iconprovider.h"
#include "historymodel.h"

BookmarksToolbar::BookmarksToolbar(QupZilla* mainClass, QWidget* parent) :
    QToolBar(parent)
    ,p_QupZilla(mainClass)
    ,m_bookmarksModel(mApp->bookmarksModel())
    ,m_historyModel(mApp->history())
{
    setObjectName("bookmarksToolbar");
    setWindowTitle(tr("Bookmarks"));
    setStyleSheet("QToolBar{background-image:url(:icons/transp.png); border:none;}");
    setMovable(false);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));

    connect(m_bookmarksModel, SIGNAL(bookmarkAdded(BookmarksModel::Bookmark)), this, SLOT(addBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkDeleted(BookmarksModel::Bookmark)), this, SLOT(removeBookmark(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkEdited(BookmarksModel::Bookmark,BookmarksModel::Bookmark)), this, SLOT(bookmarkEdited(BookmarksModel::Bookmark,BookmarksModel::Bookmark)));

    QTimer::singleShot(0, this, SLOT(refreshBookmarks()));
}

void BookmarksToolbar::setColor(QColor color)
{
    setStyleSheet("QToolButton {color: "+color.name()+";}");
}

void BookmarksToolbar::customContextMenuRequested(const QPoint &pos)
{
    if (actionAt(pos))
        return;

    QMenu menu;
    menu.addAction(tr("&Bookmark Current Page"), p_QupZilla, SLOT(bookmarkPage()));
    menu.addAction(tr("Bookmark &All Tabs"), p_QupZilla, SLOT(bookmarkAllTabs()));
    menu.addAction(QIcon::fromTheme("user-bookmarks"), tr("&Organize Bookmarks"), p_QupZilla, SLOT(showBookmarksManager()));
    menu.addSeparator();
    menu.addAction(
#ifdef Q_WS_X11
            style()->standardIcon(QStyle::SP_BrowserReload)
#else
            QIcon(":/icons/faenza/reload.png")
#endif
            ,tr("&Reload Toolbar"), this, SLOT(refreshBookmarks()));
    menu.addSeparator();
    menu.addAction(m_bookmarksModel->isShowingMostVisited() ? tr("Hide Most &Visited") : tr("Show Most &Visited"), this, SLOT(showMostVisited()));
    menu.addAction(tr("&Hide Toolbar"), this, SLOT(hidePanel()));

    //Prevent choosing first option with double rightclick
    QPoint position = QCursor::pos();
    QPoint p(position.x(), position.y()+1);
    menu.exec(p);
}

void BookmarksToolbar::showMostVisited()
{
    m_bookmarksModel->setShowingMostVisited(!m_bookmarksModel->isShowingMostVisited());
    refreshBookmarks();
}

void BookmarksToolbar::hidePanel()
{
    p_QupZilla->showBookmarksToolbar();
}

void BookmarksToolbar::addBookmark(const BookmarksModel::Bookmark &bookmark)
{
    if (bookmark.folder != "bookmarksToolbar")
        return;
    QAction* action = new QAction(this);
    QString title = bookmark.title;
    if (title.length()>15) {
        title.truncate(13);
        title+="..";
    }

    action->setText(title);
    action->setData(bookmark.url);
    action->setIcon(_iconForUrl(bookmark.url));
    QToolButton* button = new QToolButton(this);
    button->setDefaultAction(action);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setMaximumHeight(25);
    button->setToolTip(bookmark.url.toEncoded());
    button->setWhatsThis(bookmark.title);

    connect(action, SIGNAL(triggered()), p_QupZilla, SLOT(loadActionUrl()));
    insertWidget(actions().at(actions().count() - 1), button);
}

void BookmarksToolbar::removeBookmark(const BookmarksModel::Bookmark &bookmark)
{
    foreach (QAction* act, actions()) {
        QToolButton* button = qobject_cast<QToolButton*>(widgetForAction(act));
        if (!button)
            continue;

        QAction* action = button->actions().at(0);
        if (!action)
            continue;

        if (action->data().toUrl() == bookmark.url) {
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
        foreach (QAction* act, actions()) {
            QToolButton* button = qobject_cast<QToolButton*>(widgetForAction(act));
            if (!button)
                continue;

            QAction* action = button->actions().at(0);
            if (!action)
                continue;

            if (action->data().toUrl() == before.url && button->whatsThis() == before.title) {
                QString title = after.title;
                if (title.length()>15) {
                    title.truncate(13);
                    title+="..";
                }

                action->setText(title);
                action->setData(after.url);
                action->setIcon(_iconForUrl(after.url));
                button->setToolTip(after.url.toEncoded());
                button->setWhatsThis(after.title);
            }
        }
    }
}

void BookmarksToolbar::refreshBookmarks()
{
    clear();
    QSqlQuery query;
    query.exec("SELECT title, url FROM bookmarks WHERE folder='bookmarksToolbar'");
    while(query.next()) {
        QUrl url = query.value(1).toUrl();
        QString title = query.value(0).toString();
        QString title_ = title;
        QAction* action = new QAction(this);
        if (title.length()>15) {
            title.truncate(13);
            title+="..";
        }

        action->setText(title);
        action->setData(url);
        action->setIcon(_iconForUrl(url));
        QToolButton* button = new QToolButton(this);
        button->setDefaultAction(action);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setMaximumHeight(25);
        button->setToolTip(url.toEncoded());
        button->setWhatsThis(title_);
        connect(action, SIGNAL(triggered()), p_QupZilla, SLOT(loadActionUrl()));
        addWidget(button);
    }

    if (!m_bookmarksModel->isShowingMostVisited())
        return;

    m_mostVis = new QToolButton(this);
    m_mostVis->setPopupMode(QToolButton::InstantPopup);
    m_mostVis->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_mostVis->setMaximumHeight(25);
    m_mostVis->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    m_mostVis->setText(tr("Most visited"));
    m_mostVis->setToolTip(tr("Sites You visited the most"));

    m_menuMostVisited = new QMenu();
    m_mostVis->setMenu(m_menuMostVisited);
    connect(m_menuMostVisited, SIGNAL(aboutToShow()), this, SLOT(refreshMostVisited()));

    addWidget(m_mostVis);
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

//    QSqlQuery query;
//    query.exec("SELECT title, url FROM history ORDER BY count DESC LIMIT 10");
//    while(query.next()) {
//        QUrl url = query.value(1).toUrl();
//        QString title = query.value(0).toString();
//        if (title.length()>40) {
//            title.truncate(40);
//            title+="..";
//        }
//        m_menuMostVisited->addAction(_iconForUrl(url), title, p_QupZilla, SLOT(loadActionUrl()))->setData(url);
//    }
}
