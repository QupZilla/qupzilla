/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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

TabBar::TabBar(BrowserWindow* window, TabWidget* tabWidget)
    : ComboTabBar()
    , m_window(window)
    , m_tabWidget(tabWidget)
    , m_hideTabBarWithOneTab(false)
    , m_showCloseOnInactive(0)
    , m_clickedTab(0)
    , m_normalTabWidth(0)
    , m_activeTabWidth(0)
    , m_forceHidden(false)
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

    // ComboTabBar features
    setUsesScrollButtons(true);
    setCloseButtonsToolTip(BrowserWindow::tr("Close Tab"));
    connect(this, SIGNAL(overFlowChanged(bool)), this, SLOT(overflowChanged(bool)));

    if (mApp->isPrivate()) {
        QLabel* privateBrowsing = new QLabel(this);
        privateBrowsing->setObjectName(QSL("private-browsing-icon"));
        privateBrowsing->setPixmap(IconProvider::privateBrowsingIcon().pixmap(16));
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
    if (m_forceHidden) {
        ComboTabBar::setVisible(false);
        return;
    }

    // Make sure to honor user preference
    if (visible) {
        visible = !(count() == 1 && m_hideTabBarWithOneTab);
    }

    ComboTabBar::setVisible(visible);
}

void TabBar::setForceHidden(bool hidden)
{
    m_forceHidden = hidden;
    setVisible(!m_forceHidden);
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

//TODO: replace these 3 w/ preferencable mbox
void TabBar::closeAllButCurrent()
{
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Close Tabs"), tr("Do you really want to close other tabs?"),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (button == QMessageBox::Yes) {
        emit closeAllButCurrent(m_clickedTab);
    }
}

void TabBar::closeToRight()
{
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Close Tabs"), tr("Do you really want to close all tabs to the right?"),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (button == QMessageBox::Yes) {
        emit closeToRight(m_clickedTab);
    }
}

void TabBar::closeToLeft()
{
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Close Tabs"), tr("Do you really want to close all tabs to the left?"),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (button == QMessageBox::Yes) {
        emit closeToLeft(m_clickedTab);
    }
}

QSize TabBar::tabSizeHint(int index, bool fast) const
{
    if (!m_window->isVisible()) {
        // Don't calculate it when window is not visible
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
    switch (sizeType) {
    case ComboTabBar::PinnedTabWidth:
        return iconButtonSize().width() + style()->pixelMetric(QStyle::PM_TabBarTabHSpace, 0, this);

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
        menu.addAction(webTab->isMuted() ? tr("Un&mute Tab") : tr("&Mute Tab"), this, SLOT(muteTab()));
        menu.addSeparator();
        menu.addAction(tr("Re&load All Tabs"), m_tabWidget, SLOT(reloadAllTabs()));
        menu.addAction(tr("Bookmark &All Tabs"), m_window, SLOT(bookmarkAllTabs()));
        menu.addSeparator();
        menu.addAction(tr("Close Ot&her Tabs"), this, SLOT(closeAllButCurrent()));
        menu.addAction(tr("Close Tabs To The Right"), this, SLOT(closeToRight()));
        menu.addAction(tr("Close Tabs To The Left"), this, SLOT(closeToLeft()));
        menu.addSeparator();
        menu.addAction(m_window->action(QSL("Other/RestoreClosedTab")));
        menu.addAction(QIcon::fromTheme("window-close"), tr("Cl&ose Tab"), this, SLOT(closeTab()));
    } else {
        menu.addAction(IconProvider::newTabIcon(), tr("&New tab"), m_window, SLOT(addTab()));
        menu.addSeparator();
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
    m_tabWidget->requestCloseTab(currentIndex());
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
        m_tabWidget->requestCloseTab(tabToClose);
    }
}

void TabBar::currentTabChanged(int index)
{
    if (!validIndex(index)) {
        return;
    }

    // Don't hide close buttons when dragging tabs
    if (m_dragStartPosition.isNull()) {
        showCloseButton(index);
        hideCloseButton(m_tabWidget->lastTabIndex());

        QTimer::singleShot(100, this, [this]() { ensureVisible(); });
    }

    m_tabWidget->currentTabChanged(index);
}

void TabBar::pinTab()
{
    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_clickedTab));

    if (webTab) {
        webTab->togglePinned();
    }
}

void TabBar::muteTab()
{
    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_clickedTab));

    if (webTab) {
        webTab->toggleMuted();
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
        }
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
            m_tabWidget->requestCloseTab(id);
            return;
        }
    }

    ComboTabBar::mouseReleaseEvent(event);
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
