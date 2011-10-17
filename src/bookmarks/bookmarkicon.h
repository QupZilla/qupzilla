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
#ifndef BOOKMARKICON_H
#define BOOKMARKICON_H

#include <QUrl>

#include "clickablelabel.h"
#include "bookmarksmodel.h"
class QupZilla;
class BookmarksModel;
class BookmarkIcon : public ClickableLabel
{
    Q_OBJECT
public:
    explicit BookmarkIcon(QupZilla* mainClass, QWidget* parent = 0);
    void checkBookmark(const QUrl &url);

signals:

public slots:

private slots:
    void iconClicked();
    void bookmarkAdded(const BookmarksModel::Bookmark &bookmark);
    void bookmarkDeleted(const BookmarksModel::Bookmark &bookmark);

private:
    void setBookmarkSaved();
    void setBookmarkDisabled();

    QupZilla* p_QupZilla;
    BookmarksModel* m_bookmarksModel;

    QUrl m_lastUrl;

};

#endif // BOOKMARKICON_H
