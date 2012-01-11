/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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

#define _bookmarksToolbar BookmarksModel::toTranslatedFolder("bookmarksToolbar")
#define _bookmarksMenu BookmarksModel::toTranslatedFolder("bookmarksMenu")
#define _bookmarksUnsorted BookmarksModel::toTranslatedFolder("unsorted")

#include <QObject>
#include <QVariant>
#include <QUrl>
#include <QIcon>
#include <QSqlQuery>

class WebView;
class BookmarksModel : public QObject
{
    Q_OBJECT
public:
    explicit BookmarksModel(QObject* parent = 0);

    struct Bookmark {
        int id;
        QString title;
        QString folder;
        QUrl url;
        QIcon icon;
        bool inSubfolder;

        Bookmark() {
            id = -1;
            inSubfolder = false;
        }

        bool operator==(const Bookmark &other) const {
            return (this->title == other.title &&
                    this->folder == other.folder &&
                    this->url == other.url &&
                    this->inSubfolder == other.inSubfolder);
        }
    };

    void loadSettings();
    inline bool isShowingMostVisited() { return m_showMostVisited; }
    void setShowingMostVisited(bool state);

    bool isBookmarked(const QUrl &url);
    int bookmarkId(const QUrl &url);
    int bookmarkId(const QUrl &url, const QString &title, const QString &folder);
    Bookmark getBookmark(int id);

    bool saveBookmark(const QUrl &url, const QString &title, const QIcon &icon, const QString &folder = "unsorted");
    bool saveBookmark(WebView* view, const QString &folder = "unsorted");

    bool removeBookmark(int id);
    bool removeBookmark(const QUrl &url);
    bool removeBookmark(WebView* view);

    bool editBookmark(int id, const QString &title = "", const QUrl &url = QUrl(), const QString &folder = "");
//    bool editBookmark(int id, const QString &title, const QString &folder);
//    bool editBookmark(int id, const QUrl &url, const QString &title);

    bool createFolder(const QString &name);
    bool removeFolder(const QString &name);

    QList<Bookmark> folderBookmarks(const QString &name);

    bool createSubfolder(const QString &name);
    bool isSubfolder(const QString &name);

    bool renameFolder(const QString &before, const QString &after);

    static bool bookmarksEqual(const Bookmark &one, const Bookmark &two);
    static QString toTranslatedFolder(const QString &name);
    static QString fromTranslatedFolder(const QString &name);

signals:
    void bookmarkAdded(const BookmarksModel::Bookmark &bookmark);
    void bookmarkDeleted(const BookmarksModel::Bookmark &bookmark);
    void bookmarkEdited(const BookmarksModel::Bookmark &before, const BookmarksModel::Bookmark &after);

    void folderAdded(const QString &title);
    void folderDeleted(const QString &title);

    void subfolderAdded(const QString &title);
    void folderRenamed(const QString &before, const QString &after);

public slots:

private:
    bool m_showMostVisited;
};

typedef BookmarksModel::Bookmark Bookmark;

Q_DECLARE_METATYPE(BookmarksModel::Bookmark)

#endif // BOOKMARKSMODEL_H
