/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#include "historytreeview.h"
#include "historymodel.h"
#include "historyitem.h"
#include "headerview.h"
#include "mainapplication.h"
#include "iconprovider.h"

#include <QClipboard>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>

HistoryTreeView::HistoryTreeView(QWidget* parent)
    : QTreeView(parent)
    , m_history(mApp->history())
    , m_filter(new HistoryFilterModel(m_history->model()))
    , m_type(HistoryManagerViewType)
{
    setModel(m_filter);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);

    m_header = new HeaderView(this);
    m_header->setDefaultSectionSizes(QList<double>() << 0.4 << 0.35 << 0.10 << 0.08);
    m_header->setSectionHidden(4, true);
    setHeader(m_header);

    connect(m_filter, SIGNAL(expandAllItems()), this, SLOT(expandAll()));
    connect(m_filter, SIGNAL(collapseAllItems()), this, SLOT(collapseAll()));
}

HistoryTreeView::ViewType HistoryTreeView::viewType() const
{
    return m_type;
}

void HistoryTreeView::setViewType(HistoryTreeView::ViewType type)
{
    m_type = type;

    switch (m_type) {
    case HistoryManagerViewType:
        setColumnHidden(1, false);
        setColumnHidden(2, false);
        setColumnHidden(3, false);
        setHeaderHidden(false);
        setMouseTracking(false);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        break;
    case HistorySidebarViewType:
        setColumnHidden(1, true);
        setColumnHidden(2, true);
        setColumnHidden(3, true);
        setHeaderHidden(true);
        setMouseTracking(true);
        setSelectionMode(QAbstractItemView::SingleSelection);
        break;
    default:
        break;
    }
}

QUrl HistoryTreeView::selectedUrl() const
{
    const QList<QModelIndex> indexes = selectionModel()->selectedRows();

    if (indexes.count() != 1)
        return QUrl();

    // TopLevelItems have invalid (empty) UrlRole data
    return indexes.at(0).data(HistoryModel::UrlRole).toUrl();
}

HeaderView* HistoryTreeView::header() const
{
    return m_header;
}

void HistoryTreeView::search(const QString &string)
{
    m_filter->setFilterFixedString(string);
}

void HistoryTreeView::removeSelectedItems()
{
    QList<int> list;
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QList<QPersistentModelIndex> topLevelIndexes;

    foreach (const QModelIndex &index, selectedIndexes()) {
        if (index.column() > 0) {
            continue;
        }

        if (index.data(HistoryModel::IsTopLevelRole).toBool()) {
            qint64 start = index.data(HistoryModel::TimestampStartRole).toLongLong();
            qint64 end = index.data(HistoryModel::TimestampEndRole).toLongLong();

            list.append(m_history->indexesFromTimeRange(start, end));

            topLevelIndexes.append(index);
        }
        else {
            int id = index.data(HistoryModel::IdRole).toInt();
            if (!list.contains(id)) {
                list.append(id);
            }
        }
    }

    m_history->deleteHistoryEntry(list);
    m_history->model()->removeTopLevelIndexes(topLevelIndexes);

    QApplication::restoreOverrideCursor();
}

void HistoryTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    emit contextMenuRequested(viewport()->mapToGlobal(event->pos()));
}

void HistoryTreeView::mouseMoveEvent(QMouseEvent* event)
{
    QTreeView::mouseMoveEvent(event);

    if (m_type == HistorySidebarViewType) {
        QCursor cursor = Qt::ArrowCursor;
        if (event->buttons() == Qt::NoButton) {
            QModelIndex index = indexAt(event->pos());
            if (index.isValid() && !index.data(HistoryModel::IsTopLevelRole).toBool()) {
                cursor = Qt::PointingHandCursor;
            }
        }
        viewport()->setCursor(cursor);
    }
}

void HistoryTreeView::mousePressEvent(QMouseEvent* event)
{
    QTreeView::mousePressEvent(event);

    if (selectionModel()->selectedRows().count() == 1) {
        QModelIndex index = indexAt(event->pos());
        Qt::MouseButtons buttons = event->buttons();
        Qt::KeyboardModifiers modifiers = event->modifiers();

        if (index.isValid() && !index.data(HistoryModel::IsTopLevelRole).toBool()) {
            const QUrl url = index.data(HistoryModel::UrlRole).toUrl();

            if (buttons == Qt::LeftButton && modifiers == Qt::ShiftModifier) {
                emit urlShiftActivated(url);
            }
            else if (buttons == Qt::MiddleButton || (buttons == Qt::LeftButton && modifiers == Qt::ControlModifier)) {
                emit urlCtrlActivated(url);
            }
        }
    }
}

void HistoryTreeView::mouseReleaseEvent(QMouseEvent* event)
{
    QTreeView::mouseReleaseEvent(event);

    if (selectionModel()->selectedRows().count() == 1) {
        QModelIndex index = indexAt(event->pos());

        if (index.isValid() && !index.data(HistoryModel::IsTopLevelRole).toBool()) {
            const QUrl url = index.data(HistoryModel::UrlRole).toUrl();

            // Activate urls with single mouse click in Sidebar
            if (m_type == HistorySidebarViewType && event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier) {
                emit urlActivated(url);
            }
        }
    }
}

void HistoryTreeView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QTreeView::mouseDoubleClickEvent(event);

    if (selectionModel()->selectedRows().count() == 1) {
        QModelIndex index = indexAt(event->pos());

        if (index.isValid() && !index.data(HistoryModel::IsTopLevelRole).toBool()) {
            const QUrl url = index.data(HistoryModel::UrlRole).toUrl();
            Qt::MouseButtons buttons = event->buttons();
            Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

            if (buttons == Qt::LeftButton && modifiers == Qt::NoModifier) {
                emit urlActivated(url);
            }
            else if (buttons == Qt::LeftButton && modifiers == Qt::ShiftModifier) {
                emit urlShiftActivated(url);
            }
        }
    }
}

void HistoryTreeView::keyPressEvent(QKeyEvent* event)
{
    QTreeView::keyPressEvent(event);

    if (selectionModel()->selectedRows().count() == 1) {
        QModelIndex index = selectionModel()->selectedRows().at(0);
        const QUrl url = index.data(HistoryModel::UrlRole).toUrl();
        const bool isTopLevel = index.data(HistoryModel::IsTopLevelRole).toBool();

        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (isTopLevel && (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::KeypadModifier)) {
                setExpanded(index, !isExpanded(index));
            }
            else {
                Qt::KeyboardModifiers modifiers = event->modifiers();

                if (modifiers == Qt::NoModifier || modifiers == Qt::KeypadModifier) {
                    emit urlActivated(url);
                }
                else if (modifiers == Qt::ControlModifier) {
                    emit urlCtrlActivated(url);
                }
                else if (modifiers == Qt::ShiftModifier) {
                    emit urlShiftActivated(url);
                }
            }
            break;

        case Qt::Key_Delete:
            removeSelectedItems();
            break;
        }
    }
}

void HistoryTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    bool itemTopLevel = index.data(HistoryModel::IsTopLevelRole).toBool();
    bool iconLoaded = !index.data(HistoryModel::IconRole).value<QIcon>().isNull();

    if (index.isValid() && !itemTopLevel && !iconLoaded) {
        const QPersistentModelIndex idx = index;
        model()->setData(idx, IconProvider::iconForUrl(index.data(HistoryModel::UrlRole).toUrl()), HistoryModel::IconRole);
    }

    QTreeView::drawRow(painter, options, index);
}
