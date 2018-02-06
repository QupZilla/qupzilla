/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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

#include <QPointer>

#include "combotabbar.h"
#include "qzcommon.h"

class BrowserWindow;
class TabWidget;
class WebTab;

class QUPZILLA_EXPORT TabBar : public ComboTabBar
{
    Q_OBJECT
public:
    explicit TabBar(BrowserWindow* window, TabWidget* tabWidget);

    void loadSettings();

    TabWidget* tabWidget() const;
    void setVisible(bool visible);
    void setForceHidden(bool hidden);

    void setTabText(int index, const QString &text);

    void wheelEvent(QWheelEvent* event);

signals:
    void moveAddTabButton(int posX);

private slots:
    void currentTabChanged(int index);
    void overflowChanged(bool overflowed);
    void closeTabFromButton();

private:
    inline bool validIndex(int index) const { return index >= 0 && index < count(); }

    void tabInserted(int index);
    void tabRemoved(int index);

    void hideCloseButton(int index);
    void showCloseButton(int index);
    void updatePinnedTabCloseButton(int index);

    void contextMenuEvent(QContextMenuEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent* event);

    QSize tabSizeHint(int index, bool fast) const;
    int comboTabBarPixelMetric(ComboTabBar::SizeType sizeType) const;
    WebTab* webTab(int index = -1) const;

    BrowserWindow* m_window;
    TabWidget* m_tabWidget;

    bool m_hideTabBarWithOneTab;

    int m_showCloseOnInactive;

    mutable int m_normalTabWidth;
    mutable int m_activeTabWidth;

    QPoint m_dragStartPosition;

    bool m_forceHidden;
    QPointer<WebTab> m_lastTab;
};

#endif // TABBAR_H
