/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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

TabBar::TabBar(QupZilla* mainClass, QWidget* parent) :
    QTabBar(parent)
    ,p_QupZilla(mainClass)
    ,m_clickedTab(0)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setObjectName("tabBar");
    setElideMode(Qt::ElideRight);
    setTabsClosable(true);
    setDocumentMode(true);
    loadSettings();

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));
}

void TabBar::loadSettings()
{
    QSettings settings(mApp->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Browser-Tabs-Settings");

    setMovable( settings.value("makeTabsMovable",true).toBool() );
    if (settings.value("ActivateLastTabWhenClosingActual", false).toBool())
        setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);

    settings.endGroup();
}

void TabBar::contextMenuRequested(const QPoint &position)
{
    int index = tabAt(position);
    m_clickedTab = index;

    TabWidget* tabWidget = qobject_cast<TabWidget*>(parentWidget());
    if (!tabWidget)
        return;

    QMenu menu;
    menu.addAction(QIcon(":/icons/menu/popup.png"),tr("&New tab"), p_QupZilla, SLOT(addTab()));
    menu.addSeparator();
    if (index!=-1) {
        WebTab* webTab = qobject_cast<WebTab*>(tabWidget->widget(m_clickedTab));
        if (!webTab)
            return;
        if (p_QupZilla->weView(m_clickedTab)->isLoading()) {
            menu.addAction(
#ifdef Q_WS_X11
                style()->standardIcon(QStyle::SP_BrowserStop)
#else
                QIcon(":/icons/faenza/stop.png")
#endif
                ,tr("&Stop Tab"), this, SLOT(stopTab()));
        } else {
            menu.addAction(
#ifdef Q_WS_X11
                style()->standardIcon(QStyle::SP_BrowserReload)
#else
                QIcon(":/icons/faenza/reload.png")
#endif
                ,tr("&Reload Tab"), this, SLOT(reloadTab()));
        }
        menu.addAction(tr("&Duplicate Tab"), this, SLOT(duplicateTab()));
        menu.addAction(webTab->isPinned() ? tr("Un&pin Tab") : tr("&Pin Tab"), this, SLOT(pinTab()));
        menu.addSeparator();
        menu.addAction(tr("Re&load All Tabs"), tabWidget, SLOT(reloadAllTabs()));
        menu.addAction(tr("&Bookmark This Tab"), this, SLOT(bookmarkTab()));
        menu.addAction(tr("Bookmark &All Tabs"), p_QupZilla, SLOT(bookmarkAllTabs()));
        menu.addSeparator();
        QAction* action = p_QupZilla->actionRestoreTab();
        tabWidget->canRestoreTab() ? action->setEnabled(true) : action->setEnabled(false);
        menu.addAction(action);
        menu.addSeparator();
        menu.addAction(tr("Close Ot&her Tabs"), this, SLOT(closeAllButCurrent()));
        menu.addAction(QIcon::fromTheme("window-close"),tr("Cl&ose"), this, SLOT(closeTab()));
        menu.addSeparator();
    } else {
        menu.addAction(tr("Reloa&d All Tabs"), tabWidget, SLOT(reloadAllTabs()));
        menu.addAction(tr("Bookmark &All Ta&bs"), p_QupZilla, SLOT(bookmarkAllTabs()));
        menu.addSeparator();
        QAction* action = menu.addAction(QIcon::fromTheme("user-trash"),tr("Restore &Closed Tab"), tabWidget, SLOT(restoreClosedTab()));
        tabWidget->canRestoreTab() ? action->setEnabled(true) : action->setEnabled(false);
    }

    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y()+1);
    menu.exec(p);
    p_QupZilla->actionRestoreTab()->setEnabled(true);
}

QSize TabBar::tabSizeHint(int index) const
{
    QSize size = QTabBar::tabSizeHint(index);
    TabWidget* tabWidget = qobject_cast<TabWidget*>(parentWidget());
    if (tabWidget) {
        WebTab* webTab = qobject_cast<WebTab*>(tabWidget->widget(index));
        if (webTab && webTab->isPinned())
#ifdef Q_WS_WIN
            size.setWidth(38);
#else
            size.setWidth(31);
#endif
    }
    return size;
}

#ifdef Q_WS_X11
void TabBar::tabInserted(int index)
{
//    CloseButton* closeButton = new CloseButton(this);
//    closeButton->setAutoRaise(true);
//    closeButton->setMaximumSize(17,17);
//    closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
//    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeCurrentTab()));
//    setTabButton(index, QTabBar::RightSide, closeButton);
    QAbstractButton* button = (QAbstractButton*)tabButton(index, QTabBar::RightSide);
    if (!button)
        return;
    button->setMaximumSize(17,17);
}
#endif

//void TabBar::showCloseButton(int index)
//{
//    TabWidget* tabWidget = qobject_cast<TabWidget*>(parentWidget());
//    if (!tabWidget)
//        return;

//    WebTab* webTab = qobject_cast<WebTab*>(tabWidget->widget(index));
//    if (webTab && webTab->isPinned())
//        return;

//    CloseButton* button = (CloseButton*)tabButton(index, QTabBar::RightSide);
//    if (!button)
//        return;

//    button->show();
//}

//void TabBar::hideCloseButton(int index)
//{
//    CloseButton* button = (CloseButton*)tabButton(index, QTabBar::RightSide);
//    if (!button)
//        return;
//    button->hide();
//}

void TabBar::updateCloseButton(int index)
{
    QAbstractButton* button = (QAbstractButton*)tabButton(index, QTabBar::RightSide);
    if (!button)
        return;

    TabWidget* tabWidget = qobject_cast<TabWidget*>(parentWidget());
    if (!tabWidget)
        return;

    WebTab* webTab = qobject_cast<WebTab*>(tabWidget->widget(index));
    if (webTab && webTab->isPinned())
        button->hide();
    else
        button->show();
}

void TabBar::closeCurrentTab()
{
    int id = currentIndex();
    TabWidget* tabWidget = qobject_cast<TabWidget*>(parentWidget());
    if (!tabWidget || id < 0)
        return;
    tabWidget->closeTab(id);
}

void TabBar::bookmarkTab()
{
    p_QupZilla->addBookmark(p_QupZilla->weView(m_clickedTab)->url(), p_QupZilla->weView(m_clickedTab)->title());
}

void TabBar::pinTab()
{
    TabWidget* tabWidget = qobject_cast<TabWidget*>(parentWidget());
    if (!tabWidget)
        return;

    WebTab* webTab = qobject_cast<WebTab*>(tabWidget->widget(m_clickedTab));
    if (!webTab)
        return;

    webTab->pinTab(m_clickedTab);
}

void TabBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    TabWidget* tabWidget = qobject_cast<TabWidget*>(parentWidget());
    if (!tabWidget)
        return;
    if (event->button() == Qt::LeftButton && tabAt(event->pos()) == -1) {
        tabWidget->addView(QUrl(),tr("New tab"), TabWidget::NewTab, true);
        return;
    }
    QTabBar::mouseDoubleClickEvent(event);
}
