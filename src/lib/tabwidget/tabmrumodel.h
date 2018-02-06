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

class WebTab;
class BrowserWindow;

class TabMruModelItem;

class QUPZILLA_EXPORT TabMruModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    explicit TabMruModel(BrowserWindow *window, QObject *parent = nullptr);
    ~TabMruModel();

    QModelIndex tabIndex(WebTab *tab) const;
    WebTab *tab(const QModelIndex &index) const;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;

private:
    void init();
    QModelIndex index(TabMruModelItem *item) const;
    TabMruModelItem *item(const QModelIndex &index) const;

    void currentTabChanged(int index);
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void sourceReset();

    BrowserWindow *m_window;
    TabMruModelItem *m_root = nullptr;
    QHash<WebTab*, TabMruModelItem*> m_items;
};
