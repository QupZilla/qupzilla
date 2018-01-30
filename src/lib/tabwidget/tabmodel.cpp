/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "tabmodel.h"
#include "webtab.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "browserwindow.h"

#include <QMimeData>

TabModel::TabModel(BrowserWindow *window, QObject *parent)
    : QAbstractListModel(parent)
    , m_window(window)
{
    init();
}

WebTab *TabModel::webTab(int row) const
{
    return m_tabs.value(row);
}

int TabModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_window ? m_window->tabCount() : 0;
}

Qt::ItemFlags TabModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemIsDropEnabled;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

QVariant TabModel::data(const QModelIndex &index, int role) const
{
    if (!m_window || index.row() < 0 || index.row() > m_window->tabCount()) {
        return QVariant();
    }

    WebTab *tab = webTab(index.row());
    if (!tab) {
        return QVariant();
    }

    switch (role) {
    case WebTabRole:
        return QVariant::fromValue(tab);

    case TitleRole:
    case Qt::DisplayRole:
        return tab->title();

    case IconRole:
    case Qt::DecorationRole:
        return tab->icon();

    case PinnedRole:
        return tab->isPinned();

    case RestoredRole:
        return tab->isRestored();

    case ParentTabRole:
        return QVariant::fromValue(tab->parentTab());

    case ChildTabsRole:
        return QVariant::fromValue(tab->childTabs());

    default:
        return QVariant();
    }
}

Qt::DropActions TabModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

#define MIMETYPE QStringLiteral("application/qupzilla.tabmodel.tab")

QStringList TabModel::mimeTypes() const
{
    return {MIMETYPE};
}

QMimeData *TabModel::mimeData(const QModelIndexList &indexes) const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.isValid() && index.column() == 0) {
            stream << index.row();
        }
    }

    QMimeData *mimeData = new QMimeData();
    mimeData->setData(MIMETYPE, data);
    return mimeData;
}

bool TabModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (!m_window || !data->hasFormat(MIMETYPE) || parent.isValid() || column != 0) {
        return false;
    }

    QByteArray encodedData = data->data(MIMETYPE);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    QVector<WebTab*> tabs;
    while (!stream.atEnd()) {
        int index;
        stream >> index;
        WebTab *tab = webTab(index);
        if (tab) {
            tabs.append(tab);
        }
    }

    if (tabs.isEmpty()) {
        return false;
    }

    for (int i = 0; i < tabs.count(); ++i) {
        const int from = tabs.at(i)->tabIndex();
        const int to = row >= from ? row - 1 : row++;
        // FIXME: This switches order when moving > 2 non-contiguous indices
        m_window->tabWidget()->moveTab(from, to);
    }

    return true;
}

void TabModel::init()
{
    for (int i = 0; i < m_window->tabCount(); ++i) {
        tabInserted(i);
    }

    connect(m_window->tabWidget(), &TabWidget::tabInserted, this, &TabModel::tabInserted);
    connect(m_window->tabWidget(), &TabWidget::tabRemoved, this, &TabModel::tabRemoved);
    connect(m_window->tabWidget(), &TabWidget::tabMoved, this, &TabModel::tabMoved);

    connect(m_window, &QObject::destroyed, this, [this]() {
        beginResetModel();
        m_window = nullptr;
        for (WebTab *tab : qAsConst(m_tabs)) {
            tab->disconnect(this);
        }
        endResetModel();
    });
}

void TabModel::tabInserted(int index)
{
    WebTab *tab = m_window->weView(index)->webTab();

    beginInsertRows(QModelIndex(), index, index);
    m_tabs.insert(index, tab);
    endInsertRows();

    auto emitDataChanged = [this](WebTab *tab, int role) {
        const QModelIndex idx = TabModel::index(m_tabs.indexOf(tab), 0);
        emit dataChanged(idx, idx, {role});
    };

    connect(tab, &WebTab::titleChanged, this, std::bind(emitDataChanged, tab, Qt::DisplayRole));
    connect(tab, &WebTab::titleChanged, this, std::bind(emitDataChanged, tab, TitleRole));
    connect(tab, &WebTab::iconChanged, this, std::bind(emitDataChanged, tab, Qt::DecorationRole));
    connect(tab, &WebTab::iconChanged, this, std::bind(emitDataChanged, tab, IconRole));
    connect(tab, &WebTab::pinnedChanged, this, std::bind(emitDataChanged, tab, PinnedRole));
    connect(tab, &WebTab::restoredChanged, this, std::bind(emitDataChanged, tab, RestoredRole));
    connect(tab, &WebTab::parentTabChanged, this, std::bind(emitDataChanged, tab, ParentTabRole));
    connect(tab, &WebTab::childTabAdded, this, std::bind(emitDataChanged, tab, ChildTabsRole));
    connect(tab, &WebTab::childTabRemoved, this, std::bind(emitDataChanged, tab, ChildTabsRole));
}

void TabModel::tabRemoved(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_tabs.remove(index);
    endRemoveRows();
}

void TabModel::tabMoved(int from, int to)
{
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to);
    m_tabs.insert(to, m_tabs.takeAt(from));
    endMoveRows();
}
