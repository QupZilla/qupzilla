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
#ifndef LIBRARY_H
#define LIBRARY_H

#include <QWidget>

#include "qz_namespace.h"

namespace Ui
{
class BrowsingLibrary;
}

class HistoryManager;
class BookmarksManager;
class RSSManager;
class QupZilla;
class QT_QUPZILLA_EXPORT BrowsingLibrary : public QWidget
{
    Q_OBJECT

public:
    explicit BrowsingLibrary(QupZilla* mainClass, QWidget* parent = 0);
    ~BrowsingLibrary();

    void showHistory(QupZilla* mainClass);
    void showBookmarks(QupZilla* mainClass);
    void showRSS(QupZilla* mainClass);

    void optimizeDatabase();

    HistoryManager* historyManager() { return m_historyManager; }
    BookmarksManager* bookmarksManager() { return m_bookmarksManager; }
    RSSManager* rssManager() { return m_rssManager; }

private slots:
    void currentIndexChanged(int index);
    void search();

private:
    void closeEvent(QCloseEvent* e);
    void keyPressEvent(QKeyEvent* e);

    Ui::BrowsingLibrary* ui;
    HistoryManager* m_historyManager;
    BookmarksManager* m_bookmarksManager;
    RSSManager* m_rssManager;

    bool m_bookmarksLoaded;
    bool m_rssLoaded;
};

#endif // LIBRARY_H
