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
#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QWidget>
#include <QPointer>

#include "qz_namespace.h"
#include "historyview.h"

namespace Ui
{
class HistoryManager;
}

class QTreeWidgetItem;

class QupZilla;

class QT_QUPZILLA_EXPORT HistoryManager : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryManager(QupZilla* mainClass, QWidget* parent = 0);
    ~HistoryManager();

    void setMainWindow(QupZilla* window);

    void restoreState(const QByteArray &state);
    QByteArray saveState();

public slots:
    void search(const QString &searchText);

private slots:
    void openLink(const QUrl &url, HistoryView::OpenBehavior openIn);
    void clearHistory();

private:
    QupZilla* getQupZilla();

    Ui::HistoryManager* ui;
    QPointer<QupZilla> p_QupZilla;
};

#endif // HISTORYMANAGER_H
