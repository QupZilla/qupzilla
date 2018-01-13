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
#include "combotabbar.h"
#include "toolbutton.h"
#include "tabicon.h"
#include "mainapplication.h"
#include "proxystyle.h"
#include "qzsettings.h"

#include <QIcon>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QStyleOptionTabV3>
#include <QStyleOptionTabBarBaseV2>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QTimer>
#include <QTabBar>
#include <QMouseEvent>
#include <QApplication>
#include <QToolTip>

class QMovableTabWidget : public QWidget
{
public:
    QPixmap m_pixmap;
};

ComboTabBar::ComboTabBar(QWidget* parent)
    : QWidget(parent)
    , m_mainTabBar(0)
    , m_pinnedTabBar(0)
    , m_mainBarOverFlowed(false)
    , m_lastAppliedOverflow(false)
    , m_usesScrollButtons(false)
    , m_blockCurrentChangedSignal(false)
{
    QObject::setObjectName(QSL("tabbarwidget"));

    m_mainTabBar = new TabBarHelper(/*isPinnedTabBar*/ false, this);
    m_pinnedTabBar = new TabBarHelper(/*isPinnedTabBar*/ true, this);
    m_mainTabBarWidget = new TabBarScrollWidget(m_mainTabBar, this);
    m_pinnedTabBarWidget = new TabBarScrollWidget(m_pinnedTabBar, this);

    m_mainTabBar->setScrollArea(m_mainTabBarWidget->scrollArea());
    m_pinnedTabBar->setScrollArea(m_pinnedTabBarWidget->scrollArea());

    connect(m_mainTabBarWidget->scrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(setMinimumWidths()));
    connect(m_mainTabBarWidget->scrollBar(), SIGNAL(valueChanged(int)), this, SIGNAL(scrollBarValueChanged(int)));
    connect(m_pinnedTabBarWidget->scrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(setMinimumWidths()));
    connect(m_pinnedTabBarWidget->scrollBar(), SIGNAL(valueChanged(int)), this, SIGNAL(scrollBarValueChanged(int)));
    connect(this, SIGNAL(overFlowChanged(bool)), m_mainTabBarWidget, SLOT(overFlowChanged(bool)));

    m_mainTabBar->setActiveTabBar(true);
    m_pinnedTabBar->setTabsClosable(false);

    m_leftLayout = new QHBoxLayout;
    m_leftLayout->setSpacing(0);
    m_leftLayout->setContentsMargins(0, 0, 0, 0);
    m_leftContainer = new QWidget(this);
    m_leftContainer->setLayout(m_leftLayout);

    m_rightLayout = new QHBoxLayout;
    m_rightLayout->setSpacing(0);
    m_rightLayout->setContentsMargins(0, 0, 0, 0);
    m_rightContainer = new QWidget(this);
    m_rightContainer->setLayout(m_rightLayout);

    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_leftContainer);
    m_mainLayout->addWidget(m_pinnedTabBarWidget);
    m_mainLayout->addWidget(m_mainTabBarWidget);
    m_mainLayout->addWidget(m_rightContainer);
    setLayout(m_mainLayout);

    connect(m_mainTabBar, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
    connect(m_mainTabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(slotTabCloseRequested(int)));
    connect(m_mainTabBar, SIGNAL(tabMoved(int,int)), this, SLOT(slotTabMoved(int,int)));

    connect(m_pinnedTabBar, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
    connect(m_pinnedTabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(slotTabCloseRequested(int)));
    connect(m_pinnedTabBar, SIGNAL(tabMoved(int,int)), this, SLOT(slotTabMoved(int,int)));

    setAutoFillBackground(false);
    m_mainTabBar->setAutoFillBackground(false);
    m_pinnedTabBar->setAutoFillBackground(false);

    m_mainTabBar->installEventFilter(this);
    m_pinnedTabBar->installEventFilter(this);
    m_leftContainer->installEventFilter(this);
    m_rightContainer->installEventFilter(this);
    m_mainTabBarWidget->installEventFilter(this);
    m_pinnedTabBarWidget->installEventFilter(this);
}

int ComboTabBar::addTab(const QString &text)
{
    return insertTab(-1, text);
}

int ComboTabBar::addTab(const QIcon &icon, const QString &text)
{
    return insertTab(-1, icon, text);
}

int ComboTabBar::insertTab(int index, const QString &text)
{
    return insertTab(index, QIcon(), text);
}

int ComboTabBar::insertTab(int index, const QIcon &icon, const QString &text, bool pinned)
{
    if (pinned) {
        index = m_pinnedTabBar->insertTab(index, icon, text);
    }
    else {
        index = m_mainTabBar->insertTab(index - pinnedTabsCount(), icon, text);

        if (tabsClosable()) {
            QWidget* closeButton = m_mainTabBar->tabButton(index, closeButtonPosition());
            if ((closeButton && closeButton->objectName() != QLatin1String("combotabbar_tabs_close_button")) || !closeButton) {
                // insert our close button
                insertCloseButton(index + pinnedTabsCount());
                if (closeButton) {
                    closeButton->deleteLater();
                }
            }
        }

        index += pinnedTabsCount();
    }

    updatePinnedTabBarVisibility();
    tabInserted(index);
    setMinimumWidths();

    return index;
}

void ComboTabBar::removeTab(int index)
{
    if (validIndex(index)) {
        setUpdatesEnabled(false);

        localTabBar(index)->removeTab(toLocalIndex(index));
        updatePinnedTabBarVisibility();
        tabRemoved(index);
        setMinimumWidths();

        setUpdatesEnabled(true);
        updateTabBars();
    }
}

void ComboTabBar::moveTab(int from, int to)
{
    if (from >= pinnedTabsCount() && to >= pinnedTabsCount()) {
        m_mainTabBar->moveTab(from - pinnedTabsCount(), to - pinnedTabsCount());
    }
    else if (from < pinnedTabsCount() && to < pinnedTabsCount()) {
        m_pinnedTabBar->moveTab(from, to);
    }
}

bool ComboTabBar::isTabEnabled(int index) const
{
    return localTabBar(index)->isTabEnabled(toLocalIndex(index));
}

void ComboTabBar::setTabEnabled(int index, bool enabled)
{
    localTabBar(index)->setTabEnabled(toLocalIndex(index), enabled);
}

QColor ComboTabBar::tabTextColor(int index) const
{
    return localTabBar(index)->tabTextColor(toLocalIndex(index));
}

void ComboTabBar::setTabTextColor(int index, const QColor &color)
{
    localTabBar(index)->setTabTextColor(toLocalIndex(index), color);
}

QRect ComboTabBar::tabRect(int index) const
{
    return mapFromLocalTabRect(localTabBar(index)->tabRect(toLocalIndex(index)), localTabBar(index));
}

QRect ComboTabBar::draggedTabRect() const
{
    const QRect r = m_pinnedTabBar->draggedTabRect();
    if (r.isValid()) {
        return mapFromLocalTabRect(r, m_pinnedTabBar);
    }
    return mapFromLocalTabRect(m_mainTabBar->draggedTabRect(), m_mainTabBar);
}

QPixmap ComboTabBar::tabPixmap(int index) const
{
    return localTabBar(index)->tabPixmap(toLocalIndex(index));
}

int ComboTabBar::tabAt(const QPoint &pos) const
{
    QWidget* w = QApplication::widgetAt(mapToGlobal(pos));
    if (!qobject_cast<TabBarHelper*>(w) && !qobject_cast<TabIcon*>(w) && !qobject_cast<CloseButton*>(w))
        return -1;

    if (m_pinnedTabBarWidget->geometry().contains(pos)) {
        return m_pinnedTabBarWidget->tabAt(m_pinnedTabBarWidget->mapFromParent(pos));
    } else if (m_mainTabBarWidget->geometry().contains(pos)) {
        int index = m_mainTabBarWidget->tabAt(m_mainTabBarWidget->mapFromParent(pos));
        if (index != -1)
            index += pinnedTabsCount();
        return index;
    }

    return -1;
}

bool ComboTabBar::emptyArea(const QPoint &pos) const
{
    if (tabAt(pos) != -1)
        return false;

    return qobject_cast<TabBarHelper*>(QApplication::widgetAt(mapToGlobal(pos)));
}

int ComboTabBar::mainTabBarCurrentIndex() const
{
    return (m_mainTabBar->currentIndex() == -1 ? -1 : pinnedTabsCount() + m_mainTabBar->currentIndex());
}

int ComboTabBar::currentIndex() const
{
    if (m_pinnedTabBar->isActiveTabBar()) {
        return m_pinnedTabBar->currentIndex();
    }
    else {
        return (m_mainTabBar->currentIndex() == -1 ? -1 : pinnedTabsCount() + m_mainTabBar->currentIndex());
    }
}

void ComboTabBar::setCurrentIndex(int index)
{
    return localTabBar(index)->setCurrentIndex(toLocalIndex(index));
}

void ComboTabBar::slotCurrentChanged(int index)
{
    if (m_blockCurrentChangedSignal) {
        return;
    }

    if (sender() == m_pinnedTabBar) {
        if (index == -1 && m_mainTabBar->count() > 0) {
            m_mainTabBar->setActiveTabBar(true);
            m_pinnedTabBar->setActiveTabBar(false);
            emit currentChanged(pinnedTabsCount());
        }
        else {
            m_pinnedTabBar->setActiveTabBar(true);
            m_mainTabBar->setActiveTabBar(false);
            emit currentChanged(index);
        }
    }
    else {
        if (index == -1 && pinnedTabsCount() > 0) {
            m_pinnedTabBar->setActiveTabBar(true);
            m_mainTabBar->setActiveTabBar(false);
            emit currentChanged(pinnedTabsCount() - 1);
        }
        else {
            m_mainTabBar->setActiveTabBar(true);
            m_pinnedTabBar->setActiveTabBar(false);
            emit currentChanged(index + pinnedTabsCount());
        }
    }
}

void ComboTabBar::slotTabCloseRequested(int index)
{
    if (sender() == m_pinnedTabBar) {
        emit tabCloseRequested(index);
    }
    else {
        emit tabCloseRequested(index + pinnedTabsCount());
    }
}

void ComboTabBar::slotTabMoved(int from, int to)
{
    if (sender() == m_pinnedTabBar) {
        emit tabMoved(from, to);
    }
    else {
        emit tabMoved(from + pinnedTabsCount(), to + pinnedTabsCount());
    }
}

void ComboTabBar::closeTabFromButton()
{
    QWidget* button = qobject_cast<QWidget*>(sender());

    int tabToClose = -1;

    for (int i = 0; i < m_mainTabBar->count(); ++i) {
        if (m_mainTabBar->tabButton(i, closeButtonPosition()) == button) {
            tabToClose = i;
            break;
        }
    }

    if (tabToClose != -1) {
        emit tabCloseRequested(tabToClose + pinnedTabsCount());
    }
}

void ComboTabBar::updateTabBars()
{
    m_mainTabBar->update();
    m_pinnedTabBar->update();
}

void ComboTabBar::emitOverFlowChanged()
{
    if (m_mainBarOverFlowed != m_lastAppliedOverflow) {
        emit overFlowChanged(m_mainBarOverFlowed);
        m_lastAppliedOverflow = m_mainBarOverFlowed;
    }
}

int ComboTabBar::count() const
{
    return pinnedTabsCount() + m_mainTabBar->count();
}

void ComboTabBar::setDrawBase(bool drawTheBase)
{
    m_mainTabBar->setDrawBase(drawTheBase);
    m_pinnedTabBar->setDrawBase(drawTheBase);
}

bool ComboTabBar::drawBase() const
{
    return m_mainTabBar->drawBase();
}

Qt::TextElideMode ComboTabBar::elideMode() const
{
    return m_mainTabBar->elideMode();
}

void ComboTabBar::setElideMode(Qt::TextElideMode elide)
{
    m_mainTabBar->setElideMode(elide);
    m_pinnedTabBar->setElideMode(elide);
}

QString ComboTabBar::tabText(int index) const
{
    return localTabBar(index)->tabText(toLocalIndex(index));
}

void ComboTabBar::setTabText(int index, const QString &text)
{
    localTabBar(index)->setTabText(toLocalIndex(index), text);
}

void ComboTabBar::setTabToolTip(int index, const QString &tip)
{
    localTabBar(index)->setTabToolTip(toLocalIndex(index), tip);
}

QString ComboTabBar::tabToolTip(int index) const
{
    return localTabBar(index)->tabToolTip(toLocalIndex(index));
}

bool ComboTabBar::tabsClosable() const
{
    return m_mainTabBar->tabsClosable();
}

void ComboTabBar::setTabsClosable(bool closable)
{
    if (closable == tabsClosable()) {
        return;
    }

    if (closable) {
        // insert our close button
        for (int i = 0; i < m_mainTabBar->count(); ++i) {
            QWidget* closeButton = m_mainTabBar->tabButton(i, closeButtonPosition());
            if (closeButton) {
                if (closeButton->objectName() == QLatin1String("combotabbar_tabs_close_button")) {
                    continue;
                }
            }

            insertCloseButton(i + pinnedTabsCount());
            if (closeButton) {
                closeButton->deleteLater();
            }
        }
    }
    m_mainTabBar->setTabsClosable(closable);
}

void ComboTabBar::setTabButton(int index, QTabBar::ButtonPosition position, QWidget* widget)
{
    if (widget)
        widget->setMinimumSize(closeButtonSize());
    localTabBar(index)->setTabButton(toLocalIndex(index), position, widget);
}

QWidget* ComboTabBar::tabButton(int index, QTabBar::ButtonPosition position) const
{
    return localTabBar(index)->tabButton(toLocalIndex(index), position);
}

QTabBar::SelectionBehavior ComboTabBar::selectionBehaviorOnRemove() const
{
    return m_mainTabBar->selectionBehaviorOnRemove();
}

void ComboTabBar::setSelectionBehaviorOnRemove(QTabBar::SelectionBehavior behavior)
{
    m_mainTabBar->setSelectionBehaviorOnRemove(behavior);
    m_pinnedTabBar->setSelectionBehaviorOnRemove(behavior);
}

bool ComboTabBar::expanding() const
{
    return m_mainTabBar->expanding();
}

void ComboTabBar::setExpanding(bool enabled)
{
    m_mainTabBar->setExpanding(enabled);
    m_pinnedTabBar->setExpanding(enabled);
}

bool ComboTabBar::isMovable() const
{
    return m_mainTabBar->isMovable();
}

void ComboTabBar::setMovable(bool movable)
{
    m_mainTabBar->setMovable(movable);
    m_pinnedTabBar->setMovable(movable);
}

bool ComboTabBar::documentMode() const
{
    return m_mainTabBar->documentMode();
}

void ComboTabBar::setDocumentMode(bool set)
{
    m_mainTabBar->setDocumentMode(set);
    m_pinnedTabBar->setDocumentMode(set);
}

int ComboTabBar::pinnedTabsCount() const
{
    return m_pinnedTabBar->count();
}

int ComboTabBar::normalTabsCount() const
{
    return m_mainTabBar->count();
}

bool ComboTabBar::isPinned(int index) const
{
    return index >= 0 && index < pinnedTabsCount();
}

void ComboTabBar::setFocusPolicy(Qt::FocusPolicy policy)
{
    QWidget::setFocusPolicy(policy);
    m_mainTabBar->setFocusPolicy(policy);
    m_pinnedTabBar->setFocusPolicy(policy);
}

void ComboTabBar::setObjectName(const QString &name)
{
    m_mainTabBar->setObjectName(name);
    m_pinnedTabBar->setObjectName(name);
}

void ComboTabBar::setMouseTracking(bool enable)
{
    m_mainTabBarWidget->scrollArea()->setMouseTracking(enable);
    m_mainTabBarWidget->setMouseTracking(enable);
    m_mainTabBar->setMouseTracking(enable);

    m_pinnedTabBarWidget->scrollArea()->setMouseTracking(enable);
    m_pinnedTabBarWidget->setMouseTracking(enable);
    m_pinnedTabBar->setMouseTracking(enable);

    QWidget::setMouseTracking(enable);
}

void ComboTabBar::setUpLayout()
{
    int height = qMax(m_mainTabBar->height(), m_pinnedTabBar->height());

    // Workaround for Oxygen theme. For some reason, QTabBar::height() returns bigger
    // height than it actually should.
    if (mApp->styleName() == QLatin1String("oxygen")) {
        height -= 4;
    }

    // We need to setup heights even before m_mainTabBar->height() has correct value
    // So lets just set minimum 5px height
    height = qMax(5, height);

    setFixedHeight(height);
    m_pinnedTabBar->setFixedHeight(height);
    m_leftContainer->setFixedHeight(height);
    m_rightContainer->setFixedHeight(height);
    m_mainTabBarWidget->setUpLayout();
    m_pinnedTabBarWidget->setUpLayout();

    setMinimumWidths();

    if (isVisible() && m_mainTabBar->count() > 0) {
        // ComboTabBar is now visible, we can sync heights of both tabbars
        m_pinnedTabBar->setFixedHeight(m_mainTabBar->sizeHint().height());
        m_mainTabBar->setFixedHeight(m_mainTabBar->sizeHint().height());
    }
}

void ComboTabBar::insertCloseButton(int index)
{
    index -= pinnedTabsCount();
    if (index < 0) {
        return;
    }

    QAbstractButton* closeButton = new CloseButton(this);
    closeButton->setFixedSize(closeButtonSize());
    closeButton->setToolTip(m_closeButtonsToolTip);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeTabFromButton()));
    m_mainTabBar->setTabButton(index, closeButtonPosition(), closeButton);
}

void ComboTabBar::setCloseButtonsToolTip(const QString &tip)
{
    m_closeButtonsToolTip = tip;
}

int ComboTabBar::mainTabBarWidth() const
{
    return m_mainTabBar->width();
}

int ComboTabBar::pinTabBarWidth() const
{
    return m_pinnedTabBarWidget->isHidden() ? 0 : m_pinnedTabBarWidget->width();
}

bool ComboTabBar::event(QEvent *event)
{
    const bool res = QWidget::event(event);

    switch (event->type()) {
    case QEvent::ToolTip:
        if (!isDragInProgress() && !isScrollInProgress()) {
            int index = tabAt(mapFromGlobal(QCursor::pos()));
            if (index >= 0)
                QToolTip::showText(QCursor::pos(), tabToolTip(index));
        }
        break;

    case QEvent::Resize:
        ensureVisible();
        break;

    case QEvent::Show:
        if (!event->spontaneous())
            QTimer::singleShot(0, this, &ComboTabBar::setUpLayout);
        break;

    case QEvent::Enter:
    case QEvent::Leave:
        // Make sure tabs are painted with correct mouseover state
        QTimer::singleShot(100, this, &ComboTabBar::updateTabBars);
        break;

    default:
        break;
    }

    return res;
}

void ComboTabBar::wheelEvent(QWheelEvent* event)
{
    event->accept();

    if (qzSettings->alwaysSwitchTabsWithWheel || (!m_mainTabBarWidget->isOverflowed() && !m_pinnedTabBarWidget->isOverflowed())) {
        m_wheelHelper.processEvent(event);
        while (WheelHelper::Direction direction = m_wheelHelper.takeDirection()) {
            switch (direction) {
            case WheelHelper::WheelUp:
            case WheelHelper::WheelLeft:
                setCurrentNextEnabledIndex(-1);
                break;

            case WheelHelper::WheelDown:
            case WheelHelper::WheelRight:
                setCurrentNextEnabledIndex(1);
                break;

            default:
                break;
            }
        }
        return;
    }

    if (m_mainTabBarWidget->underMouse()) {
        if (m_mainTabBarWidget->isOverflowed()) {
            m_mainTabBarWidget->scrollByWheel(event);
        }
        else if (m_pinnedTabBarWidget->isOverflowed()) {
            m_pinnedTabBarWidget->scrollByWheel(event);
        }
    }
    else if (m_pinnedTabBarWidget->underMouse()) {
        if (m_pinnedTabBarWidget->isOverflowed()) {
            m_pinnedTabBarWidget->scrollByWheel(event);
        }
        else if (m_mainTabBarWidget->isOverflowed()) {
            m_mainTabBarWidget->scrollByWheel(event);
        }
    }
}

bool ComboTabBar::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == m_mainTabBar && ev->type() == QEvent::Resize) {
        QResizeEvent* event = static_cast<QResizeEvent*>(ev);
        if (event->oldSize().height() != event->size().height()) {
            setUpLayout();
        }
    }

    // Handle wheel events exclusively in ComboTabBar
    if (ev->type() == QEvent::Wheel) {
        wheelEvent(static_cast<QWheelEvent*>(ev));
        return true;
    }

    return QWidget::eventFilter(obj, ev);
}

void ComboTabBar::paintEvent(QPaintEvent* ev)
{
    Q_UNUSED(ev);

    // This is needed to apply style sheets
    QStyleOption option;
    option.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &p, this);

#ifndef Q_OS_MACOS
    // Draw tabbar base even on parts of ComboTabBar that are not directly QTabBar
    QStyleOptionTabBarBase opt;
    TabBarHelper::initStyleBaseOption(&opt, m_mainTabBar, size());

    // Left container
    opt.rect.setX(m_leftContainer->x());
    opt.rect.setWidth(m_leftContainer->width());
    style()->drawPrimitive(QStyle::PE_FrameTabBarBase, &opt, &p);

    // Right container
    opt.rect.setX(m_rightContainer->x());
    opt.rect.setWidth(m_rightContainer->width());
    style()->drawPrimitive(QStyle::PE_FrameTabBarBase, &opt, &p);

    if (m_mainBarOverFlowed) {
        const int scrollButtonWidth = m_mainTabBarWidget->scrollButtonsWidth();

        // Left scroll button
        opt.rect.setX(m_mainTabBarWidget->x());
        opt.rect.setWidth(scrollButtonWidth);
        style()->drawPrimitive(QStyle::PE_FrameTabBarBase, &opt, &p);

        // Right scroll button
        opt.rect.setX(m_mainTabBarWidget->x() + m_mainTabBarWidget->width() - scrollButtonWidth);
        opt.rect.setWidth(scrollButtonWidth);
        style()->drawPrimitive(QStyle::PE_FrameTabBarBase, &opt, &p);
    }

    // Draw base even when main tabbar is empty
    if (normalTabsCount() == 0) {
        opt.rect.setX(m_mainTabBarWidget->x());
        opt.rect.setWidth(m_mainTabBarWidget->width());
        style()->drawPrimitive(QStyle::PE_FrameTabBarBase, &opt, &p);
    }
#endif
}

int ComboTabBar::comboTabBarPixelMetric(ComboTabBar::SizeType sizeType) const
{
    switch (sizeType) {
    case ExtraReservedWidth:
        return 0;

    case NormalTabMaximumWidth:
        return 150;

    case ActiveTabMinimumWidth:
    case NormalTabMinimumWidth:
    case OverflowedTabWidth:
        return 100;

    case PinnedTabWidth:
        return 30;

    default:
        break;
    }

    return -1;
}

QTabBar::ButtonPosition ComboTabBar::iconButtonPosition() const
{
    return (closeButtonPosition() == QTabBar::RightSide ? QTabBar::LeftSide : QTabBar::RightSide);
}

QTabBar::ButtonPosition ComboTabBar::closeButtonPosition() const
{
    return (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this);
}

QSize ComboTabBar::iconButtonSize() const
{
    QSize s = closeButtonSize();
    s.setWidth(qMax(16, s.width()));
    s.setHeight(qMax(16, s.height()));
    return s;
}

QSize ComboTabBar::closeButtonSize() const
{
    int width = style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, 0, this);
    int height = style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, 0, this);
    return QSize(width, height);
}

bool ComboTabBar::validIndex(int index) const
{
    return (index >= 0 && index < count());
}

void ComboTabBar::setCurrentNextEnabledIndex(int offset)
{
    for (int index = currentIndex() + offset; validIndex(index); index += offset) {
        if (isTabEnabled(index)) {
            setCurrentIndex(index);
            break;
        }
    }
}

bool ComboTabBar::usesScrollButtons() const
{
    return m_mainTabBarWidget->usesScrollButtons();
}

void ComboTabBar::setUsesScrollButtons(bool useButtons)
{
    m_mainTabBarWidget->setUsesScrollButtons(useButtons);
}

void ComboTabBar::showDropIndicator(int index, DropIndicatorPosition position)
{
    clearDropIndicator();
    localTabBar(index)->showDropIndicator(toLocalIndex(index), position);
}

void ComboTabBar::clearDropIndicator()
{
    m_mainTabBar->clearDropIndicator();
    m_pinnedTabBar->clearDropIndicator();
}

bool ComboTabBar::isDragInProgress() const
{
    return m_mainTabBar->isDragInProgress() || m_pinnedTabBar->isDragInProgress();
}

bool ComboTabBar::isScrollInProgress() const
{
    return m_mainTabBarWidget->scrollBar()->isScrolling() || m_pinnedTabBarWidget->scrollBar()->isScrolling();
}

bool ComboTabBar::isMainBarOverflowed() const
{
    return m_mainBarOverFlowed;
}

int ComboTabBar::cornerWidth(Qt::Corner corner) const
{
    if (corner == Qt::TopLeftCorner) {
        return m_leftContainer->width();
    }
    else if (corner == Qt::TopRightCorner) {
        return m_rightContainer->width();
    }

    qFatal("ComboTabBar::cornerWidth Only TopLeft and TopRight corners are implemented!");
    return -1;
}

void ComboTabBar::addCornerWidget(QWidget* widget, Qt::Corner corner)
{
    if (corner == Qt::TopLeftCorner) {
        m_leftLayout->addWidget(widget);
    }
    else if (corner == Qt::TopRightCorner) {
        m_rightLayout->addWidget(widget);
    }
    else {
        qFatal("ComboTabBar::addCornerWidget Only TopLeft and TopRight corners are implemented!");
    }
}

// static
int ComboTabBar::slideAnimationDuration()
{
    // taken from qtabbar_p.h
    return 250;
}

void ComboTabBar::ensureVisible(int index, int xmargin)
{
    if (index == -1) {
        index = currentIndex();
    }

    if (index < pinnedTabsCount()) {
        if (xmargin == -1) {
            xmargin = qMax(20, comboTabBarPixelMetric(PinnedTabWidth));
        }
        m_pinnedTabBarWidget->ensureVisible(index, xmargin);
    }
    else {
        if (xmargin == -1) {
            xmargin = comboTabBarPixelMetric(OverflowedTabWidth);
        }
        index -= pinnedTabsCount();
        m_mainTabBarWidget->ensureVisible(index, xmargin);
    }
}

QSize ComboTabBar::tabSizeHint(int index, bool fast) const
{
    Q_UNUSED(fast)

    return localTabBar(index)->baseClassTabSizeHint(toLocalIndex(index));
}

void ComboTabBar::tabInserted(int index)
{
    Q_UNUSED(index)
}

void ComboTabBar::tabRemoved(int index)
{
    Q_UNUSED(index)
}

TabBarHelper* ComboTabBar::mainTabBar() const
{
    return m_mainTabBar;
}

TabBarHelper* ComboTabBar::localTabBar(int index) const
{
    if (index < 0 || index >= pinnedTabsCount()) {
        return m_mainTabBar;
    }
    else {
        return m_pinnedTabBar;
    }
}

int ComboTabBar::toLocalIndex(int globalIndex) const
{
    if (globalIndex < 0) {
        return -1;
    }

    if (globalIndex >= pinnedTabsCount()) {
        return globalIndex - pinnedTabsCount();
    }
    else {
        return globalIndex;
    }
}

QRect ComboTabBar::mapFromLocalTabRect(const QRect &rect, QWidget *tabBar) const
{
    if (!rect.isValid()) {
        return rect;
    }

    QRect r = rect;

    if (tabBar == m_mainTabBar) {
        r.moveLeft(r.x() + mapFromGlobal(m_mainTabBar->mapToGlobal(QPoint(0, 0))).x());
        QRect widgetRect = m_mainTabBarWidget->scrollArea()->viewport()->rect();
        widgetRect.moveLeft(widgetRect.x() + mapFromGlobal(m_mainTabBarWidget->scrollArea()->viewport()->mapToGlobal(QPoint(0, 0))).x());
        r = r.intersected(widgetRect);
    } else {
        r.moveLeft(r.x() + mapFromGlobal(m_pinnedTabBar->mapToGlobal(QPoint(0, 0))).x());
        QRect widgetRect = m_pinnedTabBarWidget->scrollArea()->viewport()->rect();
        widgetRect.moveLeft(widgetRect.x() + mapFromGlobal(m_pinnedTabBarWidget->scrollArea()->viewport()->mapToGlobal(QPoint(0, 0))).x());
        r = r.intersected(widgetRect);
    }

    return r;
}

void ComboTabBar::updatePinnedTabBarVisibility()
{
    m_pinnedTabBarWidget->setVisible(pinnedTabsCount() > 0);
}

void ComboTabBar::setMinimumWidths()
{
    if (!isVisible() || comboTabBarPixelMetric(PinnedTabWidth) < 0) {
        return;
    }

    const int tabBarsSpacing = 3; // To distinguish tabbars
    int pinnedTabBarWidth = pinnedTabsCount() * comboTabBarPixelMetric(PinnedTabWidth);
    m_pinnedTabBar->setMinimumWidth(pinnedTabBarWidth);
    m_pinnedTabBarWidget->setFixedWidth(pinnedTabBarWidth + tabBarsSpacing);

    // Width that is needed by main tabbar
    int mainTabBarWidth = comboTabBarPixelMetric(NormalTabMinimumWidth) * (m_mainTabBar->count() - 1) +
                          comboTabBarPixelMetric(ActiveTabMinimumWidth) +
                          comboTabBarPixelMetric(ExtraReservedWidth);

    // This is the full width that would be needed for the tabbar (including pinned tabbar and corner widgets)
    int realTabBarWidth = mainTabBarWidth + m_pinnedTabBarWidget->width() +
                          cornerWidth(Qt::TopLeftCorner) +
                          cornerWidth(Qt::TopRightCorner);

    // Does it fit in our widget?
    if (realTabBarWidth <= width()) {
        if (m_mainBarOverFlowed) {
            m_mainBarOverFlowed = false;
            QTimer::singleShot(0, this, SLOT(emitOverFlowChanged()));
        }

        m_mainTabBar->useFastTabSizeHint(false);
        m_mainTabBar->setMinimumWidth(mainTabBarWidth);
    }
    else {
        if (!m_mainBarOverFlowed) {
            m_mainBarOverFlowed = true;
            QTimer::singleShot(0, this, SLOT(emitOverFlowChanged()));
        }

        // All tabs have now same width, we can use fast tabSizeHint
        m_mainTabBar->useFastTabSizeHint(true);
        m_mainTabBar->setMinimumWidth(m_mainTabBar->count() * comboTabBarPixelMetric(OverflowedTabWidth));
    }
}


TabBarHelper::TabBarHelper(bool isPinnedTabBar, ComboTabBar* comboTabBar)
    : QTabBar(comboTabBar)
    , m_comboTabBar(comboTabBar)
    , m_scrollArea(0)
    , m_pressedIndex(-1)
    , m_dragInProgress(false)
    , m_activeTabBar(false)
    , m_isPinnedTabBar(isPinnedTabBar)
    , m_useFastTabSizeHint(false)
{
}

int TabBarHelper::tabPadding() const
{
    return m_tabPadding;
}

void TabBarHelper::setTabPadding(int padding)
{
    m_tabPadding = padding;
}

QColor TabBarHelper::baseColor() const
{
    return m_baseColor;
}

void TabBarHelper::setBaseColor(const QColor &color)
{
    m_baseColor = color;
}

void TabBarHelper::setTabButton(int index, QTabBar::ButtonPosition position, QWidget* widget)
{
    QTabBar::setTabButton(index, position, widget);
}

QSize TabBarHelper::tabSizeHint(int index) const
{
    if (this == m_comboTabBar->mainTabBar()) {
        index += m_comboTabBar->pinnedTabsCount();
    }
    return m_comboTabBar->tabSizeHint(index, m_useFastTabSizeHint);
}

QSize TabBarHelper::baseClassTabSizeHint(int index) const
{
    return QTabBar::tabSizeHint(index);
}

QRect TabBarHelper::draggedTabRect() const
{
    if (!m_dragInProgress) {
        return QRect();
    }

    QStyleOptionTab tab;
    initStyleOption(&tab, m_pressedIndex);

    const int tabDragOffset = dragOffset(&tab, m_pressedIndex);
    if (tabDragOffset != 0) {
        tab.rect.moveLeft(tab.rect.x() + tabDragOffset);
    }
    return tab.rect;
}

QPixmap TabBarHelper::tabPixmap(int index) const
{
    QStyleOptionTab tab;
    initStyleOption(&tab, index);

    tab.state &= ~QStyle::State_MouseOver;
    tab.position = QStyleOptionTab::OnlyOneTab;
    tab.leftButtonSize = QSize();
    tab.rightButtonSize = QSize();

    QWidget *iconButton = tabButton(index, m_comboTabBar->iconButtonPosition());
    QWidget *closeButton = tabButton(index, m_comboTabBar->closeButtonPosition());

    if (iconButton) {
        const QPixmap pix = iconButton->grab();
        if (!pix.isNull()) {
            tab.icon = pix;
            tab.iconSize = pix.size() / pix.devicePixelRatioF();
        }
    }

    if (closeButton) {
        const int width = tab.fontMetrics.width(tab.text) + closeButton->width();
        tab.text = tab.fontMetrics.elidedText(tabText(index), Qt::ElideRight, width);
    }

    QPixmap out(tab.rect.size() * devicePixelRatioF());
    out.setDevicePixelRatio(devicePixelRatioF());
    out.fill(Qt::transparent);
    tab.rect = QRect(QPoint(0, 0), tab.rect.size());

    QPainter p(&out);
    style()->drawControl(QStyle::CE_TabBarTab, &tab, &p, this);
    p.end();

    return out;
}

bool TabBarHelper::isActiveTabBar()
{
    return m_activeTabBar;
}

void TabBarHelper::setActiveTabBar(bool activate)
{
    if (m_activeTabBar != activate) {
        m_activeTabBar = activate;

        // If the last tab in a tabbar is closed, the selection jumps to the other
        // tabbar. The stacked widget automatically selects the next tab, which is
        // either the last tab in pinned tabbar or the first one in main tabbar.

        if (!m_activeTabBar) {
            m_comboTabBar->m_blockCurrentChangedSignal = true;
            setCurrentIndex(m_isPinnedTabBar ? count() - 1 : 0);
            m_comboTabBar->m_blockCurrentChangedSignal = false;
        }

        update();
    }
}

void TabBarHelper::removeTab(int index)
{
    // Removing tab in inactive tabbar will change current index and thus
    // changing active tabbar, which is really not wanted.
    // Also removing tab will cause a duplicate call to ComboTabBar::slotCurrentChanged()
    m_comboTabBar->m_blockCurrentChangedSignal = true;

    QTabBar::removeTab(index);

    m_comboTabBar->m_blockCurrentChangedSignal = false;
}

void TabBarHelper::setScrollArea(QScrollArea* scrollArea)
{
    m_scrollArea = scrollArea;
}

void TabBarHelper::useFastTabSizeHint(bool enabled)
{
    m_useFastTabSizeHint = enabled;
}

void TabBarHelper::showDropIndicator(int index, ComboTabBar::DropIndicatorPosition position)
{
    m_dropIndicatorIndex = index;
    m_dropIndicatorPosition = position;
    update();
}

void TabBarHelper::clearDropIndicator()
{
    m_dropIndicatorIndex = -1;
    update();
}

bool TabBarHelper::isDisplayedOnViewPort(int globalLeft, int globalRight)
{
    bool isVisible = true;

    if (m_scrollArea) {
        if (globalRight < m_scrollArea->viewport()->mapToGlobal(QPoint(0, 0)).x() ||
            globalLeft > m_scrollArea->viewport()->mapToGlobal(m_scrollArea->viewport()->rect().topRight()).x()
           ) {
            isVisible = false;
        }
    }

    return isVisible;
}

bool TabBarHelper::isDragInProgress() const
{
    return m_dragInProgress;
}

void TabBarHelper::setCurrentIndex(int index)
{
    if (index == currentIndex() && !m_activeTabBar) {
        emit currentChanged(currentIndex());
    }

    QTabBar::setCurrentIndex(index);
}

bool TabBarHelper::event(QEvent* ev)
{
    switch (ev->type()) {
    case QEvent::ToolTip:
        ev->ignore();
        return false;

    default:
        break;
    }

    QTabBar::event(ev);
    ev->ignore();
    return false;
}

// Hack to get dragOffset from QTabBar internals
int TabBarHelper::dragOffset(QStyleOptionTab *option, int tabIndex) const
{
    QRect rect;
    QWidget *button = tabButton(tabIndex, QTabBar::LeftSide);
    if (button) {
        rect = style()->subElementRect(QStyle::SE_TabBarTabLeftButton, option, this);
    }
    if (!rect.isValid()) {
        button = tabButton(tabIndex, QTabBar::RightSide);
        rect = style()->subElementRect(QStyle::SE_TabBarTabRightButton, option, this);
    }
    if (!button || !rect.isValid()) {
        return 0;
    }
    return button->pos().x() - rect.topLeft().x();
}

// Taken from qtabbar.cpp
void TabBarHelper::initStyleBaseOption(QStyleOptionTabBarBase *optTabBase, QTabBar* tabbar, QSize size)
{
    QStyleOptionTab tabOverlap;
    tabOverlap.shape = tabbar->shape();
    int overlap = tabbar->style()->pixelMetric(QStyle::PM_TabBarBaseOverlap, &tabOverlap, tabbar);
    QWidget* theParent = tabbar->parentWidget();
    optTabBase->init(tabbar);
    optTabBase->shape = tabbar->shape();
    optTabBase->documentMode = tabbar->documentMode();
    if (theParent && overlap > 0) {
        QRect rect;
        switch (tabOverlap.shape) {
        case QTabBar::RoundedNorth:
        case QTabBar::TriangularNorth:
            rect.setRect(0, size.height() - overlap, size.width(), overlap);
            break;
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth:
            rect.setRect(0, 0, size.width(), overlap);
            break;
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
            rect.setRect(0, 0, overlap, size.height());
            break;
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
            rect.setRect(size.width() - overlap, 0, overlap, size.height());
            break;
        }
        optTabBase->rect = rect;
    }
}

// Adapted from qtabbar.cpp
// Note: doesn't support vertical tabs
void TabBarHelper::paintEvent(QPaintEvent *)
{
    QStyleOptionTabBarBase optTabBase;
    initStyleBaseOption(&optTabBase, this, size());

    QStylePainter p(this);
    int selected = currentIndex();

    for (int i = 0; i < count(); ++i) {
        optTabBase.tabBarRect |= tabRect(i);
    }

    if (m_activeTabBar) {
        optTabBase.selectedTabRect = tabRect(selected);
    }

    if (drawBase()) {
        p.drawPrimitive(QStyle::PE_FrameTabBarBase, optTabBase);
    }

    const QPoint cursorPos = QCursor::pos();
    int indexUnderMouse = isDisplayedOnViewPort(cursorPos.x(), cursorPos.x()) ? tabAt(mapFromGlobal(cursorPos)) : -1;

    for (int i = 0; i < count(); ++i) {
        if (i == selected) {
            continue;
        }

        QStyleOptionTab tab;
        initStyleOption(&tab, i);

        const int tabDragOffset = dragOffset(&tab, i);
        if (tabDragOffset != 0) {
            tab.rect.moveLeft(tab.rect.x() + tabDragOffset);
        }

        // Don't bother drawing a tab if the entire tab is outside of the visible tab bar.
        if (!isDisplayedOnViewPort(mapToGlobal(tab.rect.topLeft()).x(), mapToGlobal(tab.rect.topRight()).x())) {
            continue;
        }

        if (!m_activeTabBar) {
            tab.selectedPosition = QStyleOptionTab::NotAdjacent;
        }

        if (!(tab.state & QStyle::State_Enabled)) {
            tab.palette.setCurrentColorGroup(QPalette::Disabled);
        }

        // Update mouseover state when scrolling
        if (!m_dragInProgress && i == indexUnderMouse) {
            tab.state |= QStyle::State_MouseOver;
        } else {
            tab.state &= ~QStyle::State_MouseOver;
        }

        p.drawControl(QStyle::CE_TabBarTab, tab);
    }

    // Draw the selected tab last to get it "on top"
    if (selected >= 0) {
        QStyleOptionTab tab;
        initStyleOption(&tab, selected);

        const int tabDragOffset = dragOffset(&tab, selected);
        if (tabDragOffset != 0) {
            tab.rect.moveLeft(tab.rect.x() + tabDragOffset);
        }

        // Update mouseover state when scrolling
        if (selected == indexUnderMouse) {
            tab.state |= QStyle::State_MouseOver;
        } else {
            tab.state &= ~QStyle::State_MouseOver;
        }

        if (!m_activeTabBar) {
            // If this is inactive tab, we still need to draw selected tab outside the tabbar
            // Some themes (eg. Oxygen) draws line under tabs with selected tab
            // Let's just move it outside rect(), it appears to work
            QStyleOptionTab tb = tab;
            tb.rect.moveRight((rect().x() + rect().width()) * 2);
            p.drawControl(QStyle::CE_TabBarTab, tb);

            // Draw the tab without selected state
            tab.state = tab.state & ~QStyle::State_Selected;
        }

        if (!m_movingTab || !m_movingTab->isVisible()) {
            p.drawControl(QStyle::CE_TabBarTab, tab);
        } else {
            int taboverlap = style()->pixelMetric(QStyle::PM_TabBarTabOverlap, nullptr, this);
            m_movingTab->setGeometry(tab.rect.adjusted(-taboverlap, 0, taboverlap, 0));

            QRect grabRect = tabRect(selected);
            grabRect.adjust(-taboverlap, 0, taboverlap, 0);
            QPixmap grabImage(grabRect.size() * devicePixelRatioF());
            grabImage.setDevicePixelRatio(devicePixelRatioF());
            grabImage.fill(Qt::transparent);
            QStylePainter p(&grabImage, this);
            p.initFrom(this);
            if (tabDragOffset != 0) {
                tab.position = QStyleOptionTab::OnlyOneTab;
            }
            tab.rect.moveTopLeft(QPoint(taboverlap, 0));
            p.drawControl(QStyle::CE_TabBarTab, tab);
            m_movingTab->m_pixmap = grabImage;
            m_movingTab->update();
        }
    }

    // Draw drop indicator
    if (m_dropIndicatorIndex != -1) {
        const QRect tr = tabRect(m_dropIndicatorIndex);
        QRect r;
        if (m_dropIndicatorPosition == ComboTabBar::BeforeTab) {
            r = QRect(qMax(0, tr.left() - 1), tr.top(), 3, tr.height());
        } else {
            const int rightOffset = m_dropIndicatorIndex == count() - 1 ? -2 : 0;
            r = QRect(tr.right() + rightOffset, tr.top(), 3, tr.height());
        }
        // Modified code from KFilePlacesView
        QColor color = palette().brush(QPalette::Normal, QPalette::Highlight).color();
        const int x = (r.left() + r.right()) / 2;
        const int thickness = qRound(r.width() / 2.0);
        int alpha = 255;
        const int alphaDec = alpha / (thickness + 1);
        for (int i = 0; i < thickness; i++) {
            color.setAlpha(alpha);
            alpha -= alphaDec;
            p.setPen(color);
            p.drawLine(x - i, r.top(), x - i, r.bottom());
            p.drawLine(x + i, r.top(), x + i, r.bottom());
        }
    }
}

void TabBarHelper::mousePressEvent(QMouseEvent* event)
{
    event->ignore();
    if (event->buttons() == Qt::LeftButton) {
        m_pressedIndex = tabAt(event->pos());
        if (m_pressedIndex != -1) {
            m_dragStartPosition = event->pos();
            // virtualize selecting tab by click
            if (m_pressedIndex == currentIndex() && !m_activeTabBar) {
                emit currentChanged(currentIndex());
            }
        }
    }

    QTabBar::mousePressEvent(event);
}

void TabBarHelper::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragInProgress && m_pressedIndex != -1) {
        if ((event->pos() - m_dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
            m_dragInProgress = true;
        }
    }

    QTabBar::mouseMoveEvent(event);

    // Hack to find QMovableTabWidget
    if (m_dragInProgress && !m_movingTab) {
        const auto objects = children();
        const int taboverlap = style()->pixelMetric(QStyle::PM_TabBarTabOverlap, nullptr, this);
        QRect grabRect = tabRect(currentIndex());
        grabRect.adjust(-taboverlap, 0, taboverlap, 0);
        for (QObject *object : objects) {
            QWidget *widget = qobject_cast<QWidget*>(object);
            if (widget && widget->geometry() == grabRect) {
                m_movingTab = static_cast<QMovableTabWidget*>(widget);
                break;
            }
        }
    }

    // Don't allow to move tabs outside of tabbar
    if (m_dragInProgress && m_movingTab) {
        QRect r = tabRect(m_pressedIndex);
        r.moveLeft(r.x() + (event->pos().x() - m_dragStartPosition.x()));
        bool sendEvent = false;
        int diff = r.topRight().x() - tabRect(count() - 1).topRight().x();
        if (diff > 0) {
            sendEvent = true;
        } else {
            diff = r.topLeft().x() - tabRect(0).topLeft().x();
            if (diff < 0) {
                sendEvent = true;
            }
        }
        if (sendEvent) {
            QPoint pos = event->pos();
            pos.setX(pos.x() - diff);
            QMouseEvent ev(event->type(), pos, event->button(), event->buttons(), event->modifiers());
            QTabBar::mouseMoveEvent(&ev);
        }
    }
}

void TabBarHelper::mouseReleaseEvent(QMouseEvent* event)
{
    event->ignore();

    if (event->button() == Qt::LeftButton) {
        m_pressedIndex = -1;
        m_dragInProgress = false;
        m_dragStartPosition = QPoint();
    }

    QTabBar::mouseReleaseEvent(event);

    update();
}

void TabBarHelper::initStyleOption(QStyleOptionTab* option, int tabIndex) const
{
    QTabBar::initStyleOption(option, tabIndex);

    // Workaround zero padding when tabs are styled using style sheets
    if (m_tabPadding) {
        const QRect textRect = style()->subElementRect(QStyle::SE_TabBarTabText, option, this);
        const int width = textRect.width() - 2 * m_tabPadding;
        option->text = option->fontMetrics.elidedText(tabText(tabIndex), elideMode(), width, Qt::TextShowMnemonic);
    }

    // Bespin doesn't highlight current tab when there is only one tab in tabbar
    static int isBespin = -1;

    if (isBespin == -1)
        isBespin = mApp->styleName() == QL1S("bespin");

    if (!isBespin)
        return;

    int index = m_isPinnedTabBar ? tabIndex : m_comboTabBar->pinnedTabsCount() + tabIndex;

    if (m_comboTabBar->count() > 1) {
        if (index == 0)
            option->position = QStyleOptionTab::Beginning;
        else if (index == m_comboTabBar->count() - 1)
            option->position = QStyleOptionTab::End;
        else
            option->position = QStyleOptionTab::Middle;
    }
    else {
        option->position = QStyleOptionTab::OnlyOneTab;
    }
}


TabScrollBar::TabScrollBar(QWidget* parent)
    : QScrollBar(Qt::Horizontal, parent)
{
    m_animation = new QPropertyAnimation(this, "value", this);
}

TabScrollBar::~TabScrollBar()
{
}

bool TabScrollBar::isScrolling() const
{
    return m_animation->state() == QPropertyAnimation::Running;
}

void TabScrollBar::animateToValue(int to, QEasingCurve::Type type)
{
    to = qBound(minimum(), to, maximum());
    int lenght = qAbs(to - value());
    int duration = qMin(1500, 200 + lenght / 2);

    m_animation->stop();
    m_animation->setEasingCurve(type);
    m_animation->setDuration(duration);
    m_animation->setStartValue(value());
    m_animation->setEndValue(to);
    m_animation->start();
}


TabBarScrollWidget::TabBarScrollWidget(QTabBar* tabBar, QWidget* parent)
    : QWidget(parent)
    , m_tabBar(tabBar)
    , m_usesScrollButtons(false)
    , m_totalDeltas(0)
{
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFocusPolicy(Qt::NoFocus);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_scrollBar = new TabScrollBar(m_scrollArea);
    m_scrollArea->setHorizontalScrollBar(m_scrollBar);
    m_scrollArea->setWidget(m_tabBar);

    m_leftScrollButton = new ToolButton(this);
    m_leftScrollButton->setFocusPolicy(Qt::NoFocus);
    m_leftScrollButton->setAutoRaise(true);
    m_leftScrollButton->setObjectName("tabbar-button-left");
    m_leftScrollButton->setAutoRepeat(true);
    m_leftScrollButton->setAutoRepeatDelay(200);
    m_leftScrollButton->setAutoRepeatInterval(200);
    connect(m_leftScrollButton, SIGNAL(pressed()), this, SLOT(scrollStart()));
    connect(m_leftScrollButton, SIGNAL(doubleClicked()), this, SLOT(scrollToLeftEdge()));
    connect(m_leftScrollButton, SIGNAL(middleMouseClicked()), this, SLOT(ensureVisible()));

    m_rightScrollButton = new ToolButton(this);
    m_rightScrollButton->setFocusPolicy(Qt::NoFocus);
    m_rightScrollButton->setAutoRaise(true);
    m_rightScrollButton->setObjectName("tabbar-button-right");
    m_rightScrollButton->setAutoRepeat(true);
    m_rightScrollButton->setAutoRepeatDelay(200);
    m_rightScrollButton->setAutoRepeatInterval(200);
    connect(m_rightScrollButton, SIGNAL(pressed()), this, SLOT(scrollStart()));
    connect(m_rightScrollButton, SIGNAL(doubleClicked()), this, SLOT(scrollToRightEdge()));
    connect(m_rightScrollButton, SIGNAL(middleMouseClicked()), this, SLOT(ensureVisible()));

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(m_leftScrollButton);
    hLayout->addWidget(m_scrollArea);
    hLayout->addWidget(m_rightScrollButton);
    setLayout(hLayout);

    m_scrollArea->viewport()->setAutoFillBackground(false);
    connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(updateScrollButtonsState()));

    updateScrollButtonsState();
    overFlowChanged(false);
}

QTabBar* TabBarScrollWidget::tabBar()
{
    return m_tabBar;
}

QScrollArea* TabBarScrollWidget::scrollArea()
{
    return m_scrollArea;
}

TabScrollBar* TabBarScrollWidget::scrollBar()
{
    return m_scrollBar;
}

void TabBarScrollWidget::ensureVisible(int index, int xmargin)
{
    if (index == -1) {
        index = m_tabBar->currentIndex();
    }

    if (index < 0 || index >= m_tabBar->count()) {
        return;
    }
    xmargin = qMin(xmargin, m_scrollArea->viewport()->width() / 2);

    // Qt Bug? the following lines were taken from QScrollArea::ensureVisible() and
    // then were fixed. The original version caculates wrong values in RTL layouts.
    const QRect logicalTabRect = QStyle::visualRect(m_tabBar->layoutDirection(), m_tabBar->rect(), m_tabBar->tabRect(index));
    int logicalX = QStyle::visualPos(Qt::LeftToRight, m_scrollArea->viewport()->rect(), logicalTabRect.center()).x();

    if (logicalX - xmargin < m_scrollBar->value()) {
        m_scrollBar->animateToValue(qMax(0, logicalX - xmargin));
    }
    else if (logicalX > m_scrollBar->value() + m_scrollArea->viewport()->width() - xmargin) {
        m_scrollBar->animateToValue(qMin(logicalX - m_scrollArea->viewport()->width() + xmargin,
                                         m_scrollBar->maximum()));
    }
}

void TabBarScrollWidget::scrollToLeft(int n, QEasingCurve::Type type)
{
    n = qMax(1, n);
    m_scrollBar->animateToValue(m_scrollBar->value() - n * m_scrollBar->singleStep(), type);
}

void TabBarScrollWidget::scrollToRight(int n, QEasingCurve::Type type)
{
    n = qMax(1, n);
    m_scrollBar->animateToValue(m_scrollBar->value() + n * m_scrollBar->singleStep(), type);
}

void TabBarScrollWidget::scrollToLeftEdge()
{
    m_scrollBar->animateToValue(m_scrollBar->minimum());
}

void TabBarScrollWidget::scrollToRightEdge()
{
    m_scrollBar->animateToValue(m_scrollBar->maximum());
}

void TabBarScrollWidget::setUpLayout()
{
    const int height = m_tabBar->height();

    setFixedHeight(height);
}

void TabBarScrollWidget::updateScrollButtonsState()
{
    m_leftScrollButton->setEnabled(m_scrollBar->value() != m_scrollBar->minimum());
    m_rightScrollButton->setEnabled(m_scrollBar->value() != m_scrollBar->maximum());
}

void TabBarScrollWidget::overFlowChanged(bool overflowed)
{
    bool showScrollButtons = overflowed && m_usesScrollButtons;

    m_leftScrollButton->setVisible(showScrollButtons);
    m_rightScrollButton->setVisible(showScrollButtons);
}

void TabBarScrollWidget::scrollStart()
{
    bool ctrlModifier = QApplication::keyboardModifiers() & Qt::ControlModifier;

    if (sender() == m_leftScrollButton) {
        if (ctrlModifier) {
            scrollToLeftEdge();
        }
        else {
            scrollToLeft(5, QEasingCurve::Linear);
        }
    }
    else if (sender() == m_rightScrollButton) {
        if (ctrlModifier) {
            scrollToRightEdge();
        }
        else {
            scrollToRight(5, QEasingCurve::Linear);
        }
    }
}

void TabBarScrollWidget::scrollByWheel(QWheelEvent* event)
{
    event->accept();

    // Check if direction has changed from last time
    if (m_totalDeltas * event->delta() < 0) {
        m_totalDeltas = 0;
    }

    m_totalDeltas += event->delta();

    // Slower scrolling for horizontal wheel scrolling
    if (event->orientation() == Qt::Horizontal) {
        if (event->delta() > 0) {
            scrollToLeft();
        }
        else if (event->delta() < 0) {
            scrollToRight();
        }
        return;
    }

    // Faster scrolling with control modifier
    if (event->orientation() == Qt::Vertical && event->modifiers() == Qt::ControlModifier) {
        if (event->delta() > 0) {
            scrollToLeft(10);
        }
        else if (event->delta() < 0) {
            scrollToRight(10);
        }
        return;
    }

    // Fast scrolling with just wheel scroll
    int factor = qMax(qRound(m_scrollBar->pageStep() / 1.5), m_scrollBar->singleStep());
    if ((event->modifiers() & Qt::ControlModifier) || (event->modifiers() & Qt::ShiftModifier)) {
        factor = m_scrollBar->pageStep();
    }

    int offset = (m_totalDeltas / 120) * factor;
    if (offset != 0) {
        if (isRightToLeft()) {
            m_scrollBar->animateToValue(m_scrollBar->value() + offset);
        }
        else {
            m_scrollBar->animateToValue(m_scrollBar->value() - offset);
        }

        m_totalDeltas -= (offset / factor) * 120;
    }
}

int TabBarScrollWidget::scrollButtonsWidth() const
{
    // Assumes both buttons have the same width
    return m_leftScrollButton->width();
}

bool TabBarScrollWidget::usesScrollButtons() const
{
    return m_usesScrollButtons;
}

void TabBarScrollWidget::setUsesScrollButtons(bool useButtons)
{
    if (useButtons != m_usesScrollButtons) {
        m_usesScrollButtons = useButtons;
        updateScrollButtonsState();
        m_tabBar->setElideMode(m_tabBar->elideMode());
    }
}

bool TabBarScrollWidget::isOverflowed() const
{
    return m_tabBar->count() > 0 && m_scrollBar->minimum() != m_scrollBar->maximum();
}

int TabBarScrollWidget::tabAt(const QPoint &pos) const
{
    if (m_leftScrollButton->isVisible() && (m_leftScrollButton->rect().contains(pos) ||
                                            m_rightScrollButton->rect().contains(pos))) {
        return -1;
    }

    return m_tabBar->tabAt(m_tabBar->mapFromGlobal(mapToGlobal(pos)));
}

void TabBarScrollWidget::mouseMoveEvent(QMouseEvent* event)
{
    event->ignore();
}

void TabBarScrollWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    updateScrollButtonsState();
}


CloseButton::CloseButton(QWidget* parent)
    : QAbstractButton(parent)
{
    setObjectName("combotabbar_tabs_close_button");
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::ArrowCursor);
}

void CloseButton::enterEvent(QEvent* event)
{
    if (isEnabled()) {
        update();
    }

    QAbstractButton::enterEvent(event);
}

void CloseButton::leaveEvent(QEvent* event)
{
    if (isEnabled()) {
        update();
    }

    QAbstractButton::leaveEvent(event);
}

void CloseButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QStyleOption opt;
    opt.init(this);
    opt.state |= QStyle::State_AutoRaise;

    // update raised state on scrolling
    bool isUnderMouse = rect().contains(mapFromGlobal(QCursor::pos()));

    if (isEnabled() && isUnderMouse && !isChecked() && !isDown()) {
        opt.state |= QStyle::State_Raised;
    }
    if (isChecked()) {
        opt.state |= QStyle::State_On;
    }
    if (isDown()) {
        opt.state |= QStyle::State_Sunken;
    }

    if (TabBarHelper* tb = qobject_cast<TabBarHelper*>(parent())) {
        int index = tb->currentIndex();
        QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, tb);
        if (tb->tabButton(index, closeSide) == this && tb->isActiveTabBar()) {
            opt.state |= QStyle::State_Selected;
        }
    }

    style()->drawPrimitive(QStyle::PE_IndicatorTabClose, &opt, &p, this);
}
