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
#pragma once

#include <QAbstractProxyModel>

#include "qzcommon.h"

class QTimer;

class WebTab;
class BrowserWindow;
class TabTreeModelItem;

class QUPZILLA_EXPORT TabTreeModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    explicit TabTreeModel(BrowserWindow *window, QObject *parent = nullptr);
    ~TabTreeModel();

    QModelIndex tabIndex(WebTab *tab) const;
    WebTab *tab(const QModelIndex &index) const;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    bool hasChildren(const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;

    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

private:
    void init();
    QModelIndex index(TabTreeModelItem *item) const;
    TabTreeModelItem *item(const QModelIndex &index) const;
    TabTreeModelItem *createItems(TabTreeModelItem *root);

    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void sourceReset();

    void insertIndex(const QModelIndex &sourceIndex);
    void removeIndex(const QModelIndex &sourceIndex);
    void connectTab(WebTab *tab);
    void syncTopLevelTabs();

    BrowserWindow *m_window;
    TabTreeModelItem *m_root = nullptr;
    QHash<WebTab*, TabTreeModelItem*> m_items;
};
