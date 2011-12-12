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
#ifndef BOOKMARKSMANAGER_H
#define BOOKMARKSMANAGER_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QWeakPointer>
#include <QDialogButtonBox>
#include <QShortcut>

#include "bookmarksmodel.h"

namespace Ui
{
class BookmarksManager;
}

class WebView;
class QupZilla;
class BookmarksModel;
class BookmarksManager : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksManager(QupZilla* mainClass, QWidget* parent = 0);
    ~BookmarksManager();
    void addBookmark(WebView* view);
    void insertBookmark(const QUrl &url, const QString &title, const QIcon &icon);
    void setMainWindow(QupZilla* window);

    void search(const QString &string);

public slots:
    void refreshTable();
    void insertAllTabs();

private slots:
    void optimizeDb();
    void deleteItem();
    void itemChanged(QTreeWidgetItem* item);
    void addSubfolder();
    void addFolder();
    void renameFolder();
    void contextMenuRequested(const QPoint &position);
    void loadInNewTab();
    void itemControlClicked(QTreeWidgetItem* item);
    void moveBookmark();
    void renameBookmark();
    void importBookmarks();

    void addFolder(const QString &name);
    void addSubfolder(const QString &name);
    void removeFolder(const QString &name);
    void renameFolder(const QString &before, const QString &after);

    void addBookmark(const BookmarksModel::Bookmark &bookmark);
    void removeBookmark(const BookmarksModel::Bookmark &bookmark);
    void bookmarkEdited(const BookmarksModel::Bookmark &before, const BookmarksModel::Bookmark &after);

private:
    QupZilla* getQupZilla();

    bool m_isRefreshing;
    Ui::BookmarksManager* ui;
    QWeakPointer<QupZilla> p_QupZilla;
    BookmarksModel* m_bookmarksModel;
};

#endif // BOOKMARKSMANAGER_H
