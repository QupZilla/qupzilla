/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014-2016 David Rosca <nowrep@gmail.com>
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
#ifndef MAINMENU_H
#define MAINMENU_H

#include <QMenu>
#include <QHash>
#include <QPointer>

#include "qzcommon.h"

class QMenuBar;

class Preferences;
class HistoryMenu;
class BookmarksMenu;
class BrowserWindow;

class QUPZILLA_EXPORT MainMenu : public QMenu
{
    Q_OBJECT

public:
    explicit MainMenu(BrowserWindow* window, QWidget* parent = 0);


    void initMenuBar(QMenuBar* menuBar) const;
    void initSuperMenu(QMenu* superMenu) const;

    QAction* action(const QString &name) const;

public slots:
    void setWindow(BrowserWindow* window);

private slots:
    // Standard actions
    void showAboutDialog();
    void showPreferences();
    void quitApplication();

    // File menu
    void newTab();
    void newWindow();
    void newPrivateWindow();
    void openLocation();
    void openFile();
    void closeWindow();
    void savePageAs();
    void sendLink();
    void printPage();

    // Edit menu
    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editSelectAll();
    void editFind();

    // View menu
    void showStatusBar();
    void stop();
    void reload();
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void showPageSource();
    void showFullScreen();

    // Tools menu
    void webSearch();
    void showSiteInfo();
    void showDownloadManager();
    void showCookieManager();
    void showAdBlockDialog();
    void toggleWebInspector();
    void showClearRecentHistoryDialog();

    // Help menu
    void aboutQt();
    void showInfoAboutApp();
    void showConfigInfo();
    void reportIssue();

    // Other actions
    void restoreClosedTab();

    void aboutToShowFileMenu();
    void aboutToHideFileMenu();
    void aboutToShowViewMenu();
    void aboutToHideViewMenu();
    void aboutToShowEditMenu();
    void aboutToHideEditMenu();
    void aboutToShowToolsMenu();
    void aboutToHideToolsMenu();
    void aboutToShowSuperMenu();
    void aboutToHideSuperMenu();

    void aboutToShowToolbarsMenu();
    void aboutToShowSidebarsMenu();
    void aboutToShowEncodingMenu();

private:
    void init();
    void addActionsToWindow();
    void callSlot(const char* slot);

    QHash<QString, QAction*> m_actions;
    QPointer<BrowserWindow> m_window;
    QPointer<Preferences> m_preferences;

    QMenu* m_menuFile;
    QMenu* m_menuEdit;
    QMenu* m_menuView;
    QMenu* m_menuTools;
    QMenu* m_menuHelp;
    QMenu* m_submenuExtensions;
    HistoryMenu* m_menuHistory;
    BookmarksMenu* m_menuBookmarks;
};

#endif // MAINMENU_H
