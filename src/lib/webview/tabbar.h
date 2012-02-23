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
#ifndef TABBAR_H
#define TABBAR_H

#include <QTabBar>
#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QStyle>
#include <QApplication>

class QupZilla;
class TabWidget;
class TabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QupZilla* mainClass, TabWidget* tabWidget);
//    void hideCloseButton(int index);
//    void showCloseButton(int index);
    void updateCloseButton(int index);

    QSize getTabSizeHint(int index) { return QTabBar::tabSizeHint(index); }
    void loadSettings();
    QSize getTabSizeHint(int index) const { return QTabBar::tabSizeHint(index); }

    void setVisible(bool visible);
    void updateVisibilityWithFullscreen(bool visible);

    int pinnedTabsCount();
    int normalTabsCount();

    void disconnectObjects();

signals:
    void reloadTab(int index);
    void stopTab(int index);
    void backTab(int index);
    void forwardTab(int index);
    void closeAllButCurrent(int index);
    void closeTab(int index);
    void duplicateTab(int index);

    void moveAddTabButton(int posX);

    void showButtons();
    void hideButtons();

public slots:

private slots:
    void pinnedTabAdded();
    void pinnedTabClosed();

    void contextMenuRequested(const QPoint &position);
    void reloadTab() { emit reloadTab(m_clickedTab); }
    void stopTab() { emit stopTab(m_clickedTab); }
    void backTab() { emit backTab(m_clickedTab); }
    void forwardTab() { emit forwardTab(m_clickedTab); }
    void closeAllButCurrent() { emit closeAllButCurrent(m_clickedTab); }
    void closeTab() { emit closeTab(m_clickedTab); }
    void duplicateTab() { emit duplicateTab(m_clickedTab); }
    void bookmarkTab();
    void pinTab();
    void closeCurrentTab();

private:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    QSize tabSizeHint(int index) const;
//    void tabInserted(int index);

//    void emitMoveAddTabButton(int pox);

    QupZilla* p_QupZilla;
    TabWidget* m_tabWidget;

    bool m_showCloseButtonWithOneTab;
    bool m_showTabBarWithOneTab;

    int m_clickedTab;
    int m_pinnedTabsCount;

    int m_normalTabWidth;
    int m_lastTabWidth;
    bool m_adjustingLastTab;

    QPoint m_dragStartPosition;
};

#endif // TABBAR_H
