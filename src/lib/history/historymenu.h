/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2018 David Rosca <nowrep@gmail.com>
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
#ifndef HISTORYMENU_H
#define HISTORYMENU_H

#include <QPointer>

#include "enhancedmenu.h"
#include "qzcommon.h"

class BrowserWindow;

class QUPZILLA_EXPORT HistoryMenu : public Menu
{
    Q_OBJECT

public:
    explicit HistoryMenu(QWidget* parent = 0);

    void setMainWindow(BrowserWindow* window);

signals:

private slots:
    void goBack();
    void goForward();
    void goHome();
    void showHistoryManager();

    void aboutToShow();
    void aboutToHide();

    void aboutToShowMostVisited();
    void aboutToShowClosedTabs();
    void aboutToShowClosedWindows();

    void historyEntryActivated();
    void historyEntryCtrlActivated();
    void historyEntryShiftActivated();

    void openUrl(const QUrl &url);
    void openUrlInNewTab(const QUrl &url);
    void openUrlInNewWindow(const QUrl &url);

private:
    void init();

    QPointer<BrowserWindow> m_window;
    Menu* m_menuMostVisited;
    Menu* m_menuClosedTabs;
    Menu *m_menuClosedWindows;
};

#endif // HISTORYMENU_H
