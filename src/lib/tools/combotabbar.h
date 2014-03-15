/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#ifndef COMBOTABBAR_H
#define COMBOTABBAR_H

#include "qz_namespace.h"

#include <QTabBar>
#include <QScrollBar>
#include <QAbstractButton>
#include <QEasingCurve>
#include <QStyleOption>

class QScrollArea;
class QPropertyAnimation;
class QHBoxLayout;

class TabBarScrollWidget;
class TabBarHelper;
class ToolButton;

class QT_QUPZILLA_EXPORT ComboTabBar : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)
    Q_PROPERTY(int count READ count)

    friend class TabBarHelper;

public:
    enum SizeType {
        PinnedTabWidth,
        ActiveTabMinimumWidth,
        NormalTabMinimumWidth,
        NormalTabMaximumWidth,
        OverflowedTabWidth,
        ExtraReservedWidth
    };

    explicit ComboTabBar(QWidget* parent = 0);

    int addTab(const QString &text);
    int addTab(const QIcon &icon, const QString &text);

    int insertTab(int index, const QString &text);
    int insertTab(int index, const QIcon &icon, const QString &text, bool pinned = false);

    void removeTab(int index);
    void moveTab(int from, int to);

    bool isTabEnabled(int index) const;
    void setTabEnabled(int index, bool enabled);

    QColor tabTextColor(int index) const;
    void setTabTextColor(int index, const QColor &color);

    QRect tabRect(int index) const;
    int tabAt(const QPoint &pos) const;
    bool emptyArea(const QPoint &pos) const;

    int mainTabBarCurrentIndex() const;
    int currentIndex() const;
    int count() const;

    void setDrawBase(bool drawTheBase);
    bool drawBase() const;

    Qt::TextElideMode elideMode() const;
    void setElideMode(Qt::TextElideMode elide);

    QString tabText(int index) const;
    void setTabText(int index, const QString &text);

    void setTabToolTip(int index, const QString &tip);
    QString tabToolTip(int index) const;

    bool tabsClosable() const;
    void setTabsClosable(bool closable);

    void setTabButton(int index, QTabBar::ButtonPosition position, QWidget* widget);
    QWidget* tabButton(int index, QTabBar::ButtonPosition position) const;

    QTabBar::SelectionBehavior selectionBehaviorOnRemove() const;
    void setSelectionBehaviorOnRemove(QTabBar::SelectionBehavior behavior);

    bool expanding() const;
    void setExpanding(bool enabled);

    bool isMovable() const;
    void setMovable(bool movable);

    bool documentMode() const;
    void setDocumentMode(bool set);

    int pinnedTabsCount() const;
    int normalTabsCount() const;
    bool isPinned(int index) const;
    void setMaxVisiblePinnedTab(int max);

    void setObjectName(const QString &name);
    void setMouseTracking(bool enable);

    void insertCloseButton(int index);
    void setCloseButtonsToolTip(const QString &tip);
    void enableBluredBackground(bool enable);

    QTabBar::ButtonPosition iconButtonPosition();
    QTabBar::ButtonPosition closeButtonPosition();

    bool validIndex(int index) const;
    void setCurrentNextEnabledIndex(int offset);

    bool usesScrollButtons() const;
    void setUsesScrollButtons(bool useButtons);

    bool isDragInProgress() const;
    void addMainBarWidget(QWidget* widget, Qt::Alignment align, int stretch = 0, Qt::Alignment layoutAlignment = 0);

public slots:
    void setUpLayout();
    void ensureVisible(int index = -1, int xmargin = -1);
    void setCurrentIndex(int index);

private slots:
    void setMinimumWidths();
    void slotCurrentChanged(int index);
    void slotTabCloseRequested(int index);
    void slotTabMoved(int from, int to);
    void closeTabFromButton();
    void enableUpdates();
    void updateTabBars();

protected:
    int mainTabBarWidth() const;
    int pinTabBarWidth() const;
    void wheelEvent(QWheelEvent* event);
    void showEvent(QShowEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject* obj, QEvent* ev);

    virtual int comboTabBarPixelMetric(SizeType sizeType) const;
    virtual QSize tabSizeHint(int index, bool fast = false) const;
    virtual void tabInserted(int index);
    virtual void tabRemoved(int index);

private:
    TabBarHelper* localTabBar(int index = -1) const;
    int toLocalIndex(int globalIndex) const;

    inline TabBarHelper* mainTabBar() { return m_mainTabBar; }
    void updatePinnedTabBarVisibility();

    QHBoxLayout* m_mainLayout;

    TabBarHelper* m_mainTabBar;
    TabBarHelper* m_pinnedTabBar;

    TabBarScrollWidget* m_mainTabBarWidget;
    TabBarScrollWidget* m_pinnedTabBarWidget;

    QString m_closeButtonsToolTip;
    bool m_mainBarOverFlowed;
    bool m_usesScrollButtons;

signals:
    void overFlowChanged(bool overFlow);
    void currentChanged(int index);
    void tabCloseRequested(int index);
    void tabMoved(int from, int to);
    void scrollBarValueChanged(int value);
};

class QT_QUPZILLA_EXPORT TabBarHelper : public QTabBar
{
    Q_OBJECT

public:
    explicit TabBarHelper(ComboTabBar* comboTabBar);

    void setTabButton(int index, QTabBar::ButtonPosition position, QWidget* widget);

    QSize tabSizeHint(int index) const;
    QSize baseClassTabSizeHint(int index) const;

    bool isActiveTabBar();
    void setActiveTabBar(bool activate);

    void removeTab(int index);

    void setScrollArea(QScrollArea* scrollArea);
    void useFastTabSizeHint(bool enabled);

    bool isDisplayedOnViewPort(int globalLeft, int globalRight);
    bool isDragInProgress() const;
    void enableBluredBackground(bool enable);

public slots:
    void setCurrentIndex(int index);

private slots:
    void resetDragState();

private:
    void initStyleBaseOption(QStyleOptionTabBarBaseV2* optTabBase, QTabBar* tabbar, QSize size);
    bool event(QEvent* ev);
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    ComboTabBar* m_comboTabBar;
    QScrollArea* m_scrollArea;

    int m_pressedIndex;
    int m_pressedGlobalX;
    bool m_dragInProgress;
    bool m_activeTabBar;
    bool m_useFastTabSizeHint;
    bool m_bluredBackground;
};

class QT_QUPZILLA_EXPORT TabScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    explicit TabScrollBar(QWidget* parent = 0);

    ~TabScrollBar();

    void animateToValue(int to, QEasingCurve::Type type = QEasingCurve::OutQuad);

private:
    QPropertyAnimation* m_animation;
};


class QT_QUPZILLA_EXPORT TabBarScrollWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabBarScrollWidget(QTabBar* tabBar, QWidget* parent = 0);

    void addLeftWidget(QWidget* widget, int stretch = 0, Qt::Alignment alignment = 0);
    void addRightWidget(QWidget* widget, int stretch = 0, Qt::Alignment alignment = 0);

    QTabBar* tabBar();
    QScrollArea* scrollArea();
    TabScrollBar* scrollBar();

    void scrollByWheel(QWheelEvent* event);

    bool usesScrollButtons() const;
    void setUsesScrollButtons(bool useButtons);

    bool isOverflowed() const;
    int tabAt(const QPoint &pos) const;

    void setContainersName(const QString &name);
    void enableBluredBackground(bool enable);

public slots:
    void ensureVisible(int index = -1, int xmargin = 132);
    void scrollToLeft(int n = 5, QEasingCurve::Type type = QEasingCurve::OutQuad);
    void scrollToRight(int n = 5, QEasingCurve::Type type = QEasingCurve::OutQuad);
    void scrollToLeftEdge();
    void scrollToRightEdge();
    void setUpLayout();

private slots:
    void scrollBarValueChange();
    void overFlowChanged(bool overflowed);
    void scrollStart();

private:
    bool eventFilter(QObject* obj, QEvent* ev);
    void mouseMoveEvent(QMouseEvent* event);

    QTabBar* m_tabBar;
    QScrollArea* m_scrollArea;
    TabScrollBar* m_scrollBar;
    QHBoxLayout* m_leftLayout;
    QHBoxLayout* m_rightLayout;
    ToolButton* m_rightScrollButton;
    ToolButton* m_leftScrollButton;
    QWidget* m_leftContainer;
    QWidget* m_rightContainer;
    bool m_usesScrollButtons;
    bool m_bluredBackground;
    int m_totalDeltas;
};

// Class for close button on tabs
// * taken from qtabbar.cpp
class CloseButton : public QAbstractButton
{
    Q_OBJECT

public:
    CloseButton(QWidget* parent = 0);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void paintEvent(QPaintEvent* event);
};
#endif // COMBOTABBAR_H
