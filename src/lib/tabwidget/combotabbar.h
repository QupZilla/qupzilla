/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2013-2014 S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
* Copyright (C) 2014-2018 David Rosca <nowrep@gmail.com>
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

#include "qzcommon.h"
#include "wheelhelper.h"

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

class QUPZILLA_EXPORT ComboTabBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)
    Q_PROPERTY(int count READ count)

public:
    enum SizeType {
        PinnedTabWidth,
        ActiveTabMinimumWidth,
        NormalTabMinimumWidth,
        NormalTabMaximumWidth,
        OverflowedTabWidth,
        ExtraReservedWidth
    };

    enum DropIndicatorPosition {
        BeforeTab,
        AfterTab
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
    QRect draggedTabRect() const;
    QPixmap tabPixmap(int index) const;

    // Returns tab index at pos, or -1
    int tabAt(const QPoint &pos) const;

    // Returns true if there is an empty area at pos
    // (returns false if there are buttons or other widgets on the pos)
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

    void setFocusPolicy(Qt::FocusPolicy policy);
    void setObjectName(const QString &name);
    void setMouseTracking(bool enable);

    void insertCloseButton(int index);
    void setCloseButtonsToolTip(const QString &tip);

    QTabBar::ButtonPosition iconButtonPosition() const;
    QTabBar::ButtonPosition closeButtonPosition() const;

    QSize iconButtonSize() const;
    QSize closeButtonSize() const;

    bool validIndex(int index) const;
    void setCurrentNextEnabledIndex(int offset);

    bool usesScrollButtons() const;
    void setUsesScrollButtons(bool useButtons);

    void showDropIndicator(int index, DropIndicatorPosition position);
    void clearDropIndicator();

    bool isDragInProgress() const;
    bool isScrollInProgress() const;
    bool isMainBarOverflowed() const;

    // Width of all widgets in the corner
    int cornerWidth(Qt::Corner corner) const;
    // Add widget to the left/right corner
    void addCornerWidget(QWidget* widget, Qt::Corner corner);

    // Duration of tab slide animation when releasing dragged tab
    static int slideAnimationDuration();

public slots:
    void setUpLayout();
    void ensureVisible(int index = -1, int xmargin = -1);
    void setCurrentIndex(int index);

signals:
    void overFlowChanged(bool overFlow);
    void currentChanged(int index);
    void tabCloseRequested(int index);
    void tabMoved(int from, int to);
    void scrollBarValueChanged(int value);

private slots:
    void setMinimumWidths();
    void slotCurrentChanged(int index);
    void slotTabCloseRequested(int index);
    void slotTabMoved(int from, int to);
    void closeTabFromButton();
    void updateTabBars();
    void emitOverFlowChanged();

protected:
    int mainTabBarWidth() const;
    int pinTabBarWidth() const;

    bool event(QEvent *event);
    void wheelEvent(QWheelEvent* event);
    bool eventFilter(QObject* obj, QEvent* ev);
    void paintEvent(QPaintEvent* ev);

    virtual int comboTabBarPixelMetric(SizeType sizeType) const;
    virtual QSize tabSizeHint(int index, bool fast = false) const;
    virtual void tabInserted(int index);
    virtual void tabRemoved(int index);

private:
    TabBarHelper* mainTabBar() const;
    TabBarHelper* localTabBar(int index = -1) const;

    int toLocalIndex(int globalIndex) const;
    QRect mapFromLocalTabRect(const QRect &rect, QWidget *tabBar) const;
    void updatePinnedTabBarVisibility();

    QHBoxLayout* m_mainLayout;
    QHBoxLayout* m_leftLayout;
    QHBoxLayout* m_rightLayout;
    QWidget* m_leftContainer;
    QWidget* m_rightContainer;

    TabBarHelper* m_mainTabBar;
    TabBarHelper* m_pinnedTabBar;

    TabBarScrollWidget* m_mainTabBarWidget;
    TabBarScrollWidget* m_pinnedTabBarWidget;

    QString m_closeButtonsToolTip;
    bool m_mainBarOverFlowed;
    bool m_lastAppliedOverflow;
    bool m_usesScrollButtons;
    bool m_blockCurrentChangedSignal;

    WheelHelper m_wheelHelper;

    friend class TabBarHelper;
    friend class TabStackedWidget;
};

class QUPZILLA_EXPORT TabBarHelper : public QTabBar
{
    Q_OBJECT
    Q_PROPERTY(int tabPadding READ tabPadding WRITE setTabPadding)
    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor)

public:
    explicit TabBarHelper(bool isPinnedTabBar, ComboTabBar* comboTabBar);

    int tabPadding() const;
    void setTabPadding(int padding);

    QColor baseColor() const;
    void setBaseColor(const QColor &color);

    void setTabButton(int index, QTabBar::ButtonPosition position, QWidget* widget);

    QSize tabSizeHint(int index) const;
    QSize baseClassTabSizeHint(int index) const;

    QRect draggedTabRect() const;
    QPixmap tabPixmap(int index) const;

    bool isActiveTabBar();
    void setActiveTabBar(bool activate);

    void removeTab(int index);

    void setScrollArea(QScrollArea* scrollArea);
    void useFastTabSizeHint(bool enabled);

    void showDropIndicator(int index, ComboTabBar::DropIndicatorPosition position);
    void clearDropIndicator();

    bool isDisplayedOnViewPort(int globalLeft, int globalRight);
    bool isDragInProgress() const;

    static void initStyleBaseOption(QStyleOptionTabBarBase* optTabBase, QTabBar* tabbar, QSize size);

public slots:
    void setCurrentIndex(int index);

private:
    bool event(QEvent* ev);
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent* event);

    int dragOffset(QStyleOptionTab *option, int tabIndex) const;
    void initStyleOption(QStyleOptionTab* option, int tabIndex) const;

    ComboTabBar* m_comboTabBar;
    QScrollArea* m_scrollArea;

    int m_tabPadding = -1;
    QColor m_baseColor;
    int m_pressedIndex;
    bool m_dragInProgress;
    QPoint m_dragStartPosition;
    class QMovableTabWidget *m_movingTab = nullptr;
    bool m_activeTabBar;
    bool m_isPinnedTabBar;
    bool m_useFastTabSizeHint;
    int m_dropIndicatorIndex = -1;
    ComboTabBar::DropIndicatorPosition m_dropIndicatorPosition;
};

class QUPZILLA_EXPORT TabScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    explicit TabScrollBar(QWidget* parent = 0);
    ~TabScrollBar();

    bool isScrolling() const;

    void animateToValue(int to, QEasingCurve::Type type = QEasingCurve::OutQuad);

private:
    QPropertyAnimation* m_animation;
};


class QUPZILLA_EXPORT TabBarScrollWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabBarScrollWidget(QTabBar* tabBar, QWidget* parent = 0);

    QTabBar* tabBar();
    QScrollArea* scrollArea();
    TabScrollBar* scrollBar();

    void scrollByWheel(QWheelEvent* event);

    int scrollButtonsWidth() const;
    bool usesScrollButtons() const;
    void setUsesScrollButtons(bool useButtons);

    bool isOverflowed() const;
    int tabAt(const QPoint &pos) const;

public slots:
    void ensureVisible(int index = -1, int xmargin = 132);
    void scrollToLeft(int n = 5, QEasingCurve::Type type = QEasingCurve::OutQuad);
    void scrollToRight(int n = 5, QEasingCurve::Type type = QEasingCurve::OutQuad);
    void scrollToLeftEdge();
    void scrollToRightEdge();
    void setUpLayout();

private slots:
    void overFlowChanged(bool overflowed);
    void scrollStart();
    void updateScrollButtonsState();

private:
    void mouseMoveEvent(QMouseEvent* event);
    void resizeEvent(QResizeEvent* event);

    QTabBar* m_tabBar;
    QScrollArea* m_scrollArea;
    TabScrollBar* m_scrollBar;
    ToolButton* m_rightScrollButton;
    ToolButton* m_leftScrollButton;
    bool m_usesScrollButtons;
    int m_totalDeltas;
};

// Class for close button on tabs
// * taken from qtabbar.cpp
class CloseButton : public QAbstractButton
{
    Q_OBJECT

public:
    CloseButton(QWidget *parent = nullptr);

    QSize sizeHint() const override;

    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};
#endif // COMBOTABBAR_H
