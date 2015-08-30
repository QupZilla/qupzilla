/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
* Copyright (C) 2013-2014  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#ifndef MACMENURECEIVER_H
#define MACMENURECEIVER_H

#include <QObject>

#include "qzcommon.h"

class QWebEngineFrame;
class Menu;
class QMenu;
class QAction;
class QMenuBar;

class QUPZILLA_EXPORT MacMenuReceiver : public QObject
{
    Q_OBJECT
public:
    MacMenuReceiver(QObject* parent = 0);

    inline QMenuBar* menuBar() { return m_macMenuBar; }
    inline void setMenuBar(QMenuBar* menuBar) { m_macMenuBar = menuBar; }

    inline bool bookmarksMenuChanged() { return m_bookmarksMenuChanged; }
    inline void setBookmarksMenuChanged(bool changed) { m_bookmarksMenuChanged = changed; }

    inline QAction* menuBookmarksAction() { return m_menuBookmarksAction; }
    inline void setMenuBookmarksAction(QAction* action) { m_menuBookmarksAction = action; }

public slots:
    void aboutToShowFileMenu(QMenu* menu = 0);
    void aboutToShowEditMenu(QMenu* menu = 0);
    void aboutToShowViewMenu(QMenu* menu = 0);
    void aboutToShowHistoryMenu(QMenu* menu = 0);
    void aboutToShowBookmarksMenu(QMenu* menu = 0);
    void aboutToShowToolsMenu(QMenu* menu = 0);

private:
    void setEnabledSelectedMenuActions(QMenu* menu, const QList<int> indexList = QList<int>());
    void setDisabledSelectedMenuActions(QMenu* menu, const QList<int> indexList = QList<int>());
    bool callSlot(const char* member, bool makeIfNoWindow = false,
                  QGenericArgument val0 = QGenericArgument(0),
                  QGenericArgument val1 = QGenericArgument());

    QMenuBar* m_macMenuBar;
    bool m_bookmarksMenuChanged;
    QAction* m_menuBookmarksAction;

private slots:
    void goNext();
    void goBack();
    void goHome();
    void stop();
    void reload();
    void reloadByPassCache();
    void aboutQupZilla();
    void addTab();
    void savePageScreen();

    void aboutToHideFileMenu();
    void aboutToHideHistoryMenu();
    void aboutToShowClosedTabsMenu();
    void aboutToHideViewMenu();
    void aboutToHideEditMenu();
    void aboutToHideToolsMenu();
    void aboutToShowEncodingMenu();

    void searchOnPage();
    void showCookieManager();
    void showHistoryManager();
    void showBookmarksManager();
    void showDownloadManager();
    void showMenubar();
    void showNavigationToolbar();
    void showStatusbar();
    void showClearPrivateData();
    void aboutToShowHistoryRecentMenu();
    void aboutToShowHistoryMostMenu();
    void showPreferences();
    void showBookmarkImport();

    void refreshHistory();
    void bookmarkAllTabs();
    void newWindow();

    void openLocation();
    void openFile();
    void savePage();
    void sendLink();
    void webSearch();

    // Edit menu actions
    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editSelectAll();

    void zoomIn();
    void zoomOut();
    void zoomReset();
    void toggleFullScreen();
    void changeEncoding(QObject* obj = 0);

    void triggerCaretBrowsing();
    void triggerTabsOnTop(bool enable);

    void closeWindow();
    void quitApp();

    void printPage(QWebEngineFrame* frame = 0);
    void showBookmarksToolbar();
    void showSource(QWebEngineFrame* frame = 0, const QString &selectedHtml = QString());
    void bookmarkPage();
    void showPageInfo();
    void showWebInspector(bool toggle = true);

    void loadActionUrl(QObject* obj = 0);
    void loadActionUrlInNewTab(QObject* obj = 0);
    void loadActionUrlInNewNotSelectedTab(QObject* obj = 0);

    void closeTab();
    void restoreClosedTab(QObject* obj = 0);
    void restoreAllClosedTabs();
    void clearClosedTabsList();
};
#endif // MACMENURECEIVER_H
