/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "proxystyle.h"

#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QMessageBox>
#include <QStyleOption>
#include <QApplication>
#include <QTimer>
#include <QRect>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QToolTip>

TabBar::TabBar(QupZilla* mainClass, TabWidget* tabWidget)
    : ComboTabBar()
    , p_QupZilla(mainClass)
    , m_tabWidget(tabWidget)
    , m_tabPreview(new TabPreview(mainClass, mainClass))
    , m_showTabPreviews(false)
    , m_hideTabBarWithOneTab(false)
    , m_clickedTab(0)
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
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));

    m_tabPreviewShowTimer = new QTimer(this);
    m_tabPreviewShowTimer->setInterval(300);
    m_tabPreviewShowTimer->setSingleShot(true);
    connect(m_tabPreviewShowTimer, SIGNAL(timeout()), this, SLOT(showTabPreview()));

    m_tabPreviewHideTimer = new QTimer(this);
    m_tabPreviewHideTimer->setInterval(300);
    m_tabPreviewHideTimer->setSingleShot(true);
    connect(m_tabPreviewHideTimer, SIGNAL(timeout()), m_tabPreview, SLOT(hideAnimated()));

    // ComboTabBar features
    setUsesScrollButtons(true);
    setCloseButtonsToolTip(QupZilla::tr("Close Tab"));
    connect(this, SIGNAL(overFlowChanged(bool)), this, SLOT(overFlowChange(bool)));
    connect(this, SIGNAL(scrollBarValueChanged(int)), this, SLOT(hideTabPreview()));
}

void TabBar::loadSettings()
{
    Settings settings;
    settings.beginGroup("Browser-Tabs-Settings");
    m_hideTabBarWithOneTab = settings.value("hideTabsWithOneTab", false).toBool();
    m_tabPreview->setAnimationsEnabled(settings.value("tabPreviewAnimationsEnabled", true).toBool());
    m_showTabPreviews = settings.value("showTabPreviews", false).toBool();
    bool activateLastTab = settings.value("ActivateLastTabWhenClosingActual", false).toBool();
    settings.endGroup();

    setSelectionBehaviorOnRemove(activateLastTab ? QTabBar::SelectPreviousTab : QTabBar::SelectRightTab);
    setVisible(!(count() == 1 && m_hideTabBarWithOneTab));

    setUpLayout();
}

void TabBar::updateVisibilityWithFullscreen(bool visible)
{
    // It is needed to save original geometry, otherwise
    // tabbar will get 3px height in fullscreen once it was hidden

    // Make sure to honor user preference
    if (visible) {
        visible = !(count() == 1 && m_hideTabBarWithOneTab);
    }

    ComboTabBar::setVisible(visible);

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
    if (visible && p_QupZilla->isFullScreen()) {
        return;
    }

    // Make sure to honor user preference
    if (visible) {
        visible = !(count() == 1 && m_hideTabBarWithOneTab);
    }

    if (visible) {
        emit showButtons();
    }
    else {
        m_originalGeometry = geometry();
        emit hideButtons();
    }

    hideTabPreview(false);
    ComboTabBar::setVisible(visible);
}

void TabBar::contextMenuRequested(const QPoint &position)
{
    int index = tabAt(position);
    m_clickedTab = index;

    QMenu menu;
    menu.addAction(QIcon::fromTheme("tab-new", QIcon(":/icons/menu/tab-new.png")), tr("&New tab"), p_QupZilla, SLOT(addTab()));
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

        menu.addAction(QIcon::fromTheme("tab-duplicate"), tr("&Duplicate Tab"), this, SLOT(duplicateTab()));

        if (count() > 1 && !webTab->isPinned()) {
            menu.addAction(QIcon::fromTheme("tab-detach"), tr("D&etach Tab"), this, SLOT(detachTab()));
        }

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
        menu.addAction(tr("Bookmark &All Tabs"), p_QupZilla, SLOT(bookmarkAllTabs()));
        menu.addSeparator();
        QAction* action = menu.addAction(QIcon::fromTheme("user-trash"), tr("Restore &Closed Tab"), m_tabWidget, SLOT(restoreClosedTab()));
        action->setEnabled(m_tabWidget->canRestoreTab());
    }

    // Prevent choosing first option with double rightclick
    const QPoint pos = mapToGlobal(position);
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);

    p_QupZilla->actionRestoreTab()->setEnabled(true);
}

void TabBar::closeAllButCurrent()
{
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Close Tabs"), tr("Do you really want to close other tabs?"),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (button == QMessageBox::Yes) {
        emit closeAllButCurrent(m_clickedTab);
    }
}

QSize TabBar::tabSizeHint(int index, bool fast) const
{
    if (!isVisible() || !mApp->proxyStyle()) {
        // Don't calculate it when tabbar is not visible
        // It produces invalid size anyway
        //
        // We also need ProxyStyle to be set before calculating minimum sizes for tabs
        return QSize(-1, -1);
    }

    static int PINNED_TAB_WIDTH = comboTabBarPixelMetric(ComboTabBar::PinnedTabWidth);
    static int MINIMUM_ACTIVE_TAB_WIDTH = comboTabBarPixelMetric(ComboTabBar::ActiveTabMinimumWidth);
    static int MAXIMUM_TAB_WIDTH = comboTabBarPixelMetric(ComboTabBar::NormalTabMaximumWidth);
    static int MINIMUM_TAB_WIDTH = comboTabBarPixelMetric(ComboTabBar::NormalTabMinimumWidth);

    QSize size = ComboTabBar::tabSizeHint(index);

    // The overflowed tabs have similar size and we can use this fast method
    if (fast) {
        size.setWidth(index >= pinnedTabsCount() ? MINIMUM_TAB_WIDTH : PINNED_TAB_WIDTH);
        return size;
    }

    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    TabBar* tabBar = const_cast <TabBar*>(this);

    if (webTab && webTab->isPinned()) {
        size.setWidth(PINNED_TAB_WIDTH);
    }
    else {
        int availableWidth = mainTabBarWidth();

        if (!m_tabWidget->buttonListTabs()->isForceHidden()) {
            availableWidth -= comboTabBarPixelMetric(ExtraReservedWidth);
        }

        if (availableWidth < 0) {
            return QSize(-1, -1);
        }

        const int normalTabsCount = ComboTabBar::normalTabsCount();

        if (availableWidth >= MAXIMUM_TAB_WIDTH * normalTabsCount) {
            m_normalTabWidth = MAXIMUM_TAB_WIDTH;
            size.setWidth(m_normalTabWidth);
        }
        else if (normalTabsCount > 0) {
            int maxWidthForTab = availableWidth / normalTabsCount;
            int realTabWidth = maxWidthForTab;
            bool adjustingActiveTab = false;

            if (realTabWidth < MINIMUM_ACTIVE_TAB_WIDTH) {
                maxWidthForTab = normalTabsCount > 1 ? (availableWidth - MINIMUM_ACTIVE_TAB_WIDTH) / (normalTabsCount - 1) : 0;
                realTabWidth = MINIMUM_ACTIVE_TAB_WIDTH;
                adjustingActiveTab = true;
            }

            bool tryAdjusting = availableWidth >= MINIMUM_TAB_WIDTH * normalTabsCount;

            if (tabsClosable() && availableWidth < (MINIMUM_TAB_WIDTH + 25) * normalTabsCount) {
                // Hiding close buttons to save some space
                tabBar->setTabsClosable(false);
                tabBar->showCloseButton(currentIndex());
            }

            if (tryAdjusting) {
                m_normalTabWidth = maxWidthForTab;

                // Fill any empty space (we've got from rounding) with active tab
                if (index == mainTabBarCurrentIndex()) {
                    if (adjustingActiveTab) {
                        m_activeTabWidth = (availableWidth - MINIMUM_ACTIVE_TAB_WIDTH
                                            - maxWidthForTab * (normalTabsCount - 1)) + realTabWidth;
                    }
                    else {
                        m_activeTabWidth = (availableWidth - maxWidthForTab * normalTabsCount) + maxWidthForTab;
                    }
                    size.setWidth(m_activeTabWidth);
                }
                else {
                    size.setWidth(m_normalTabWidth);
                }
            }
        }

        // Restore close buttons according to preferences
        if (!tabsClosable() && availableWidth >= (MINIMUM_TAB_WIDTH + 25) * normalTabsCount) {
            tabBar->setTabsClosable(true);

            // Hide close buttons on pinned tabs
            for (int i = 0; i < count(); ++i) {
                tabBar->updatePinnedTabCloseButton(i);
            }
        }
    }

    if (index == count() - 1) {
        WebTab* lastMainActiveTab = qobject_cast<WebTab*>(m_tabWidget->widget(mainTabBarCurrentIndex()));
        int xForAddTabButton = pinTabBarWidth() + normalTabsCount() * m_normalTabWidth;

        if (lastMainActiveTab && m_activeTabWidth > m_normalTabWidth) {
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

int TabBar::comboTabBarPixelMetric(ComboTabBar::SizeType sizeType) const
{
    if (!mApp->proxyStyle() || !isVisible()) {
        return -1;
    }

    switch (sizeType) {
    case ComboTabBar::PinnedTabWidth:
        return 16 + mApp->proxyStyle()->pixelMetric(QStyle::PM_TabBarTabHSpace, 0, this);

    case ComboTabBar::ActiveTabMinimumWidth:
    case ComboTabBar::NormalTabMinimumWidth:
    case ComboTabBar::OverflowedTabWidth:
        return 100;

    case ComboTabBar::NormalTabMaximumWidth:
        return 250;

    case ComboTabBar::ExtraReservedWidth:
        return m_tabWidget->buttonListTabs()->width() + m_tabWidget->buttonAddTab()->width();

    default:
        break;
    }

    return -1;
}

void TabBar::showCloseButton(int index)
{
    if (!validIndex(index)) {
        return;
    }

    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    QAbstractButton* button = qobject_cast<QAbstractButton*>(tabButton(index, closeButtonPosition()));

    if (button || (webTab && webTab->isPinned())) {
        return;
    }

    insertCloseButton(index);
}

void TabBar::hideCloseButton(int index)
{
    if (!validIndex(index) || tabsClosable()) {
        return;
    }

    CloseButton* button = qobject_cast<CloseButton*>(tabButton(index, closeButtonPosition()));
    if (!button) {
        return;
    }

    setTabButton(index, closeButtonPosition(), 0);
    button->deleteLater();
}

void TabBar::updatePinnedTabCloseButton(int index)
{
    if (!validIndex(index)) {
        return;
    }

    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    QAbstractButton* button = qobject_cast<QAbstractButton*>(tabButton(index, closeButtonPosition()));

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
        if (tabButton(i, closeButtonPosition()) == button) {
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

    // Don't hide close buttons when dragging tabs
    if (m_dragStartPosition.isNull()) {
        showCloseButton(index);
        hideCloseButton(m_tabWidget->lastTabIndex());

        ensureVisible(index);
    }

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
}

void TabBar::overrideTabTextColor(int index, QColor color)
{
    if (!m_originalTabTextColor.isValid()) {
        m_originalTabTextColor = tabTextColor(index);
    }

    setTabTextColor(index, color);
}

void TabBar::restoreTabTextColor(int index)
{
    setTabTextColor(index, m_originalTabTextColor);
}

void TabBar::showTabPreview(bool delayed)
{
    if (!m_showTabPreviews) {
        return;
    }

    if (delayed) {
        int index = tabAt(mapFromGlobal(QCursor::pos()));
        if (index == -1 || QApplication::mouseButtons() != Qt::NoButton) {
            return;
        }

        m_tabPreview->setPreviewIndex(index);
        m_tabPreviewShowTimer->stop();
    }

    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_tabPreview->previewIndex()));
    if (!webTab) {
        return;
    }

    m_tabPreviewHideTimer->stop();
    m_tabPreview->setWebTab(webTab, m_tabPreview->previewIndex() == currentIndex());

    QRect r(tabRect(m_tabPreview->previewIndex()));
    r.setTopLeft(mapTo(p_QupZilla, r.topLeft()));
    r.setBottomRight(mapTo(p_QupZilla, r.bottomRight()));

    m_tabPreview->showOnRect(r);
}

void TabBar::hideTabPreview(bool delayed)
{
    m_tabPreviewShowTimer->stop();

    if (delayed) {
        m_tabPreviewHideTimer->start();
    }
    else {
        m_tabPreview->hideAnimated();
    }
}

void TabBar::overFlowChange(bool overFlowed)
{
    if (overFlowed) {
        m_tabWidget->buttonAddTab()->setForceHidden(true);
        m_tabWidget->buttonListTabs()->setForceHidden(true);
        m_tabWidget->setUpLayout();
        ensureVisible(currentIndex());
    }
    else {
        m_tabWidget->buttonAddTab()->setForceHidden(false);
        m_tabWidget->buttonListTabs()->setForceHidden(false);
        m_tabWidget->showButtons();
        m_tabWidget->setUpLayout();
    }
}

void TabBar::tabInserted(int index)
{
    Q_UNUSED(index)

    setVisible(!(count() == 1 && m_hideTabBarWithOneTab));
}

void TabBar::tabRemoved(int index)
{
    Q_UNUSED(index)

    showCloseButton(currentIndex());
    setVisible(!(count() == 1 && m_hideTabBarWithOneTab));
}

void TabBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (mApp->plugins()->processMouseDoubleClick(Qz::ON_TabBar, this, event)) {
        return;
    }

    if (event->buttons() == Qt::LeftButton && emptyArea(event->pos())) {
        m_tabWidget->addView(QUrl(), Qz::NT_SelectedTabAtTheEnd, true);
        return;
    }

    ComboTabBar::mouseDoubleClickEvent(event);
}

void TabBar::mousePressEvent(QMouseEvent* event)
{
    hideTabPreview(false);

    if (mApp->plugins()->processMousePress(Qz::ON_TabBar, this, event)) {
        return;
    }

    if (event->buttons() == Qt::LeftButton && !emptyArea(event->pos())) {
        m_dragStartPosition = mapFromGlobal(event->globalPos());
    }
    else {
        m_dragStartPosition = QPoint();
    }

    ComboTabBar::mousePressEvent(event);
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

    if (m_tabPreview->isVisible() && tab != -1 && tab != m_tabPreview->previewIndex() && event->buttons() == Qt::NoButton && m_dragStartPosition.isNull()) {
        m_tabPreview->setPreviewIndex(tab);
        showTabPreview(false);
    }

    if (!m_tabPreview->isVisible()) {
        m_tabPreviewShowTimer->start();
    }

    ComboTabBar::mouseMoveEvent(event);
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
        ComboTabBar::mouseReleaseEvent(event);
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

    ComboTabBar::mouseReleaseEvent(event);
}

bool TabBar::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::ToolTip:
        if (!m_showTabPreviews && !isDragInProgress()) {
            QHelpEvent* ev = static_cast<QHelpEvent*>(event);
            int index = tabAt(ev->pos());

            if (index >= 0) {
                QToolTip::showText(mapToGlobal(ev->pos()), tabToolTip(index));
            }
        }
        break;

    case QEvent::Leave:
        if (!rect().contains(mapFromGlobal(QCursor::pos()))) {
            hideTabPreview();
        }
        break;

    case QEvent::Wheel:
        hideTabPreview(false);
        break;

    default:
        break;
    }

    return ComboTabBar::event(event);
}

void TabBar::resizeEvent(QResizeEvent* e)
{
    QPoint posit;
    posit.setY(0);

    if (isRightToLeft()) {
        posit.setX(0);
    }
    else {
        posit.setX(width() - m_tabWidget->buttonListTabs()->width());
    }
    m_tabWidget->buttonListTabs()->move(posit);

    ComboTabBar::resizeEvent(e);
}

void TabBar::wheelEvent(QWheelEvent* event)
{
    if (mApp->plugins()->processWheelEvent(Qz::ON_TabBar, this, event)) {
        return;
    }

    ComboTabBar::wheelEvent(event);
}

void TabBar::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* mime = event->mimeData();

    if (mime->hasUrls()) {
        event->acceptProposedAction();
        return;
    }

    ComboTabBar::dragEnterEvent(event);
}

void TabBar::dropEvent(QDropEvent* event)
{
    const QMimeData* mime = event->mimeData();

    if (!mime->hasUrls()) {
        ComboTabBar::dropEvent(event);
        return;
    }

    int index = tabAt(event->pos());
    if (index == -1) {
        foreach (const QUrl &url, mime->urls()) {
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
