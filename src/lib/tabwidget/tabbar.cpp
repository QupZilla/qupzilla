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
#include "browserwindow.h"
#include "webtab.h"
#include "toolbutton.h"
#include "settings.h"
#include "tabbedwebview.h"
#include "mainapplication.h"
#include "pluginproxy.h"
#include "iconprovider.h"

#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QMessageBox>
#include <QStyleOption>
#include <QApplication>
#include <QTimer>
#include <QRect>
#include <QLabel>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QToolTip>

TabBar::TabBar(BrowserWindow* window, TabWidget* tabWidget)
    : ComboTabBar()
    , m_window(window)
    , m_tabWidget(tabWidget)
    , m_tabPreview(new TabPreview(window, window))
    , m_showTabPreviews(false)
    , m_hideTabBarWithOneTab(false)
    , m_showCloseOnInactive(0)
    , m_clickedTab(0)
    , m_normalTabWidth(0)
    , m_activeTabWidth(0)
{
    setObjectName("tabbar");
    setElideMode(Qt::ElideRight);
    setFocusPolicy(Qt::NoFocus);
    setTabsClosable(false);
    setMouseTracking(true);
    setDocumentMode(true);
    setAcceptDrops(true);
    setDrawBase(false);
    setMovable(true);

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

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
    setCloseButtonsToolTip(BrowserWindow::tr("Close Tab"));
    connect(this, SIGNAL(scrollBarValueChanged(int)), this, SLOT(hideTabPreview()));
    connect(this, SIGNAL(overFlowChanged(bool)), this, SLOT(overflowChanged(bool)));

    if (mApp->isPrivate()) {
        QLabel* privateBrowsing = new QLabel(this);
        privateBrowsing->setObjectName(QSL("private-browsing-icon"));
        privateBrowsing->setPixmap(IconProvider::privateBrowsingIcon().pixmap(16, 16));
        privateBrowsing->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        privateBrowsing->setFixedWidth(30);
        addCornerWidget(privateBrowsing, Qt::TopLeftCorner);
    }
}

void TabBar::loadSettings()
{
    Settings settings;
    settings.beginGroup("Browser-Tabs-Settings");
    m_hideTabBarWithOneTab = settings.value("hideTabsWithOneTab", false).toBool();
    m_tabPreview->setAnimationsEnabled(settings.value("tabPreviewAnimationsEnabled", true).toBool());
    m_showTabPreviews = settings.value("showTabPreviews", false).toBool();
    bool activateLastTab = settings.value("ActivateLastTabWhenClosingActual", false).toBool();
    m_showCloseOnInactive = settings.value("showCloseOnInactiveTabs", 0).toInt(0);
    settings.endGroup();

    setSelectionBehaviorOnRemove(activateLastTab ? QTabBar::SelectPreviousTab : QTabBar::SelectRightTab);
    setVisible(!(count() == 1 && m_hideTabBarWithOneTab));

    setUpLayout();
}

TabWidget* TabBar::tabWidget() const
{
    return m_tabWidget;
}

void TabBar::setVisible(bool visible)
{
    if (visible && m_window->isFullScreen()) {
        return;
    }

    // Make sure to honor user preference
    if (visible) {
        visible = !(count() == 1 && m_hideTabBarWithOneTab);
    }

    hideTabPreview(false);
    ComboTabBar::setVisible(visible);
}

void TabBar::overflowChanged(bool overflowed)
{
    // Make sure close buttons on inactive tabs are hidden
    // This is needed for when leaving fullscreen from non-overflowed to overflowed state
    if (overflowed && m_showCloseOnInactive != 1) {
        setTabsClosable(false);
        showCloseButton(currentIndex());
    }
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
    if (!isVisible()) {
        // Don't calculate it when tabbar is not visible
        // It produces invalid size anyway
        return QSize(-1, -1);
    }

    const int pinnedTabWidth = comboTabBarPixelMetric(ComboTabBar::PinnedTabWidth);
    const int minTabWidth = comboTabBarPixelMetric(ComboTabBar::NormalTabMinimumWidth);

    QSize size = ComboTabBar::tabSizeHint(index);

    // The overflowed tabs have same size and we can use this fast method
    if (fast) {
        size.setWidth(index >= pinnedTabsCount() ? minTabWidth : pinnedTabWidth);
        return size;
    }

    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    TabBar* tabBar = const_cast <TabBar*>(this);

    if (webTab && webTab->isPinned()) {
        size.setWidth(pinnedTabWidth);
    }
    else {
        int availableWidth = mainTabBarWidth() - comboTabBarPixelMetric(ExtraReservedWidth);

        if (availableWidth < 0) {
            return QSize(-1, -1);
        }

        const int normalTabsCount = ComboTabBar::normalTabsCount();
        const int maxTabWidth = comboTabBarPixelMetric(ComboTabBar::NormalTabMaximumWidth);

        if (availableWidth >= maxTabWidth * normalTabsCount) {
            m_normalTabWidth = maxTabWidth;
            size.setWidth(m_normalTabWidth);
        }
        else if (normalTabsCount > 0) {
            const int minActiveTabWidth = comboTabBarPixelMetric(ComboTabBar::ActiveTabMinimumWidth);

            int maxWidthForTab = availableWidth / normalTabsCount;
            int realTabWidth = maxWidthForTab;
            bool adjustingActiveTab = false;

            if (realTabWidth < minActiveTabWidth) {
                maxWidthForTab = normalTabsCount > 1 ? (availableWidth - minActiveTabWidth) / (normalTabsCount - 1) : 0;
                realTabWidth = minActiveTabWidth;
                adjustingActiveTab = true;
            }

            bool tryAdjusting = availableWidth >= minTabWidth * normalTabsCount;

            if (m_showCloseOnInactive != 1 && tabsClosable() && availableWidth < (minTabWidth + 25) * normalTabsCount) {
                // Hiding close buttons to save some space
                tabBar->setTabsClosable(false);
                tabBar->showCloseButton(currentIndex());
            }
            if (m_showCloseOnInactive == 1) {
                // Always showing close buttons
                tabBar->setTabsClosable(true);
                tabBar->showCloseButton(currentIndex());
            }

            if (tryAdjusting) {
                m_normalTabWidth = maxWidthForTab;

                // Fill any empty space (we've got from rounding) with active tab
                if (index == mainTabBarCurrentIndex()) {
                    if (adjustingActiveTab) {
                        m_activeTabWidth = (availableWidth - minActiveTabWidth
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
        if (m_showCloseOnInactive != 2 && !tabsClosable() && availableWidth >= (minTabWidth + 25) * normalTabsCount) {
            tabBar->setTabsClosable(true);

            // Hide close buttons on pinned tabs
            for (int i = 0; i < count(); ++i) {
                tabBar->updatePinnedTabCloseButton(i);
            }
        }
    }

    if (index == count() - 1) {
        WebTab* lastMainActiveTab = qobject_cast<WebTab*>(m_tabWidget->widget(mainTabBarCurrentIndex()));
        int xForAddTabButton = cornerWidth(Qt::TopLeftCorner) + pinTabBarWidth() + normalTabsCount() * m_normalTabWidth;

        if (lastMainActiveTab && m_activeTabWidth > m_normalTabWidth) {
            xForAddTabButton += m_activeTabWidth - m_normalTabWidth;
        }

        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            xForAddTabButton = width() - xForAddTabButton;
        }

        emit tabBar->moveAddTabButton(xForAddTabButton);
    }

    return size;
}

int TabBar::comboTabBarPixelMetric(ComboTabBar::SizeType sizeType) const
{
    if (!isVisible())
        return -1;

    switch (sizeType) {
    case ComboTabBar::PinnedTabWidth:
        return 16 + style()->pixelMetric(QStyle::PM_TabBarTabHSpace, 0, this);

    case ComboTabBar::ActiveTabMinimumWidth:
    case ComboTabBar::NormalTabMinimumWidth:
    case ComboTabBar::OverflowedTabWidth:
        return 100;

    case ComboTabBar::NormalTabMaximumWidth:
        return 250;

    case ComboTabBar::ExtraReservedWidth:
        return m_tabWidget->extraReservedWidth();

    default:
        break;
    }

    return -1;
}

WebTab* TabBar::webTab(int index) const
{
    if (index == -1) {
        return qobject_cast<WebTab*>(m_tabWidget->widget(currentIndex()));
    }
    return qobject_cast<WebTab*>(m_tabWidget->widget(index));
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

void TabBar::contextMenuEvent(QContextMenuEvent* event)
{
    int index = tabAt(event->pos());
    m_clickedTab = index;

    QMenu menu;
    menu.addAction(IconProvider::newTabIcon(), tr("&New tab"), m_window, SLOT(addTab()));
    menu.addSeparator();
    if (index != -1) {
        WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_clickedTab));
        if (!webTab) {
            return;
        }

        if (m_window->weView(m_clickedTab)->isLoading()) {
            menu.addAction(QIcon::fromTheme(QSL("process-stop")), tr("&Stop Tab"), this, SLOT(stopTab()));
        }
        else {
            menu.addAction(QIcon::fromTheme(QSL("view-refresh")), tr("&Reload Tab"), this, SLOT(reloadTab()));
        }

        menu.addAction(QIcon::fromTheme("tab-duplicate"), tr("&Duplicate Tab"), this, SLOT(duplicateTab()));

        if (count() > 1 && !webTab->isPinned()) {
            menu.addAction(QIcon::fromTheme("tab-detach"), tr("D&etach Tab"), this, SLOT(detachTab()));
        }

        menu.addAction(webTab->isPinned() ? tr("Un&pin Tab") : tr("&Pin Tab"), this, SLOT(pinTab()));
        menu.addSeparator();
        menu.addAction(tr("Re&load All Tabs"), m_tabWidget, SLOT(reloadAllTabs()));
        menu.addAction(tr("&Bookmark This Tab"), this, SLOT(bookmarkTab()));
        menu.addAction(tr("Bookmark &All Tabs"), m_window, SLOT(bookmarkAllTabs()));
        menu.addSeparator();
        menu.addAction(m_window->action(QSL("Other/RestoreClosedTab")));
        menu.addSeparator();
        menu.addAction(tr("Close Ot&her Tabs"), this, SLOT(closeAllButCurrent()));
        menu.addAction(QIcon::fromTheme("window-close"), tr("Cl&ose"), this, SLOT(closeTab()));
        menu.addSeparator();
    }
    else {
        menu.addAction(tr("Reloa&d All Tabs"), m_tabWidget, SLOT(reloadAllTabs()));
        menu.addAction(tr("Bookmark &All Tabs"), m_window, SLOT(bookmarkAllTabs()));
        menu.addSeparator();
        menu.addAction(m_window->action(QSL("Other/RestoreClosedTab")));
    }

    m_window->action(QSL("Other/RestoreClosedTab"))->setEnabled(m_tabWidget->canRestoreTab());

    // Prevent choosing first option with double rightclick
    const QPoint pos = event->globalPos();
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);

    m_window->action(QSL("Other/RestoreClosedTab"))->setEnabled(true);
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
    TabbedWebView* view = m_window->weView(m_clickedTab);
    if (!view) {
        return;
    }

    WebTab* tab = view->webTab();

    m_window->addBookmark(tab->url(), tab->title());
}

void TabBar::pinTab()
{
    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_clickedTab));

    if (webTab) {
        webTab->togglePinned();
    }
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

void TabBar::setTabText(int index, const QString &text)
{
    QString tabText = text;

    // Avoid Alt+letter shortcuts
    tabText.replace(QLatin1Char('&'), QLatin1String("&&"));

    if (WebTab* tab = webTab(index)) {
        if (tab->isPinned()) {
            tabText.clear();
        }
    }

    setTabToolTip(index, text);
    ComboTabBar::setTabText(index, tabText);
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
    r.setTopLeft(mapTo(m_window, r.topLeft()));
    r.setBottomRight(mapTo(m_window, r.bottomRight()));

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

    // Make sure to move add tab button to correct position when there are no normal tabs
    if (normalTabsCount() == 0) {
        int xForAddTabButton = cornerWidth(Qt::TopLeftCorner) + pinTabBarWidth();
        if (QApplication::layoutDirection() == Qt::RightToLeft)
            xForAddTabButton = width() - xForAddTabButton;
        emit moveAddTabButton(xForAddTabButton);
    }
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

    if (m_tabWidget->buttonAddTab()->isHidden() && !isMainBarOverflowed()) {
        QTimer::singleShot(500, m_tabWidget->buttonAddTab(), SLOT(show()));
    }

    if (!rect().contains(event->pos())) {
        ComboTabBar::mouseReleaseEvent(event);
        return;
    }

    if (event->button() == Qt::MiddleButton) {
        if (emptyArea(event->pos())) {
            m_tabWidget->addView(QUrl(), Qz::NT_SelectedTabAtTheEnd, true);
            return;
        }

        int id = tabAt(event->pos());
        if (id != -1) {
            m_tabWidget->closeTab(id);
            return;
        }
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
        WebTab* tab = m_window->weView(index)->webTab();
        if (tab->isRestored()) {
            tab->webView()->load(mime->urls().at(0));
        }
    }
}
