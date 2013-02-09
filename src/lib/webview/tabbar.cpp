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
#include "tabbar.h"
#include "tabwidget.h"
#include "tabpreview.h"
#include "qupzilla.h"
#include "webtab.h"
#include "iconprovider.h"
#include "toolbutton.h"
#include "settings.h"
#include "tabbedwebview.h"
#include "mainapplication.h"
#include "pluginproxy.h"

#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QStyleOption>
#include <QApplication>
#include <QTimer>
#include <QRect>

#define MAXIMUM_TAB_WIDTH 250
#define MINIMUM_TAB_WIDTH 125

TabBar::TabBar(QupZilla* mainClass, TabWidget* tabWidget)
    : QTabBar()
    , p_QupZilla(mainClass)
    , m_tabWidget(tabWidget)
    , m_tabPreview(new TabPreview(mainClass, tabWidget))
    , m_showTabPreviews(false)
    , m_clickedTab(0)
    , m_pinnedTabsCount(0)
    , m_normalTabWidth(0)
    , m_activeTabWidth(0)
{
    setObjectName("tabbar");
    setContextMenuPolicy(Qt::CustomContextMenu);
    setElideMode(Qt::ElideRight);
    setDocumentMode(true);
    setFocusPolicy(Qt::NoFocus);
    setTabsClosable(true);
    setMouseTracking(true);
    setMovable(true);

    setAcceptDrops(true);

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));
    connect(m_tabWidget, SIGNAL(pinnedTabClosed()), this, SLOT(pinnedTabClosed()));
    connect(m_tabWidget, SIGNAL(pinnedTabAdded()), this, SLOT(pinnedTabAdded()));

    m_tabPreviewTimer = new QTimer(this);
    m_tabPreviewTimer->setInterval(200);
    m_tabPreviewTimer->setSingleShot(true);
    connect(m_tabPreviewTimer, SIGNAL(timeout()), m_tabPreview, SLOT(hideAnimated()));
}

void TabBar::loadSettings()
{
    Settings settings;
    settings.beginGroup("Browser-Tabs-Settings");

    m_tabPreview->setAnimationsEnabled(settings.value("tabPreviewAnimationsEnabled", true).toBool());
    m_showTabPreviews = settings.value("showTabPreviews", true).toBool();
    bool activateLastTab = settings.value("ActivateLastTabWhenClosingActual", false).toBool();

    setSelectionBehaviorOnRemove(activateLastTab ? QTabBar::SelectPreviousTab : QTabBar::SelectRightTab);

    settings.endGroup();
}

void TabBar::updateVisibilityWithFullscreen(bool visible)
{
    // It is needed to save original geometry, otherwise
    // tabbar will get 3px height in fullscreen once it was hidden
    QTabBar::setVisible(visible);

    if (visible) {
        setGeometry(m_originalGeometry);
        emit showButtons();
    }
    else {
        m_originalGeometry = geometry();
        emit hideButtons();
    }
}

void TabBar::setVisible(bool visible)
{
    if (visible) {
        if (p_QupZilla->isFullScreen()) {
            return;
        }

        emit showButtons();
    }
    else {
        m_originalGeometry = geometry();
        emit hideButtons();
    }

    hideTabPreview(false);

    QTabBar::setVisible(visible);
}

void TabBar::contextMenuRequested(const QPoint &position)
{
    int index = tabAt(position);
    m_clickedTab = index;

    QMenu menu;
    menu.addAction(QIcon(":/icons/menu/new-tab.png"), tr("&New tab"), p_QupZilla, SLOT(addTab()));
    menu.addSeparator();
    if (index != -1) {
        WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_clickedTab));
        if (!webTab) {
            return;
        }
        if (p_QupZilla->weView(m_clickedTab)->isLoading()) {
            menu.addAction(qIconProvider->standardIcon(QStyle::SP_BrowserStop), tr("&Stop Tab"), this, SLOT(stopTab()));
        }
        else {
            menu.addAction(qIconProvider->standardIcon(QStyle::SP_BrowserReload), tr("&Reload Tab"), this, SLOT(reloadTab()));
        }

        menu.addAction(tr("&Duplicate Tab"), this, SLOT(duplicateTab()));
        menu.addAction(webTab->isPinned() ? tr("Un&pin Tab") : tr("&Pin Tab"), this, SLOT(pinTab()));
        menu.addSeparator();
        menu.addAction(tr("Re&load All Tabs"), m_tabWidget, SLOT(reloadAllTabs()));
        menu.addAction(tr("&Bookmark This Tab"), this, SLOT(bookmarkTab()));
        menu.addAction(tr("Bookmark &All Tabs"), p_QupZilla, SLOT(bookmarkAllTabs()));
        menu.addSeparator();
        QAction* action = p_QupZilla->actionRestoreTab();
        action->setEnabled(m_tabWidget->canRestoreTab());
        menu.addAction(action);
        menu.addSeparator();
        menu.addAction(tr("Close Ot&her Tabs"), this, SLOT(closeAllButCurrent()));
        menu.addAction(QIcon::fromTheme("window-close"), tr("Cl&ose"), this, SLOT(closeTab()));
        menu.addSeparator();
    }
    else {
        menu.addAction(tr("Reloa&d All Tabs"), m_tabWidget, SLOT(reloadAllTabs()));
        menu.addAction(tr("Bookmark &All Ta&bs"), p_QupZilla, SLOT(bookmarkAllTabs()));
        menu.addSeparator();
        QAction* action = menu.addAction(QIcon::fromTheme("user-trash"), tr("Restore &Closed Tab"), m_tabWidget, SLOT(restoreClosedTab()));
        action->setEnabled(m_tabWidget->canRestoreTab());
    }

    // Prevent choosing first option with double rightclick
    const QPoint &pos = mapToGlobal(position);
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);

    p_QupZilla->actionRestoreTab()->setEnabled(true);
}

QSize TabBar::tabSizeHint(int index) const
{
    if (!isVisible()) {
        // Don't calculate it when tabbar is not visible
        // It produces invalid size anyway
        return QSize(-1, -1);
    }

    static int PINNED_TAB_WIDTH = -1;
    static int MINIMUM_ACTIVE_TAB_WIDTH = -1;

    if (PINNED_TAB_WIDTH == -1) {
        PINNED_TAB_WIDTH = 16 + style()->pixelMetric(QStyle::PM_TabBarTabHSpace, 0, this);
        MINIMUM_ACTIVE_TAB_WIDTH = PINNED_TAB_WIDTH + style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, 0, this);
        // just a hack: we want to be sure buttonAddTab and buttonListTabs can't cover the active tab
        MINIMUM_ACTIVE_TAB_WIDTH = qMax(MINIMUM_ACTIVE_TAB_WIDTH, 6 + m_tabWidget->buttonListTabs()->width() + m_tabWidget->buttonAddTab()->width());
    }

    QSize size = QTabBar::tabSizeHint(index);
    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    TabBar* tabBar = const_cast <TabBar*>(this);
    bool adjustingActiveTab = false;

    if (webTab && webTab->isPinned()) {
        size.setWidth(PINNED_TAB_WIDTH);
    }
    else {
        int availableWidth = width() - (PINNED_TAB_WIDTH * m_pinnedTabsCount) - m_tabWidget->buttonListTabs()->width() - m_tabWidget->buttonAddTab()->width();
        if (availableWidth < 0) {
            return QSize(-1, -1);
        }

        int normalTabsCount = count() - m_pinnedTabsCount;
        if (availableWidth >= MAXIMUM_TAB_WIDTH * normalTabsCount) {
            m_normalTabWidth = MAXIMUM_TAB_WIDTH;
            size.setWidth(m_normalTabWidth);
        }
        else if (availableWidth < MINIMUM_TAB_WIDTH * normalTabsCount) {
            // Tabs don't fit at all in tabbar even with MINIMUM_TAB_WIDTH
            // We will try to use as low width of tabs as possible
            // to try avoid overflowing tabs into tabbar buttons

            int maxWidthForTab = availableWidth / normalTabsCount;
            m_activeTabWidth = maxWidthForTab;
            if (m_activeTabWidth < MINIMUM_ACTIVE_TAB_WIDTH) {
                maxWidthForTab = (availableWidth - MINIMUM_ACTIVE_TAB_WIDTH) / (normalTabsCount - 1);
                m_activeTabWidth = MINIMUM_ACTIVE_TAB_WIDTH;
                adjustingActiveTab = true;
            }

            if (maxWidthForTab < PINNED_TAB_WIDTH) {
                // FIXME: It overflows now
                m_normalTabWidth = PINNED_TAB_WIDTH;
                if (index == currentIndex()) {
                    size.setWidth(m_activeTabWidth);
                }
                else {
                    size.setWidth(m_normalTabWidth);
                }
            }
            else {
                m_normalTabWidth = maxWidthForTab;

                // Fill any empty space (we've got from rounding) with active tab
                if (index == currentIndex()) {
                    if (adjustingActiveTab) {
                        m_activeTabWidth = (availableWidth - MINIMUM_ACTIVE_TAB_WIDTH
                                            - maxWidthForTab * (normalTabsCount - 1)) + m_activeTabWidth;
                    }
                    else {
                        m_activeTabWidth = (availableWidth - maxWidthForTab * normalTabsCount) + maxWidthForTab;
                    }
                    adjustingActiveTab = true;
                    size.setWidth(m_activeTabWidth);
                }
                else {
                    size.setWidth(m_normalTabWidth);
                }

                if (tabsClosable()) {
                    // Hiding close buttons to save some space
                    tabBar->setTabsClosable(false);

                    tabBar->showCloseButton(currentIndex());
                }
            }
        }
        else {
            int maxWidthForTab = availableWidth / normalTabsCount;
            m_activeTabWidth = maxWidthForTab;
            if (m_activeTabWidth < MINIMUM_ACTIVE_TAB_WIDTH) {
                maxWidthForTab = (availableWidth - MINIMUM_ACTIVE_TAB_WIDTH) / (normalTabsCount - 1);
                m_activeTabWidth = MINIMUM_ACTIVE_TAB_WIDTH;
                adjustingActiveTab = true;
            }
            m_normalTabWidth = maxWidthForTab;

            // Fill any empty space (we've got from rounding) with active tab
            if (index == currentIndex()) {
                if (adjustingActiveTab) {
                    m_activeTabWidth = (availableWidth - MINIMUM_ACTIVE_TAB_WIDTH
                                        - maxWidthForTab * (normalTabsCount - 1)) + m_activeTabWidth;
                }
                else {
                    m_activeTabWidth = (availableWidth - maxWidthForTab * normalTabsCount) + maxWidthForTab;
                }
                adjustingActiveTab = true;
                size.setWidth(m_activeTabWidth);
            }
            else {
                size.setWidth(m_normalTabWidth);
            }

            // Restore close buttons according to preferences
            if (!tabsClosable()) {
                tabBar->setTabsClosable(true);

                // Hide close buttons on pinned tabs
                for (int i = 0; i < count(); ++i) {
                    tabBar->updatePinnedTabCloseButton(i);
                }
            }
        }
    }

    if (index == currentIndex()) {
        int xForAddTabButton = (PINNED_TAB_WIDTH * m_pinnedTabsCount) + (count() - m_pinnedTabsCount) * (m_normalTabWidth);
        if (adjustingActiveTab) {
            xForAddTabButton += m_activeTabWidth - m_normalTabWidth;
        }

        // RTL Support
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            xForAddTabButton = width() - xForAddTabButton;
        }
        emit tabBar->moveAddTabButton(xForAddTabButton);
    }

    return size;
}

void TabBar::showCloseButton(int index)
{
    if (!validIndex(index)) {
        return;
    }

    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    QAbstractButton* button = qobject_cast<QAbstractButton*>(tabButton(index, QTabBar::RightSide));

    if (button || (webTab && webTab->isPinned())) {
        return;
    }

    QAbstractButton* closeButton = new CloseButton(this);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeTabFromButton()));
    setTabButton(index, QTabBar::RightSide, closeButton);
}

void TabBar::hideCloseButton(int index)
{
    if (!validIndex(index) || tabsClosable()) {
        return;
    }

    CloseButton* button = qobject_cast<CloseButton*>(tabButton(index, QTabBar::RightSide));
    if (!button) {
        return;
    }

    setTabButton(index, QTabBar::RightSide, 0);
    button->deleteLater();
}

void TabBar::updatePinnedTabCloseButton(int index)
{
    if (!validIndex(index)) {
        return;
    }

    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    QAbstractButton* button = qobject_cast<QAbstractButton*>(tabButton(index, QTabBar::RightSide));

    bool pinned = webTab && webTab->isPinned();

    if (pinned) {
        if (button) {
            button->hide();
        }
    }
    else {
        if (button) {
            button->show();
        }
        else {
            showCloseButton(index);
        }
    }
}

void TabBar::closeCurrentTab()
{
    m_tabWidget->closeTab(currentIndex());
}

void TabBar::closeTabFromButton()
{
    QWidget* button = qobject_cast<QWidget*>(sender());

    int tabToClose = -1;

    for (int i = 0; i < count(); ++i) {
        if (tabButton(i, QTabBar::RightSide) == button) {
            tabToClose = i;
            break;
        }
    }

    if (tabToClose != -1) {
        m_tabWidget->closeTab(tabToClose);
    }
}

void TabBar::currentTabChanged(int index)
{
    if (!validIndex(index)) {
        return;
    }

    hideTabPreview(false);

    showCloseButton(index);
    hideCloseButton(m_tabWidget->lastTabIndex());

    m_tabWidget->currentTabChanged(index);
}

void TabBar::bookmarkTab()
{
    TabbedWebView* view = p_QupZilla->weView(m_clickedTab);
    if (!view) {
        return;
    }

    WebTab* tab = view->webTab();

    p_QupZilla->addBookmark(tab->url(), tab->title(), tab->icon());
}

void TabBar::pinTab()
{
    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_clickedTab));
    if (!webTab) {
        return;
    }

    webTab->pinTab(m_clickedTab);

    if (webTab->isPinned()) {
        m_pinnedTabsCount++;
    }
    else {
        m_pinnedTabsCount--;
    }

    // We need to recalculate size of all tabs and repaint tabbar
    // Unfortunately, Qt doesn't offer refresh() function as a public API

    // So we are calling the lightest function that calls d->refresh()
    setElideMode(elideMode());
}

void TabBar::pinnedTabClosed()
{
    m_pinnedTabsCount--;
}

void TabBar::pinnedTabAdded()
{
    m_pinnedTabsCount++;
}

int TabBar::pinnedTabsCount()
{
    return m_pinnedTabsCount;
}

int TabBar::normalTabsCount()
{
    return count() - m_pinnedTabsCount;
}

void TabBar::showTabPreview()
{
    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_tabPreview->previewIndex()));
    if (!webTab) {
        return;
    }

    m_tabPreviewTimer->stop();
    m_tabPreview->setWebTab(webTab, m_tabPreview->previewIndex() == currentIndex());
    m_tabPreview->showOnRect(tabRect(m_tabPreview->previewIndex()));
}

void TabBar::hideTabPreview(bool delayed)
{
    if (delayed) {
        m_tabPreviewTimer->start();
    }
    else {
        m_tabPreview->hideAnimated();
    }
}

void TabBar::tabRemoved(int index)
{
    Q_UNUSED(index)

    m_tabWidget->showNavigationBar(p_QupZilla->navigationContainer());
    showCloseButton(currentIndex());
}

void TabBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (mApp->plugins()->processMouseDoubleClick(Qz::ON_TabBar, this, event)) {
        return;
    }

    if (event->button() == Qt::LeftButton && tabAt(event->pos()) == -1) {
        m_tabWidget->addView(QUrl(), Qz::NT_SelectedTabAtTheEnd, true);
        return;
    }

    QTabBar::mouseDoubleClickEvent(event);
}

void TabBar::mousePressEvent(QMouseEvent* event)
{
    hideTabPreview(false);

    if (mApp->plugins()->processMousePress(Qz::ON_TabBar, this, event)) {
        return;
    }

    if (event->buttons() & Qt::LeftButton && tabAt(event->pos()) != -1) {
        m_dragStartPosition = mapFromGlobal(event->globalPos());
    }
    else {
        m_dragStartPosition = QPoint();
    }

    QTabBar::mousePressEvent(event);
}

void TabBar::mouseMoveEvent(QMouseEvent* event)
{
    if (mApp->plugins()->processMouseMove(Qz::ON_TabBar, this, event)) {
        return;
    }

    if (!m_dragStartPosition.isNull() && m_tabWidget->buttonAddTab()->isVisible()) {
        int manhattanLength = (event->pos() - m_dragStartPosition).manhattanLength();
        if (manhattanLength > QApplication::startDragDistance()) {
            m_tabWidget->buttonAddTab()->hide();
            hideTabPreview();
        }
    }

    // Tab Preview

    const int tab = tabAt(event->pos());

    if (tab != -1 && tab != m_tabPreview->previewIndex() && event->buttons() == Qt::NoButton && m_dragStartPosition.isNull()) {
        m_tabPreview->setPreviewIndex(tab);
        if (m_tabPreview->isVisible()) {
            showTabPreview();
        }
    }

    QTabBar::mouseMoveEvent(event);
}

void TabBar::mouseReleaseEvent(QMouseEvent* event)
{
    m_dragStartPosition = QPoint();

    if (mApp->plugins()->processMouseRelease(Qz::ON_TabBar, this, event)) {
        return;
    }

    if (m_tabWidget->buttonAddTab()->isHidden()) {
        QTimer::singleShot(500, m_tabWidget->buttonAddTab(), SLOT(show()));
    }

    if (!rect().contains(event->pos())) {
        QTabBar::mouseReleaseEvent(event);
        return;
    }

    int id = tabAt(event->pos());
    if (id != -1 && event->button() == Qt::MiddleButton) {
        m_tabWidget->closeTab(id);
        return;
    }
    if (id == -1 && event->button() == Qt::MiddleButton) {
        m_tabWidget->addView(QUrl(), Qz::NT_SelectedTabAtTheEnd, true);
        return;
    }

    QTabBar::mouseReleaseEvent(event);
}

bool TabBar::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::Leave:
        hideTabPreview();
        break;

    case QEvent::ToolTip:
        if (m_showTabPreviews) {
            QHelpEvent* ev = static_cast<QHelpEvent*>(event);
            if (tabAt(ev->pos()) != -1 && !m_tabPreview->isVisible()) {
                showTabPreview();
            }
            return true;
        }
        break;

    default:
        break;
    }

    return QTabBar::event(event);
}

void TabBar::wheelEvent(QWheelEvent* event)
{
    if (mApp->plugins()->processWheelEvent(Qz::ON_TabBar, this, event)) {
        return;
    }

    QTabBar::wheelEvent(event);
}

void TabBar::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* mime = event->mimeData();

    if (mime->hasUrls()) {
        event->acceptProposedAction();
        return;
    }

    QTabBar::dragEnterEvent(event);
}

void TabBar::dropEvent(QDropEvent* event)
{
    const QMimeData* mime = event->mimeData();

    if (!mime->hasUrls()) {
        QTabBar::dropEvent(event);
        return;
    }

    int index = tabAt(event->pos());
    if (index == -1) {
        foreach(const QUrl & url, mime->urls()) {
            m_tabWidget->addView(url, Qz::NT_SelectedTabAtTheEnd);
        }
    }
    else {
        WebTab* tab = p_QupZilla->weView(index)->webTab();
        if (tab->isRestored()) {
            tab->view()->load(mime->urls().at(0));
        }
    }
}

void TabBar::disconnectObjects()
{
    disconnect(this);
}

CloseButton::CloseButton(QWidget* parent)
    : QAbstractButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::ArrowCursor);
    setToolTip(QupZilla::tr("Close Tab"));

    resize(sizeHint());
}

QSize CloseButton::sizeHint() const
{
    ensurePolished();
    int width = style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, 0, this);
    int height = style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, 0, this);
    return QSize(width, height);
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

    if (isEnabled() && underMouse() && !isChecked() && !isDown()) {
        opt.state |= QStyle::State_Raised;
    }
    if (isChecked()) {
        opt.state |= QStyle::State_On;
    }
    if (isDown()) {
        opt.state |= QStyle::State_Sunken;
    }

    if (const QTabBar* tb = qobject_cast<const QTabBar*>(parent())) {
        int index = tb->currentIndex();
        QTabBar::ButtonPosition position = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, tb);
        if (tb->tabButton(index, position) == this) {
            opt.state |= QStyle::State_Selected;
        }
    }

    style()->drawPrimitive(QStyle::PE_IndicatorTabClose, &opt, &p, this);
}
