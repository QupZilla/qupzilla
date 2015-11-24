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
#include "bookmarkstreeview.h"
#include "bookmarksitemdelegate.h"
#include "bookmarksmodel.h"
#include "bookmarkitem.h"
#include "bookmarks.h"
#include "mainapplication.h"

#include <QHeaderView>
#include <QMouseEvent>

BookmarksTreeView::BookmarksTreeView(QWidget* parent)
    : QTreeView(parent)
    , m_bookmarks(mApp->bookmarks())
    , m_model(m_bookmarks->model())
    , m_filter(new BookmarksFilterModel(m_model))
    , m_type(BookmarksManagerViewType)
{
    setModel(m_filter);
    setDragEnabled(true);
    setAcceptDrops(true);
    setUniformRowHeights(true);
    setDropIndicatorShown(true);
    setAllColumnsShowFocus(true);
    setItemDelegate(new BookmarksItemDelegate(this));
    header()->resizeSections(QHeaderView::ResizeToContents);

    connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(indexExpanded(QModelIndex)));
    connect(this, SIGNAL(collapsed(QModelIndex)), this, SLOT(indexCollapsed(QModelIndex)));
    connect(selectionModel(), SIGNAL(slotSelectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSelectionChanged()));
}

BookmarksTreeView::ViewType BookmarksTreeView::viewType() const
{
    return m_type;
}

void BookmarksTreeView::setViewType(BookmarksTreeView::ViewType type)
{
    m_type = type;

    switch (m_type) {
    case BookmarksManagerViewType:
        setColumnHidden(1, false);
        setHeaderHidden(false);
        setMouseTracking(false);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        break;
    case BookmarksSidebarViewType:
        setColumnHidden(1, true);
        setHeaderHidden(true);
        setMouseTracking(true);
        setSelectionMode(QAbstractItemView::SingleSelection);
        break;
    default:
        break;
    }

    restoreExpandedState(QModelIndex());
}

BookmarkItem* BookmarksTreeView::selectedBookmark() const
{
    QList<BookmarkItem*> items = selectedBookmarks();
    return items.count() == 1 ? items.first() : 0;
}

QList<BookmarkItem*> BookmarksTreeView::selectedBookmarks() const
{
    QList<BookmarkItem*> items;

    foreach (const QModelIndex &index, selectionModel()->selectedRows()) {
        BookmarkItem* item = m_model->item(m_filter->mapToSource(index));
        items.append(item);
    }

    return items;
}

void BookmarksTreeView::selectBookmark(BookmarkItem* item)
{
    QModelIndex col0 = m_filter->mapFromSource(m_model->index(item, 0));
    QModelIndex col1 = m_filter->mapFromSource(m_model->index(item, 1));

    selectionModel()->clearSelection();
    selectionModel()->select(col0, QItemSelectionModel::Select);
    selectionModel()->select(col1, QItemSelectionModel::Select);
}

void BookmarksTreeView::ensureBookmarkVisible(BookmarkItem* item)
{
    QModelIndex index = m_filter->mapFromSource(m_model->index(item));
    QModelIndex parent = m_filter->parent(index);

    while (parent.isValid()) {
        setExpanded(parent, true);
        parent = m_filter->parent(parent);
    }
}

void BookmarksTreeView::search(const QString &string)
{
    m_filter->setFilterFixedString(string);
}

void BookmarksTreeView::indexExpanded(const QModelIndex &parent)
{
    BookmarkItem* item = m_model->item(m_filter->mapToSource(parent));

    switch (m_type) {
    case BookmarksManagerViewType:
        item->setExpanded(true);
        break;
    case BookmarksSidebarViewType:
        item->setSidebarExpanded(true);
        break;
    default:
        break;
    }
}

void BookmarksTreeView::indexCollapsed(const QModelIndex &parent)
{
    BookmarkItem* item = m_model->item(m_filter->mapToSource(parent));

    switch (m_type) {
    case BookmarksManagerViewType:
        item->setExpanded(false);
        break;
    case BookmarksSidebarViewType:
        item->setSidebarExpanded(false);
        break;
    default:
        break;
    }
}

void BookmarksTreeView::slotSelectionChanged()
{
    emit bookmarksSelected(selectedBookmarks());
}

void BookmarksTreeView::restoreExpandedState(const QModelIndex &parent)
{
    for (int i = 0; i < m_filter->rowCount(parent); ++i) {
        QModelIndex index = m_filter->index(i, 0, parent);
        BookmarkItem* item = m_model->item(m_filter->mapToSource(index));
        setExpanded(index, m_type == BookmarksManagerViewType ? item->isExpanded() : item->isSidebarExpanded());
        restoreExpandedState(index);
    }
}

void BookmarksTreeView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    restoreExpandedState(parent);
    QTreeView::rowsInserted(parent, start, end);
}

void BookmarksTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    emit contextMenuRequested(viewport()->mapToGlobal(event->pos()));
}

void BookmarksTreeView::mouseMoveEvent(QMouseEvent* event)
{
    QTreeView::mouseMoveEvent(event);

    if (m_type == BookmarksSidebarViewType) {
        QCursor cursor = Qt::ArrowCursor;
        if (event->buttons() == Qt::NoButton) {
            QModelIndex index = indexAt(event->pos());
            if (index.isValid() && index.data(BookmarksModel::TypeRole).toInt() == BookmarkItem::Url) {
                cursor = Qt::PointingHandCursor;
            }
        }
        viewport()->setCursor(cursor);
    }
}

void BookmarksTreeView::mousePressEvent(QMouseEvent* event)
{
    QTreeView::mousePressEvent(event);

    if (selectionModel()->selectedRows().count() == 1) {
        QModelIndex index = indexAt(event->pos());
        Qt::MouseButtons buttons = event->buttons();
        Qt::KeyboardModifiers modifiers = event->modifiers();

        if (index.isValid()) {
            BookmarkItem* item = m_model->item(m_filter->mapToSource(index));

            if (buttons == Qt::LeftButton && modifiers == Qt::ShiftModifier) {
                emit bookmarkShiftActivated(item);
            }
            else if (buttons == Qt::MiddleButton || (buttons == Qt::LeftButton && modifiers == Qt::ControlModifier)) {
                emit bookmarkCtrlActivated(item);
            }
        }
    }
}

void BookmarksTreeView::mouseReleaseEvent(QMouseEvent* event)
{
    QTreeView::mouseReleaseEvent(event);

    if (selectionModel()->selectedRows().count() == 1) {
        QModelIndex index = indexAt(event->pos());

        if (index.isValid()) {
            BookmarkItem* item = m_model->item(m_filter->mapToSource(index));

            // Activate bookmarks with single mouse click in Sidebar
            if (m_type == BookmarksSidebarViewType && event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier) {
                emit bookmarkActivated(item);
            }
        }
    }
}

void BookmarksTreeView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QTreeView::mouseDoubleClickEvent(event);

    if (selectionModel()->selectedRows().count() == 1) {
        QModelIndex index = indexAt(event->pos());

        if (index.isValid()) {
            BookmarkItem* item = m_model->item(m_filter->mapToSource(index));
            Qt::MouseButtons buttons = event->buttons();
            Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

            if (buttons == Qt::LeftButton && modifiers == Qt::NoModifier) {
                emit bookmarkActivated(item);
            }
            else if (buttons == Qt::LeftButton && modifiers == Qt::ShiftModifier) {
                emit bookmarkShiftActivated(item);
            }
        }
    }
}

void BookmarksTreeView::keyPressEvent(QKeyEvent* event)
{
    QTreeView::keyPressEvent(event);

    if (selectionModel()->selectedRows().count() == 1) {
        QModelIndex index = selectionModel()->selectedRows().first();
        BookmarkItem* item = m_model->item(m_filter->mapToSource(index));

        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (item->isFolder() && event->modifiers() == Qt::NoModifier) {
                setExpanded(index, !isExpanded(index));
            }
            else {
                Qt::KeyboardModifiers modifiers = event->modifiers();

                if (modifiers == Qt::NoModifier) {
                    emit bookmarkActivated(item);
                }
                else if (modifiers == Qt::ControlModifier) {
                    emit bookmarkCtrlActivated(item);
                }
                else if (modifiers == Qt::ShiftModifier) {
                    emit bookmarkShiftActivated(item);
                }
            }
            break;
        }
    }
}
