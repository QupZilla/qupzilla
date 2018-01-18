/* ============================================================
* QupZilla - WebKit based browser
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
#ifndef HISTORYSIDEBAR_H
#define HISTORYSIDEBAR_H

#include <QWidget>
#include <QUrl>

#include "qzcommon.h"

namespace Ui
{
class HistorySideBar;
}

class BrowserWindow;

class QUPZILLA_EXPORT HistorySideBar : public QWidget
{
    Q_OBJECT

public:
    explicit HistorySideBar(BrowserWindow* window, QWidget* parent = 0);
    ~HistorySideBar();

private slots:
    void urlActivated(const QUrl &url);
    void urlCtrlActivated(const QUrl &url);
    void urlShiftActivated(const QUrl &url);

    void openUrl(const QUrl &url = QUrl());
    void openUrlInNewTab(const QUrl &url = QUrl());
    void openUrlInNewWindow(const QUrl &url = QUrl());
    void openUrlInNewPrivateWindow(const QUrl &url = QUrl());

    void createContextMenu(const QPoint &pos);

private:
    Ui::HistorySideBar* ui;
    BrowserWindow* m_window;
};

#endif // HISTORYSIDEBAR_H
