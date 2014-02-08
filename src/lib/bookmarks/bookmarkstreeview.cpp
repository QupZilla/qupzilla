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
{
    setModel(m_model);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    header()->resizeSections(QHeaderView::ResizeToContents);

    restoreExpandedState(QModelIndex());

    connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(indexExpanded(QModelIndex)));
    connect(this, SIGNAL(collapsed(QModelIndex)), this, SLOT(indexCollapsed(QModelIndex)));
    connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged()));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(createContextMenu(QPoint)));
}

QList<BookmarkItem*> BookmarksTreeView::selectedBookmarks() const
{
    QList<BookmarkItem*> items;

    foreach (const QModelIndex &index, selectionModel()->selectedRows()) {
        BookmarkItem* item = m_model->item(index);
        items.append(item);
    }

    return items;
}

void BookmarksTreeView::indexExpanded(const QModelIndex &parent)
{
    BookmarkItem* item = m_model->item(parent);
    item->setExpanded(true);
}

void BookmarksTreeView::indexCollapsed(const QModelIndex &parent)
{
    BookmarkItem* item = m_model->item(parent);
    item->setExpanded(false);
}

void BookmarksTreeView::selectionChanged()
{
    emit bookmarksSelected(selectedBookmarks());
}

void BookmarksTreeView::createContextMenu(const QPoint &point)
{
    QModelIndex index = indexAt(point);
    BookmarkItem* item = index.isValid() ? m_model->item(index) : 0;
    emit contextMenuRequested(item);
}

void BookmarksTreeView::restoreExpandedState(const QModelIndex &parent)
{
    for (int i = 0; i < m_model->rowCount(parent); ++i) {
        QModelIndex index = m_model->index(i, 0, parent);
        BookmarkItem* item = m_model->item(index);
        setExpanded(index, item->isExpanded());
        restoreExpandedState(index);
    }
}

void BookmarksTreeView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    restoreExpandedState(parent);
    QTreeView::rowsInserted(parent, start, end);
}

void BookmarksTreeView::mousePressEvent(QMouseEvent* event)
{
    QModelIndex index = indexAt(event->pos());

    if (index.isValid()) {
        BookmarkItem* item = m_model->item(index);
        Qt::MouseButtons buttons = event->buttons();
        Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

        if (buttons == Qt::LeftButton && modifiers == Qt::ShiftModifier) {
            emit bookmarkShiftActivated(item);
        }
        else if (buttons == Qt::MiddleButton || modifiers == Qt::ControlModifier) {
            emit bookmarkCtrlActivated(item);
        }
    }

    QTreeView::mousePressEvent(event);
}

void BookmarksTreeView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QModelIndex index = indexAt(event->pos());

    if (index.isValid()) {
        BookmarkItem* item = m_model->item(index);
        Qt::MouseButtons buttons = event->buttons();
        Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

        if (buttons == Qt::LeftButton && modifiers == Qt::NoModifier) {
            emit bookmarkActivated(item);
        }
        else if (buttons == Qt::LeftButton && modifiers == Qt::ShiftModifier) {
            emit bookmarkShiftActivated(item);
        }
    }

    QTreeView::mouseDoubleClickEvent(event);
}

void BookmarksTreeView::keyPressEvent(QKeyEvent* event)
{
    if (selectionModel()->selectedRows().count() == 1) {
        QModelIndex index = selectionModel()->selectedRows().first();
        BookmarkItem* item = m_model->item(index);

        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (item->type() == BookmarkItem::Folder && event->modifiers() == Qt::NoModifier) {
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

    QTreeView::keyPressEvent(event);
}
