/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QWeakPointer>
#include <QClipboard>

#include "historymodel.h"

namespace Ui
{
class HistoryManager;
}

class QupZilla;
class HistoryManager : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryManager(QupZilla* mainClass, QWidget* parent = 0);
    ~HistoryManager();

    void setMainWindow(QupZilla* window);

public slots:
    void refreshTable();
    void search(const QString &searchText);

private slots:
    void optimizeDb();
    void itemDoubleClicked(QTreeWidgetItem* item);

    void deleteItem();
    void clearHistory();
    void contextMenuRequested(const QPoint &position);
    void loadInNewTab();
    void copyUrl();

    void historyEntryAdded(const HistoryModel::HistoryEntry &entry);
    void historyEntryDeleted(const HistoryModel::HistoryEntry &entry);
    void historyEntryEdited(const HistoryModel::HistoryEntry &before, const HistoryModel::HistoryEntry &after);

private:
    QupZilla* getQupZilla();
    Ui::HistoryManager* ui;
    QWeakPointer<QupZilla> p_QupZilla;
    HistoryModel* m_historyModel;

    QList<int> m_ignoredIds;
};

#endif // HISTORYMANAGER_H
