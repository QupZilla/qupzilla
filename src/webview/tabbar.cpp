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

TabBar::TabBar(QupZilla* mainClass, QWidget* parent) :
    QTabBar(parent)
    ,p_QupZilla(mainClass)
    ,m_clickedTab(0)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setObjectName("tabBar");
    setTabsClosable(true);
    setElideMode(Qt::ElideRight);
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
    TabWidget* tabWidget = qobject_cast<TabWidget*>(parentWidget());
    if (!tabWidget)
        return;
    int index = tabAt(position);
    m_clickedTab = index;

    QMenu menu;
    menu.addAction(QIcon(":/icons/menu/popup.png"),tr("New tab"), p_QupZilla, SLOT(addTab()));
    menu.addSeparator();

    if (index!=-1) {
        menu.addAction(
#ifdef Q_WS_X11
                style()->standardIcon(QStyle::SP_ArrowBack)
#else
                QIcon(":/icons/faenza/back.png")
#endif
                       ,tr("Back"), this, SLOT(backTab()));
        menu.addAction(
#ifdef Q_WS_X11
                style()->standardIcon(QStyle::SP_ArrowForward)
#else
                QIcon(":/icons/faenza/forward.png")
#endif
                ,tr("Forward"), this, SLOT(forwardTab()));
        menu.addAction(
#ifdef Q_WS_X11
                style()->standardIcon(QStyle::SP_BrowserStop)
#else
                QIcon(":/icons/faenza/stop.png")
#endif
                ,tr("Stop Tab"), this, SLOT(stopTab()));
        menu.addAction(
#ifdef Q_WS_X11
                style()->standardIcon(QStyle::SP_BrowserReload)
#else
                QIcon(":/icons/faenza/reload.png")
#endif
                ,tr("Reload Tab"), this, SLOT(reloadTab()));
        menu.addAction(tr("Reload All Tabs"), tabWidget, SLOT(reloadAllTabs()));
        menu.addAction(tr("Bookmark This Tab"), this, SLOT(bookmarkTab()));
        menu.addAction(tr("Bookmark All Tabs"), p_QupZilla, SLOT(bookmarkAllTabs()));
        menu.addSeparator();
        QAction* action = menu.addAction(QIcon::fromTheme("user-trash"),tr("Restore Closed Tab"), tabWidget, SLOT(restoreClosedTab()));
        tabWidget->canRestoreTab() ? action->setEnabled(true) : action->setEnabled(false);
        menu.addSeparator();
        menu.addAction(tr("Close Other Tabs"), this, SLOT(closeAllButCurrent()));
        menu.addAction(QIcon::fromTheme("window-close"),tr("Close"), this, SLOT(closeTab()));
        menu.addSeparator();

        if (!p_QupZilla->weView(m_clickedTab)->history()->canGoBack())
            menu.actions().at(2)->setEnabled(false);

        if (!p_QupZilla->weView(m_clickedTab)->history()->canGoForward())
            menu.actions().at(3)->setEnabled(false);

        if (!p_QupZilla->weView(m_clickedTab)->isLoading())
            menu.actions().at(4)->setEnabled(false);
    }else{
        menu.addAction(tr("Reload All Tabs"), tabWidget, SLOT(reloadAllTabs()));
        menu.addAction(tr("Bookmark All Tabs"), p_QupZilla, SLOT(bookmarkAllTabs()));
        menu.addSeparator();
        QAction* action = menu.addAction(QIcon::fromTheme("user-trash"),tr("Restore Closed Tab"), tabWidget, SLOT(restoreClosedTab()));
        tabWidget->canRestoreTab() ? action->setEnabled(true) : action->setEnabled(false);
    }

    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y()+1);
    menu.exec(p);
}

void TabBar::bookmarkTab()
{
    p_QupZilla->addBookmark(p_QupZilla->weView(m_clickedTab)->url(), p_QupZilla->weView(m_clickedTab)->title());
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
