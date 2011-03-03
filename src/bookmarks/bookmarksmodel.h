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
#ifndef BOOKMARKSMODEL_H
#define BOOKMARKSMODEL_H
#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QUrl>
#include <QSettings>
#include <QSqlQuery>

class WebView;
class BookmarksModel
{
public:
    explicit BookmarksModel();
    void loadSettings();
    inline bool isShowingMostVisited() { return m_showMostVisited; }
    void setShowingMostVisited(bool state);

    bool isBookmarked(QUrl url);
    int bookmarkId(QUrl url);
    int bookmarkId(QUrl url, QString title, QString folder);
    QStringList getBookmark(int id);

    bool saveBookmark(QUrl url, QString title, QString folder = "unsorted");
    bool saveBookmark(WebView* view, QString folder = "unsorted");

    bool removeBookmark(int id);
    bool removeBookmark(QUrl url);
    bool removeBookmark(WebView* view);

    bool editBookmark(int id, QString title, QString folder);
    bool editBookmark(int id, QUrl url, QString title);

signals:

public slots:

private:
    bool m_showMostVisited;

};

#endif // BOOKMARKSMODEL_H
