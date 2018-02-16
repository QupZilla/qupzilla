/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2017 David Rosca <nowrep@gmail.com>
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
#include "bookmarksmodel.h"
#include "bookmarkitem.h"
#include "bookmarks.h"

#include <QApplication>
#include <QMimeData>
#include <QTimer>
#include <QStyle>

//#define BOOKMARKSMODEL_DEBUG

#ifdef BOOKMARKSMODEL_DEBUG
#include "modeltest.h"
#endif

BookmarksModel::BookmarksModel(BookmarkItem* root, Bookmarks* bookmarks, QObject* parent)
    : QAbstractItemModel(parent)
    , m_root(root)
    , m_bookmarks(bookmarks)
{
    if (m_bookmarks) {
        connect(m_bookmarks, SIGNAL(bookmarkChanged(BookmarkItem*)), this, SLOT(bookmarkChanged(BookmarkItem*)));
    }

#ifdef BOOKMARKSMODEL_DEBUG
    new ModelTest(this, this);
#endif
}

void BookmarksModel::addBookmark(BookmarkItem* parent, int row, BookmarkItem* item)
{
    Q_ASSERT(parent);
    Q_ASSERT(item);
    Q_ASSERT(row >= 0);
    Q_ASSERT(row <= parent->children().count());

    beginInsertRows(index(parent), row, row);
    parent->addChild(item, row);
    endInsertRows();
}

void BookmarksModel::removeBookmark(BookmarkItem* item)
{
    Q_ASSERT(item);
    Q_ASSERT(item->parent());

    int idx = item->parent()->children().indexOf(item);

    beginRemoveRows(index(item->parent()), idx, idx);
    item->parent()->removeChild(item);
    endRemoveRows();
}

Qt::ItemFlags BookmarksModel::flags(const QModelIndex &index) const
{
    BookmarkItem* itm = item(index);

    if (!index.isValid() || !itm) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags =  Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (itm->isFolder()) {
        flags |= Qt::ItemIsDropEnabled;
    }

    if (m_bookmarks && m_bookmarks->canBeModified(itm)) {
        flags |= Qt::ItemIsDragEnabled;
    }

    return flags;
}

QVariant BookmarksModel::data(const QModelIndex &index, int role) const
{
    BookmarkItem* itm = item(index);

    if (!itm) {
        return QVariant();
    }

    switch (role) {
    case TypeRole:
        return itm->type();
    case UrlRole:
        return itm->url();
    case UrlStringRole:
        return itm->urlString();
    case TitleRole:
        return itm->title();
    case DescriptionRole:
        return itm->description();
    case KeywordRole:
        return itm->keyword();
    case VisitCountRole:
        return -1;
    case ExpandedRole:
        return itm->isExpanded();
    case SidebarExpandedRole:
        return itm->isSidebarExpanded();
    case Qt::ToolTipRole:
        if (index.column() == 0 && itm->isUrl()) {
            return QString("%1\n%2").arg(itm->title(), QString::fromUtf8(itm->url().toEncoded()));
        }
        // fallthrough
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return itm->title();
        case 1:
            return itm->url().toEncoded();
        default:
            return QVariant();
        }
    case Qt::DecorationRole:
        if (index.column() == 0) {
            return itm->icon();
        }
        return QVariant();
    default:
        return QVariant();
    }
}

QVariant BookmarksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Title");
        case 1:
            return tr("Address");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

int BookmarksModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    BookmarkItem* itm = item(parent);
    return itm->children().count();
}

int BookmarksModel::columnCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    return 2;
}

bool BookmarksModel::hasChildren(const QModelIndex &parent) const
{
    BookmarkItem* itm = item(parent);
    return !itm->children().isEmpty();
}

Qt::DropActions BookmarksModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

#define MIMETYPE QLatin1String("application/qupzilla.bookmarks")

QStringList BookmarksModel::mimeTypes() const
{
    QStringList types;
    types.append(MIMETYPE);
    return types;
}

QMimeData* BookmarksModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        // If item's parent (=folder) is also selected, we will just move the whole folder
        if (index.isValid() && index.column() == 0 && !indexes.contains(index.parent())) {
            stream << index.row() << (quintptr) index.internalPointer();
        }
    }

    mimeData->setData(MIMETYPE, encodedData);
    return mimeData;
}

bool BookmarksModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column)

    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (!m_bookmarks || !data->hasFormat(MIMETYPE) || !parent.isValid()) {
        return false;
    }

    BookmarkItem* parentItm = item(parent);
    Q_ASSERT(parentItm->isFolder());

    QByteArray encodedData = data->data(MIMETYPE);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<BookmarkItem*> items;

    while (!stream.atEnd()) {
        int row;
        quintptr ptr;

        stream >> row >> ptr;

        QModelIndex index = createIndex(row, 0, (void*) ptr);
        BookmarkItem* itm = item(index);

        Q_ASSERT(index.isValid());
        Q_ASSERT(itm != m_bookmarks->rootItem());

        // Cannot move bookmark to itself
        if (itm == parentItm) {
            return false;
        }

        items.append(itm);
    }

    row = qMax(row, 0);

    foreach (BookmarkItem* itm, items) {
        // If we are moving an item through the folder and item is above the row to insert,
        // we must decrease row by one (by the dropped folder)
        if (itm->parent() == parentItm && itm->parent()->children().indexOf(itm) < row) {
            row--;
        }

        m_bookmarks->removeBookmark(itm);
        m_bookmarks->insertBookmark(parentItm, row++, itm);
    }

    return true;
}

QModelIndex BookmarksModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    BookmarkItem* itm = item(child);
    return index(itm->parent());
}

QModelIndex BookmarksModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    BookmarkItem* parentItem = item(parent);
    return createIndex(row, column, parentItem->children().at(row));
}

QModelIndex BookmarksModel::index(BookmarkItem* item, int column) const
{
    BookmarkItem* parent = item->parent();

    if (!parent) {
        return QModelIndex();
    }

    return createIndex(parent->children().indexOf(item), column, item);
}

BookmarkItem* BookmarksModel::item(const QModelIndex &index) const
{
    BookmarkItem* itm = static_cast<BookmarkItem*>(index.internalPointer());
    return itm ? itm : m_root;
}

void BookmarksModel::bookmarkChanged(BookmarkItem* item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}


// BookmarksFilterModel
BookmarksFilterModel::BookmarksFilterModel(QAbstractItemModel* parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(parent);
    setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_filterTimer = new QTimer(this);
    m_filterTimer->setSingleShot(true);
    m_filterTimer->setInterval(300);

    connect(m_filterTimer, SIGNAL(timeout()), this, SLOT(startFiltering()));
}

void BookmarksFilterModel::setFilterFixedString(const QString &pattern)
{
    m_pattern = pattern;

    m_filterTimer->start();
}

bool BookmarksFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (index.data(BookmarksModel::TypeRole).toInt() == BookmarkItem::Folder) {
        return true;
    }

    return (index.data(BookmarksModel::TitleRole).toString().contains(m_pattern, filterCaseSensitivity()) ||
            index.data(BookmarksModel::UrlStringRole).toString().contains(m_pattern, filterCaseSensitivity()) ||
            index.data(BookmarksModel::DescriptionRole).toString().contains(m_pattern, filterCaseSensitivity()) ||
            index.data(BookmarksModel::KeywordRole).toString().compare(m_pattern, filterCaseSensitivity()) == 0);
}

void BookmarksFilterModel::startFiltering()
{
    QSortFilterProxyModel::setFilterFixedString(m_pattern);
}
