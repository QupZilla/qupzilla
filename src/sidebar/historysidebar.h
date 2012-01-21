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
#ifndef HISTORYSIDEBAR_H
#define HISTORYSIDEBAR_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QShortcut>
#include <QClipboard>

#include "historymodel.h"

namespace Ui
{
class HistorySideBar;
}

class QupZilla;
class HistorySideBar : public QWidget
{
    Q_OBJECT

public:
    explicit HistorySideBar(QupZilla* mainClass, QWidget* parent = 0);
    ~HistorySideBar();

public slots:
    void refreshTable();

private slots:
    void search();
    void itemDoubleClicked(QTreeWidgetItem* item);
    void contextMenuRequested(const QPoint &position);
    void loadInNewTab();
    void itemControlClicked(QTreeWidgetItem* item);

    void copyAddress();

    void historyEntryAdded(const HistoryModel::HistoryEntry &entry);
    void historyEntryDeleted(const HistoryModel::HistoryEntry &entry);
    void historyEntryEdited(const HistoryModel::HistoryEntry &before, const HistoryModel::HistoryEntry &after);

private:
    Ui::HistorySideBar* ui;
    QupZilla* p_QupZilla;
    HistoryModel* m_historyModel;
};

#endif // HISTORYSIDEBAR_H
