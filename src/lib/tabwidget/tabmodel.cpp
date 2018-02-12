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
#include "browserwindow.h"

// TabModelMimeData
TabModelMimeData::TabModelMimeData()
    : QMimeData()
{
}

WebTab *TabModelMimeData::tab() const
{
    return m_tab;
}

void TabModelMimeData::setTab(WebTab *tab)
{
    m_tab = tab;
}

bool TabModelMimeData::hasFormat(const QString &format) const
{
    return mimeType() == format;
}

QStringList TabModelMimeData::formats() const
{
    return {mimeType()};
}

// static
QString TabModelMimeData::mimeType()
{
    return QSL("application/qupzilla.tabmodel.tab");
}

// TabModel
TabModel::TabModel(BrowserWindow *window, QObject *parent)
    : QAbstractListModel(parent)
    , m_window(window)
{
    init();
}

QModelIndex TabModel::tabIndex(WebTab *tab) const
{
    const int idx = m_tabs.indexOf(tab);
    if (idx < 0) {
        return QModelIndex();
    }
    return index(idx);
}

WebTab *TabModel::tab(const QModelIndex &index) const
{
    return m_tabs.value(index.row());
}

int TabModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_tabs.count();
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
    if (index.row() < 0 || index.row() > m_tabs.count()) {
        return QVariant();
    }

    WebTab *t = tab(index);
    if (!t) {
        return QVariant();
    }

    switch (role) {
    case WebTabRole:
        return QVariant::fromValue(t);

    case TitleRole:
    case Qt::DisplayRole:
        return t->title();

    case IconRole:
    case Qt::DecorationRole:
        return t->icon();

    case PinnedRole:
        return t->isPinned();

    case RestoredRole:
        return t->isRestored();

    case CurrentTabRole:
        return t->isCurrentTab();

    case LoadingRole:
        return t->isLoading();

    case AudioPlayingRole:
        return t->isPlaying();

    case AudioMutedRole:
        return t->isMuted();

    case BackgroundActivityRole:
        return t->backgroundActivity();

    default:
        return QVariant();
    }
}

Qt::DropActions TabModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList TabModel::mimeTypes() const
{
    return {TabModelMimeData::mimeType()};
}

QMimeData *TabModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty()) {
        return nullptr;
    }
    WebTab *tab = indexes.at(0).data(WebTabRole).value<WebTab*>();
    if (!tab) {
        return nullptr;
    }
    TabModelMimeData *mimeData = new TabModelMimeData;
    mimeData->setTab(tab);
    return mimeData;
}

bool TabModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(row)
    if (action != Qt::MoveAction || parent.isValid() || column > 0 || !m_window) {
        return false;
    }
    const TabModelMimeData *mimeData = qobject_cast<const TabModelMimeData*>(data);
    if (!mimeData) {
        return false;
    }
    return mimeData->tab();
}

bool TabModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!canDropMimeData(data, action, row, column, parent)) {
        return false;
    }

    const TabModelMimeData *mimeData = static_cast<const TabModelMimeData*>(data);
    WebTab *tab = mimeData->tab();

    if (tab->browserWindow() == m_window) {
        if (row < 0) {
            row = tab->isPinned() ? m_window->tabWidget()->pinnedTabsCount() : m_window->tabWidget()->count();
        }
        tab->moveTab(row > mimeData->tab()->tabIndex() ? row - 1 : row);
    } else {
        if (row < 0) {
            row = m_window->tabCount();
        }
        if (tab->browserWindow()) {
            tab->browserWindow()->tabWidget()->detachTab(tab);
        }
        tab->setPinned(row < m_window->tabWidget()->pinnedTabsCount());
        m_window->tabWidget()->insertView(row, tab, Qz::NT_SelectedTab);
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
        m_tabs.clear();
        endResetModel();
    });
}

void TabModel::tabInserted(int index)
{
    WebTab *tab = m_window->tabWidget()->webTab(index);

    beginInsertRows(QModelIndex(), index, index);
    m_tabs.insert(index, tab);
    endInsertRows();

    auto emitDataChanged = [this](WebTab *tab, int role) {
        const QModelIndex idx = tabIndex(tab);
        emit dataChanged(idx, idx, {role});
    };

    connect(tab, &WebTab::titleChanged, this, std::bind(emitDataChanged, tab, Qt::DisplayRole));
    connect(tab, &WebTab::titleChanged, this, std::bind(emitDataChanged, tab, TitleRole));
    connect(tab, &WebTab::iconChanged, this, std::bind(emitDataChanged, tab, Qt::DecorationRole));
    connect(tab, &WebTab::iconChanged, this, std::bind(emitDataChanged, tab, IconRole));
    connect(tab, &WebTab::pinnedChanged, this, std::bind(emitDataChanged, tab, PinnedRole));
    connect(tab, &WebTab::restoredChanged, this, std::bind(emitDataChanged, tab, RestoredRole));
    connect(tab, &WebTab::currentTabChanged, this, std::bind(emitDataChanged, tab, CurrentTabRole));
    connect(tab, &WebTab::loadingChanged, this, std::bind(emitDataChanged, tab, LoadingRole));
    connect(tab, &WebTab::playingChanged, this, std::bind(emitDataChanged, tab, AudioPlayingRole));
    connect(tab, &WebTab::mutedChanged, this, std::bind(emitDataChanged, tab, AudioMutedRole));
    connect(tab, &WebTab::backgroundActivityChanged, this, std::bind(emitDataChanged, tab, BackgroundActivityRole));
}

void TabModel::tabRemoved(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_tabs.remove(index);
    endRemoveRows();
}

void TabModel::tabMoved(int from, int to)
{
    if (!beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to)) {
        qWarning() << "Invalid beginMoveRows" << from << (to > from ? to + 1 : to);
        return;
    }
    m_tabs.insert(to, m_tabs.takeAt(from));
    endMoveRows();
}
