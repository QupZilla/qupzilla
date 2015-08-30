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
#ifndef LIBRARY_H
#define LIBRARY_H

#include <QWidget>

#include "qzcommon.h"

namespace Ui
{
class BrowsingLibrary;
}

class HistoryManager;
class BookmarksManager;
class BrowserWindow;
class QUPZILLA_EXPORT BrowsingLibrary : public QWidget
{
    Q_OBJECT

public:
    explicit BrowsingLibrary(BrowserWindow* window, QWidget* parent = 0);
    ~BrowsingLibrary();

    void showHistory(BrowserWindow* window);
    void showBookmarks(BrowserWindow* window);

    void optimizeDatabase();

    HistoryManager* historyManager() { return m_historyManager; }
    BookmarksManager* bookmarksManager() { return m_bookmarksManager; }

private slots:
    void currentIndexChanged(int index);
    void search();

    void importBookmarks();
    void exportBookmarks();

private:
    void closeEvent(QCloseEvent* e);
    void keyPressEvent(QKeyEvent* e);

    Ui::BrowsingLibrary* ui;
    HistoryManager* m_historyManager;
    BookmarksManager* m_bookmarksManager;
};

#endif // LIBRARY_H
