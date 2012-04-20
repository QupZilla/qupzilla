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
#include "sidebar.h"
#include "sidebarinterface.h"
#include "docktitlebarwidget.h"
#include "bookmarkssidebar.h"
#include "historysidebar.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "settings.h"

#include <QMenu>

QHash<QString, QWeakPointer<SideBarInterface> > SideBarManager::s_sidebars;

SideBar::SideBar(SideBarManager* manager, QupZilla* mainClass)
    : QWidget(mainClass)
    , p_QupZilla(mainClass)
    , m_manager(manager)
{
    setObjectName("sidebar");
    setAttribute(Qt::WA_DeleteOnClose);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    m_titleBar = new DockTitleBarWidget("", this);
    m_layout->addWidget(m_titleBar);
}

void SideBar::setTitle(const QString &title)
{
    m_titleBar->setTitle(title);
}

void SideBar::setWidget(QWidget* widget)
{
    if (m_layout->count() == 2) {
        delete m_layout->itemAt(1)->widget();
    }

    m_layout->addWidget(widget);
}

void SideBar::showBookmarks()
{
    m_titleBar->setTitle(tr("Bookmarks"));
    BookmarksSideBar* bar = new BookmarksSideBar(p_QupZilla);
    setWidget(bar);
}

void SideBar::showHistory()
{
    m_titleBar->setTitle(tr("History"));
    HistorySideBar* bar = new HistorySideBar(p_QupZilla);
    setWidget(bar);
}

void SideBar::close()
{
    m_manager->closeSideBar();

    QWidget::close();
}

SideBarManager::SideBarManager(QupZilla* parent)
    : QObject(parent)
    , p_QupZilla(parent)
    , m_menu(0)
{
}

void SideBarManager::setSideBarMenu(QMenu* menu)
{
    m_menu = menu;

    refreshMenu();
}

void SideBarManager::addSidebar(const QString &id, SideBarInterface* interface)
{
    s_sidebars[id] = interface;

    foreach(QupZilla * window, mApp->mainWindows()) {
        window->sideBarManager()->refreshMenu();
    }
}

void SideBarManager::removeSidebar(const QString &id)
{
    s_sidebars.remove(id);

    foreach(QupZilla * window, mApp->mainWindows()) {
        window->sideBarManager()->sideBarRemoved(id);
    }
}

void SideBarManager::refreshMenu()
{
    if (!m_menu) {
        return;
    }

    m_menu->clear();
    QAction* act = m_menu->addAction(SideBar::tr("Bookmarks"), this, SLOT(slotShowSideBar()));
    act->setCheckable(true);
    act->setShortcut(QKeySequence("Ctrl+B"));
    act->setData("Bookmarks");

    act = m_menu->addAction(SideBar::tr("History"), this, SLOT(slotShowSideBar()));
    act->setCheckable(true);
    act->setShortcut(QKeySequence("Ctrl+H"));
    act->setData("History");

    foreach(const QWeakPointer<SideBarInterface> &sidebar, s_sidebars) {
        if (!sidebar) {
            continue;
        }

        QAction* act = sidebar.data()->createMenuAction();
        act->setData(s_sidebars.key(sidebar));
        connect(act, SIGNAL(triggered()), this, SLOT(slotShowSideBar()));

        m_menu->addAction(act);
    }

    updateActions();
}

void SideBarManager::slotShowSideBar()
{
    if (QAction* act = qobject_cast<QAction*>(sender())) {
        showSideBar(act->data().toString());
    }
}

void SideBarManager::updateActions()
{
    if (!m_menu) {
        return;
    }

    foreach(QAction * act, m_menu->actions()) {
        act->setChecked(act->data().toString() == m_activeBar);
    }
}

void SideBarManager::showSideBar(const QString &id)
{
    if (id == "None") {
        return;
    }

    if (!m_sideBar) {
        m_sideBar = p_QupZilla->addSideBar();
    }

    if (id == m_activeBar) {
        m_sideBar.data()->close();

        m_activeBar = "None";
        return;
    }

    if (id == "Bookmarks") {
        m_sideBar.data()->showBookmarks();
    }
    else if (id == "History") {
        m_sideBar.data()->showHistory();
    }
    else {
        SideBarInterface* sidebar = s_sidebars[id].data();
        if (!sidebar) {
            m_sideBar.data()->close();
            return;
        }

        m_sideBar.data()->setTitle(sidebar->title());
        m_sideBar.data()->setWidget(sidebar->createSideBarWidget(p_QupZilla));
    }

    m_activeBar = id;

    Settings settings;
    settings.setValue("Browser-View-Settings/SideBar", id);

    updateActions();
}

void SideBarManager::sideBarRemoved(const QString &id)
{
    if (m_activeBar == id && m_sideBar) {
        m_sideBar.data()->close();
    }

    refreshMenu();
}

void SideBarManager::closeSideBar()
{
    m_activeBar = "None";

    Settings settings;
    settings.setValue("Browser-View-Settings/SideBar", m_activeBar);

    p_QupZilla->saveSideBarWidth();

    updateActions();
}
