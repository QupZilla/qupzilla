/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#include <QUrl>

#include "qzcommon.h"

namespace Ui
{
class HistoryManager;
}

class QTreeWidgetItem;

class BrowserWindow;

class QUPZILLA_EXPORT HistoryManager : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryManager(BrowserWindow* window, QWidget* parent = 0);
    ~HistoryManager();

    void setMainWindow(BrowserWindow* window);

    void restoreState(const QByteArray &state);
    QByteArray saveState();

public slots:
    void search(const QString &searchText);

private slots:
    void urlActivated(const QUrl &url);
    void urlCtrlActivated(const QUrl &url);
    void urlShiftActivated(const QUrl &url);

    void openUrl(const QUrl &url = QUrl());
    void openUrlInNewTab(const QUrl &url = QUrl());
    void openUrlInNewWindow(const QUrl &url = QUrl());
    void openUrlInNewPrivateWindow(const QUrl &url = QUrl());

    void createContextMenu(const QPoint &pos);

    void copyUrl();
    void copyTitle();
    void clearHistory();

private:
    void keyPressEvent(QKeyEvent *event) override;

    BrowserWindow* getWindow();

    Ui::HistoryManager* ui;
    QPointer<BrowserWindow> m_window;
};

#endif // HISTORYMANAGER_H
