/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "historyview.h"
#include "historymodel.h"
#include "historyitem.h"
#include "headerview.h"
#include "mainapplication.h"
#include "iconprovider.h"

#include <QClipboard>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>

HistoryView::HistoryView(QWidget* parent)
    : QTreeView(parent)
    , m_history(mApp->history())
    , m_filterModel(new HistoryFilterModel(m_history->model()))
{
    setModel(m_filterModel);

    setAllColumnsShowFocus(true);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_header = new HeaderView(this);
    setHeader(m_header);

    m_header->setDefaultSectionSizes(QList<double>() << 0.4 << 0.35 << 0.10 << 0.08);
    m_header->setSectionHidden(4, true);

    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
    connect(this, SIGNAL(pressed(QModelIndex)), this, SLOT(itemPressed(QModelIndex)));

    connect(m_filterModel, SIGNAL(expandAllItems()), this, SLOT(expandAll()));
    connect(m_filterModel, SIGNAL(collapseAllItems()), this, SLOT(collapseAll()));
}

HeaderView* HistoryView::header() const
{
    return m_header;
}

HistoryFilterModel* HistoryView::filterModel() const
{
    return m_filterModel;
}

void HistoryView::removeItems()
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

void HistoryView::itemActivated(const QModelIndex &index)
{
    if (!index.isValid() || index.data(HistoryModel::IsTopLevelRole).toBool()) {
        return;
    }

    emit openLink(index.data(HistoryModel::UrlRole).toUrl(), OpenInCurrentTab);
}

void HistoryView::itemPressed(const QModelIndex &index)
{
    if (!index.isValid() || index.data(HistoryModel::IsTopLevelRole).toBool()) {
        return;
    }

    if ((selectionMode() == QAbstractItemView::SingleSelection && QApplication::keyboardModifiers() & Qt::ControlModifier) ||
        QApplication::mouseButtons() & Qt::MiddleButton
       ) {
        emit openLink(index.data(HistoryModel::UrlRole).toUrl(), OpenInNewTab);
    }
}

void HistoryView::openLinkInCurrentTab()
{
    if (m_clickedIndex.isValid()) {
        emit openLink(m_clickedIndex.data(HistoryModel::UrlRole).toUrl(), OpenInCurrentTab);
    }
}

void HistoryView::openLinkInNewTab()
{
    if (m_clickedIndex.isValid()) {
        emit openLink(m_clickedIndex.data(HistoryModel::UrlRole).toUrl(), OpenInNewTab);
    }
}

void HistoryView::copyTitle()
{
    if (m_clickedIndex.isValid()) {
        QApplication::clipboard()->setText(m_clickedIndex.data(HistoryModel::TitleRole).toString());
    }
}

void HistoryView::copyAddress()
{
    if (m_clickedIndex.isValid()) {
        QApplication::clipboard()->setText(m_clickedIndex.data(HistoryModel::UrlStringRole).toString());
    }
}

void HistoryView::contextMenuEvent(QContextMenuEvent* event)
{
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid() || index.data(HistoryModel::IsTopLevelRole).toBool()) {
        return;
    }

    m_clickedIndex = index;

    QMenu menu;
    menu.addAction(tr("Open link in current tab"), this, SLOT(openLinkInCurrentTab()));
    menu.addAction(tr("Open link in new tab"), this, SLOT(openLinkInNewTab()));
    menu.addSeparator();
    menu.addAction(tr("Copy title"), this, SLOT(copyTitle()));
    menu.addAction(tr("Copy address"), this, SLOT(copyAddress()));
    menu.addSeparator();
    menu.addAction(tr("Remove"), this, SLOT(removeItems()));

    // Prevent choosing first option with double rightclick
    QPoint pos = viewport()->mapToGlobal(event->pos());
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);
}

void HistoryView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete) {
        removeItems();
        event->accept();
    }

    QTreeView::keyPressEvent(event);
}

void HistoryView::drawRow(QPainter* painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    bool itemTopLevel = index.data(HistoryModel::IsTopLevelRole).toBool();
    bool iconLoaded = index.data(HistoryModel::IconLoadedRole).toBool();

    if (index.isValid() && !itemTopLevel && !iconLoaded) {
        const QIcon icon = IconProvider::iconForUrl(index.data(HistoryModel::UrlRole).toUrl());
        model()->setData(index, icon, HistoryModel::IconRole);
    }

    QTreeView::drawRow(painter, options, index);
}
