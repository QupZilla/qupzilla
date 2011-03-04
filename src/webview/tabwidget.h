/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include "webview.h"
#include "webtab.h"
#include <QTabWidget>
#include <QTabBar>
#include <QDateTime>
#include <QToolButton>

class QupZilla;
class WebView;
class TabBar;
class WebTab;

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QupZilla* mainclass, QWidget *parent = 0);
    ~TabWidget();
    enum OpenUrlIn{ CurrentTab, NewSelectedTab, NewNotSelectedTab, NewTab = NewSelectedTab };

    QByteArray saveState();
    bool restoreState(const QByteArray &state);
    void setTabText(int index, const QString& text);
    void loadSettings();

    inline TabBar* getTabBar() { return m_tabBar; }
    inline bool canRestoreTab() { return m_canRestoreTab; }


public slots:
    void closeTab(int index=-1);
    int addView(QUrl url = QUrl(), QString title = tr("New tab"), OpenUrlIn openIn = NewTab, bool selectLine = false);
    void reloadTab(int index) { weView(index)->reload(); }
    void reloadAllTabs();
    void stopTab(int index) { weView(index)->stop(); }
    void backTab(int index) { weView(index)->back(); }
    void forwardTab(int index) { weView(index)->forward(); }
    void closeAllButCurrent(int index);
    void restoreClosedTab();

private slots:
    void tabChanged(int index);
    void aboutToShowTabsMenu();
    void actionChangeIndex();

private:
    inline WebView* weView() { WebTab* webTab = qobject_cast<WebTab*>(widget(currentIndex())); if (!webTab) return 0; return webTab->view(); }
    inline WebView* weView(int index) { WebTab* webTab = qobject_cast<WebTab*>(widget(index)); if (!webTab) return 0; return webTab->view(); }

    bool m_hideCloseButtonWithOneTab;
    bool m_hideTabBarWithOneTab;
    QUrl m_urlOnNewTab;
    QupZilla* p_QupZilla;

    bool m_canRestoreTab;
    int m_lastTabIndex;
    QUrl m_lastTabUrl;
    QByteArray m_lastTabHistory;

    TabBar* m_tabBar;

    QMenu* m_menuTabs;
    QToolButton* m_buttonAddTab;
    QToolButton* m_buttonListTabs;
};

#endif // TABWIDGET_H
