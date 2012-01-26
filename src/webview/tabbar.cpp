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
#include "tabbar.h"
#include "tabwidget.h"
#include "qupzilla.h"
#include "webtab.h"
#include "iconprovider.h"
#include "toolbutton.h"
#include "settings.h"

#define MAXIMUM_TAB_WIDTH 250
#define MINIMUM_TAB_WIDTH 50

#ifdef Q_WS_WIN
#define PINNED_TAB_WIDTH 38
#elif defined(KDE)
#define PINNED_TAB_WIDTH 24
#else
#define PINNED_TAB_WIDTH 31
#endif

TabBar::TabBar(QupZilla* mainClass, TabWidget* tabWidget)
    : QTabBar()
    , p_QupZilla(mainClass)
    , m_tabWidget(tabWidget)
    , m_clickedTab(0)
    , m_pinnedTabsCount(0)
    , m_normalTabWidth(0)
    , m_lastTabWidth(0)
    , m_adjustingLastTab(false)
{
    setObjectName("tabbar");
    setContextMenuPolicy(Qt::CustomContextMenu);
    setElideMode(Qt::ElideRight);
    setTabsClosable(true);
    setDocumentMode(true);
    setFocusPolicy(Qt::NoFocus);
    loadSettings();

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));
    connect(m_tabWidget, SIGNAL(pinnedTabClosed()), this, SLOT(pinnedTabClosed()));
    connect(m_tabWidget, SIGNAL(pinnedTabAdded()), this, SLOT(pinnedTabAdded()));
}

void TabBar::loadSettings()
{
    Settings settings;
    settings.beginGroup("Browser-Tabs-Settings");

    setMovable(settings.value("makeTabsMovable", true).toBool());
    if (settings.value("ActivateLastTabWhenClosingActual", false).toBool()) {
        setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    }

    settings.endGroup();
}

void TabBar::updateVisibilityWithFullscreen(bool visible)
{
    if (visible) {
        emit showButtons();
    }
    else {
        emit hideButtons();
    }

    QTabBar::setVisible(visible);
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
        emit hideButtons();
    }

    QTabBar::setVisible(visible);
}

void TabBar::contextMenuRequested(const QPoint &position)
{
    int index = tabAt(position);
    m_clickedTab = index;

    QMenu menu;
    menu.addAction(QIcon(":/icons/menu/popup.png"), tr("&New tab"), p_QupZilla, SLOT(addTab()));
    menu.addSeparator();
    if (index != -1) {
        WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_clickedTab));
        if (!webTab) {
            return;
        }
        if (p_QupZilla->weView(m_clickedTab)->isLoading()) {
            menu.addAction(IconProvider::standardIcon(QStyle::SP_BrowserStop), tr("&Stop Tab"), this, SLOT(stopTab()));
        }
        else {
            menu.addAction(IconProvider::standardIcon(QStyle::SP_BrowserReload), tr("&Reload Tab"), this, SLOT(reloadTab()));
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

    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);
    p_QupZilla->actionRestoreTab()->setEnabled(true);
}

QSize TabBar::tabSizeHint(int index) const
{
    QSize size = QTabBar::tabSizeHint(index);
    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    TabBar* tabBar = const_cast <TabBar*>(this);
    tabBar->m_adjustingLastTab = false;

    if (webTab && webTab->isPinned()) {
        size.setWidth(PINNED_TAB_WIDTH);
    }
    else {
        int availableWidth = width() - (PINNED_TAB_WIDTH * m_pinnedTabsCount) - m_tabWidget->buttonListTabs()->width() - m_tabWidget->buttonAddTab()->width();
        int normalTabsCount = count() - m_pinnedTabsCount;
        if (availableWidth >= MAXIMUM_TAB_WIDTH * normalTabsCount) {
            tabBar->m_normalTabWidth = MAXIMUM_TAB_WIDTH;
            size.setWidth(m_normalTabWidth);
        }
        else if (availableWidth < MINIMUM_TAB_WIDTH * normalTabsCount) {
            tabBar->m_normalTabWidth = MINIMUM_TAB_WIDTH;
            size.setWidth(m_normalTabWidth);
        }
        else {
            int maxWidthForTab = availableWidth / normalTabsCount;
            tabBar->m_normalTabWidth = maxWidthForTab;
            //Fill any empty space (gotten from rounding) with last tab
            if (index == count() - 1) {
                tabBar->m_lastTabWidth = (availableWidth - maxWidthForTab * normalTabsCount) + maxWidthForTab;
                tabBar->m_adjustingLastTab = true;
                size.setWidth(m_lastTabWidth);
            }
            else {
                tabBar->m_lastTabWidth = maxWidthForTab;
                size.setWidth(m_lastTabWidth);
            }
        }
    }

    if (index == count() - 1) {
        int xForAddTabButton = (PINNED_TAB_WIDTH * m_pinnedTabsCount) + (count() - m_pinnedTabsCount) * (m_normalTabWidth);
        if (m_adjustingLastTab) {
            xForAddTabButton += m_lastTabWidth - m_normalTabWidth;
        }
        emit tabBar->moveAddTabButton(xForAddTabButton);
    }

    return size;
}

//void TabBar::emitMoveAddTabButton(int pox)
//{
//    emit moveAddTabButton(pox);
//}

#if 0
void TabBar::tabInserted(int index)
{
//    CloseButton* closeButton = new CloseButton(this);
//    closeButton->setAutoRaise(true);
//    closeButton->setMaximumSize(17,17);
//    closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
//    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeCurrentTab()));
//    setTabButton(index, QTabBar::RightSide, closeButton);
    QAbstractButton* button = (QAbstractButton*)tabButton(index, QTabBar::RightSide);
    if (!button) {
        return;
    }
    button->resize(17, 17);
}

void TabBar::showCloseButton(int index)
{

    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    if (webTab && webTab->isPinned()) {
        return;
    }

    CloseButton* button = (CloseButton*)tabButton(index, QTabBar::RightSide);
    if (!button) {
        return;
    }

    button->show();
}

void TabBar::hideCloseButton(int index)
{
    CloseButton* button = (CloseButton*)tabButton(index, QTabBar::RightSide);
    if (!button) {
        return;
    }
    button->hide();
}
#endif

void TabBar::updateCloseButton(int index)
{
    QAbstractButton* button = qobject_cast<QAbstractButton*>(tabButton(index, QTabBar::RightSide));
    if (!button) {
        return;
    }

    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(index));
    if (webTab && webTab->isPinned()) {
        button->hide();
    }
    else {
        button->show();
    }
}

void TabBar::closeCurrentTab()
{
    int id = currentIndex();
    if (id < 0) {
        return;
    }

    m_tabWidget->closeTab(id);
}

void TabBar::bookmarkTab()
{
    p_QupZilla->addBookmark(p_QupZilla->weView(m_clickedTab)->url(), p_QupZilla->weView(m_clickedTab)->title(), p_QupZilla->weView(m_clickedTab)->icon());
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

    // Adjust add tab button in proper position
    tabSizeHint(count() - 1);
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

void TabBar::mousePressEvent(QMouseEvent* event)
{
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
    if (!m_dragStartPosition.isNull() && m_tabWidget->buttonAddTab()->isVisible()) {
        int manhattanLength = (event->pos() - m_dragStartPosition).manhattanLength();
        if (manhattanLength > QApplication::startDragDistance()) {
            m_tabWidget->buttonAddTab()->hide();
        }
    }

    QTabBar::mouseMoveEvent(event);
}

void TabBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && tabAt(event->pos()) == -1) {
        m_tabWidget->addView(QUrl(), Qz::NT_SelectedTabAtTheEnd, true);
        return;
    }

    QTabBar::mouseDoubleClickEvent(event);
}

void TabBar::mouseReleaseEvent(QMouseEvent* event)
{
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

void TabBar::disconnectObjects()
{
    disconnect(this);
}
