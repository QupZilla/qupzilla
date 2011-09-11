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
#ifndef BOOKMARKSTOOLBAR_H
#define BOOKMARKSTOOLBAR_H

#include <QMenu>
#include <QToolButton>
#include <QHBoxLayout>

#include "bookmarksmodel.h"

class QupZilla;
class BookmarksModel;
class HistoryModel;
class ToolButton;
class BookmarksToolbar : public QWidget
{
    Q_OBJECT
public:
    explicit BookmarksToolbar(QupZilla* mainClass, QWidget* parent = 0);

signals:

public slots:
    void refreshBookmarks();
    void refreshMostVisited();
    void showMostVisited();

private slots:
    void loadClickedBookmark();
    void hidePanel();
    void addBookmark(const BookmarksModel::Bookmark &bookmark);
    void removeBookmark(const BookmarksModel::Bookmark &bookmark);
    void bookmarkEdited(const BookmarksModel::Bookmark &before, const BookmarksModel::Bookmark &after);
    void customContextMenuRequested(const QPoint &pos);

private:
    QupZilla* p_QupZilla;
    BookmarksModel* m_bookmarksModel;
    HistoryModel* m_historyModel;
    QMenu* m_menuMostVisited;
    ToolButton* m_mostVis;
    QHBoxLayout* m_layout;

};

#endif // BOOKMARKSTOOLBAR_H
