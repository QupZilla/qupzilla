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

#include "qz_namespace.h"

class QupZilla;
class TabWidget;
class TabPreview;

class QT_QUPZILLA_EXPORT TabBar : public QTabBar
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
    void closeAllButCurrent() { emit closeAllButCurrent(m_clickedTab); }
    void closeTab() { emit closeTab(m_clickedTab); }
    void duplicateTab() { emit duplicateTab(m_clickedTab); }
    void bookmarkTab();
    void pinTab();
    void closeCurrentTab();

    void showTabPreview();
    void hideTabPreview();

private:
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void leaveEvent(QEvent* event);
    void wheelEvent(QWheelEvent* event);

    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

    QSize tabSizeHint(int index) const;
//    void tabInserted(int index);

//    void emitMoveAddTabButton(int pox);

    QupZilla* p_QupZilla;
    TabWidget* m_tabWidget;
    TabPreview* m_tabPreview;

    bool m_showCloseButtonWithOneTab;
    bool m_showTabBarWithOneTab;

    int m_clickedTab;
    int m_pinnedTabsCount;
    int m_currentTabPreview;

    int m_normalTabWidth;
    int m_lastTabWidth;
    bool m_adjustingLastTab;

    QPoint m_dragStartPosition;
};

#endif // TABBAR_H
