/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include <QRect>
#include <QAbstractButton>

#include "qz_namespace.h"

class QupZilla;
class TabWidget;
class TabPreview;

class QT_QUPZILLA_EXPORT TabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QupZilla* mainClass, TabWidget* tabWidget);

    void loadSettings();

    void setVisible(bool visible);
    void updateVisibilityWithFullscreen(bool visible);

    int pinnedTabsCount();
    int normalTabsCount();

    void updatePinnedTabCloseButton(int index);

    void disconnectObjects();

    void wheelEvent(QWheelEvent* event);

signals:
    void reloadTab(int index);
    void stopTab(int index);
    void closeAllButCurrent(int index);
    void closeTab(int index);
    void duplicateTab(int index);

    void moveAddTabButton(int posX);

    void showButtons();
    void hideButtons();

private slots:
    void currentTabChanged(int index);
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
    void closeTabFromButton();

    void showTabPreview();
    void hideTabPreview(bool delayed = true);

private:
    inline bool validIndex(int index) const { return index >= 0 && index < count(); }

    void tabRemoved(int index);

    void hideCloseButton(int index);
    void showCloseButton(int index);

    void mouseDoubleClickEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    bool event(QEvent* event);

    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

    QSize tabSizeHint(int index) const;

    QupZilla* p_QupZilla;
    TabWidget* m_tabWidget;
    TabPreview* m_tabPreview;
    QTimer* m_tabPreviewTimer;

    bool m_showTabPreviews;

    int m_clickedTab;
    int m_pinnedTabsCount;

    mutable int m_normalTabWidth;
    mutable int m_activeTabWidth;

    QRect m_originalGeometry;
    QPoint m_dragStartPosition;
};

// Class for close button on tabs
// * taken from qtabbar.cpp
class CloseButton : public QAbstractButton
{
    Q_OBJECT

public:
    CloseButton(QWidget* parent = 0);

    QSize sizeHint() const;
    inline QSize minimumSizeHint() const { return sizeHint(); }

    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void paintEvent(QPaintEvent* event);
};

#endif // TABBAR_H
