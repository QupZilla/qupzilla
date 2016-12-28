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
#ifndef BOOKMARKSMODEL_H
#define BOOKMARKSMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "qzcommon.h"

class QTimer;

class Bookmarks;
class BookmarkItem;

class QUPZILLA_EXPORT BookmarksModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        TypeRole = Qt::UserRole + 1,
        UrlRole = Qt::UserRole + 2,
        UrlStringRole = Qt::UserRole + 3,
        TitleRole = Qt::UserRole + 4,
        IconRole = Qt::UserRole + 5,
        DescriptionRole = Qt::UserRole + 6,
        KeywordRole = Qt::UserRole + 7,
        VisitCountRole = Qt::UserRole + 8,
        ExpandedRole = Qt::UserRole + 9,
        SidebarExpandedRole = Qt::UserRole + 10,
        MaxRole = SidebarExpandedRole
    };

    explicit BookmarksModel(BookmarkItem* root, Bookmarks* bookmarks, QObject* parent = 0);

    void addBookmark(BookmarkItem* parent, int row, BookmarkItem* item);
    void removeBookmark(BookmarkItem* item);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool hasChildren(const QModelIndex &parent) const;

    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    QModelIndex parent(const QModelIndex &child) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(BookmarkItem* item, int column = 0) const;

    BookmarkItem* item(const QModelIndex &index) const;

private slots:
    void bookmarkChanged(BookmarkItem* item);

private:
    BookmarkItem* m_root;
    Bookmarks* m_bookmarks;
};

class QUPZILLA_EXPORT BookmarksFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit BookmarksFilterModel(QAbstractItemModel* parent);

public slots:
    void setFilterFixedString(const QString &pattern);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private slots:
    void startFiltering();

private:
    QString m_pattern;
    QTimer* m_filterTimer;
};

#endif // BOOKMARKSMODEL_H
