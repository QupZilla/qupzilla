/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QTabBar>
#include <QDateTime>
#include <QToolButton>
#include <QStylePainter>
#include <QStackedWidget>
#include <QTextDocument>

#include "webview.h"
#include "webtab.h"

class QupZilla;
class WebView;
class TabBar;
class WebTab;
class TabListButton;
class NewTabButton;
class ClosedTabsManager;
class ToolButton;
class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QupZilla* mainclass, QWidget* parent = 0);
    ~TabWidget();
    enum OpenUrlIn { CurrentTab, NewSelectedTab, NewNotSelectedTab, NewTab = NewSelectedTab, NewBackgroundTab = NewNotSelectedTab, CleanPage, CleanSelectedPage };

    QByteArray saveState();
    bool restoreState(const QByteArray &state);
    void savePinnedTabs();
    void restorePinnedTabs();

    void setTabText(int index, const QString &text);
    void loadSettings();

    inline TabBar* getTabBar() { return m_tabBar; }
    inline ClosedTabsManager* closedTabsManager() { return m_closedTabsManager; }
    bool canRestoreTab();
    QList<WebTab*> allTabs(bool withPinned = true);
    QStackedWidget* locationBars() { return m_locationBars; }
    ToolButton* buttonListTabs() { return m_buttonListTabs; }
    ToolButton* buttonAddTab() { return m_buttonAddTab; }

    void createKeyPressEvent(QKeyEvent* event);

signals:
    void pinnedTabClosed();
    void pinnedTabAdded();

public slots:
    int addView(QUrl url = QUrl(), const QString &title = tr("New tab"), OpenUrlIn openIn = NewTab, bool selectLine = false, int position = -1);
    int duplicateTab(int index);

    void closeTab(int index = -1);
    void reloadTab(int index) { weView(index)->reload(); }
    void reloadAllTabs();
    void stopTab(int index) { weView(index)->stop(); }
    void backTab(int index) { weView(index)->back(); }
    void forwardTab(int index) { weView(index)->forward(); }
    void closeAllButCurrent(int index);
    void restoreClosedTab();
    void restoreAllClosedTabs();
    void clearClosedTabsList();

    void moveAddTabButton(int posX);
    void showButtons();
    void hideButtons();

private slots:
    void aboutToShowTabsMenu();
    void actionChangeIndex();
    void currentTabChanged(int index);
    void tabMoved(int before, int after);

private:
    void resizeEvent(QResizeEvent* e);
    inline WebView* weView() { WebTab* webTab = qobject_cast<WebTab*>(widget(currentIndex())); if (!webTab) return 0; return webTab->view(); }
    inline WebView* weView(int index) { WebTab* webTab = qobject_cast<WebTab*>(widget(index)); if (!webTab) return 0; return webTab->view(); }

    bool m_hideTabBarWithOneTab;
    QUrl m_urlOnNewTab;
    QupZilla* p_QupZilla;

    int m_lastTabIndex;
    bool m_isClosingToLastTabIndex;

    TabBar* m_tabBar;

    QMenu* m_menuTabs;
    ToolButton* m_buttonListTabs;
    ToolButton* m_buttonAddTab;
    ClosedTabsManager* m_closedTabsManager;

    QStackedWidget* m_locationBars;
};

#endif // TABWIDGET_H
