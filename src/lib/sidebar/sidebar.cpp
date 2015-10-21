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
#include "sidebar.h"
#include "sidebarinterface.h"
#include "docktitlebarwidget.h"
#include "bookmarkssidebar.h"
#include "historysidebar.h"
#include "mainapplication.h"
#include "browserwindow.h"
#include "settings.h"

#include <QMenu>

QHash<QString, QPointer<SideBarInterface> > SideBarManager::s_sidebars;

SideBar::SideBar(SideBarManager* manager, BrowserWindow* window)
    : QWidget(window)
    , m_window(window)
    , m_manager(manager)
{
    setObjectName("sidebar");
    setAttribute(Qt::WA_DeleteOnClose);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    m_titleBar = new DockTitleBarWidget(QString(), this);
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
    BookmarksSidebar* bar = new BookmarksSidebar(m_window);
    setWidget(bar);
}

void SideBar::showHistory()
{
    m_titleBar->setTitle(tr("History"));
    HistorySideBar* bar = new HistorySideBar(m_window);
    setWidget(bar);
}

void SideBar::close()
{
    m_manager->closeSideBar();

    QWidget* p = parentWidget();
    if (p) {
        p->setFocus();
    }

    QWidget::close();
}

SideBarManager::SideBarManager(BrowserWindow* parent)
    : QObject(parent)
    , m_window(parent)
{
}

void SideBarManager::createMenu(QMenu* menu)
{
    m_window->removeActions(menu->actions());
    menu->clear();

    QAction* act = menu->addAction(SideBar::tr("Bookmarks"), this, SLOT(slotShowSideBar()));
    act->setCheckable(true);
    act->setShortcut(QKeySequence("Ctrl+Shift+B"));
    act->setData("Bookmarks");
    act->setChecked(m_activeBar == QL1S("Bookmarks"));

    act = menu->addAction(SideBar::tr("History"), this, SLOT(slotShowSideBar()));
    act->setCheckable(true);
    act->setShortcut(QKeySequence("Ctrl+H"));
    act->setData("History");
    act->setChecked(m_activeBar == QL1S("History"));

    foreach (const QPointer<SideBarInterface> &sidebar, s_sidebars) {
        if (sidebar) {
            QAction* act = sidebar.data()->createMenuAction();
            act->setData(s_sidebars.key(sidebar));
            act->setChecked(m_activeBar == s_sidebars.key(sidebar));
            connect(act, SIGNAL(triggered()), this, SLOT(slotShowSideBar()));
            menu->addAction(act);
        }
    }

    m_window->addActions(menu->actions());
}

void SideBarManager::addSidebar(const QString &id, SideBarInterface* interface)
{
    s_sidebars[id] = interface;
}

void SideBarManager::removeSidebar(const QString &id)
{
    s_sidebars.remove(id);

    foreach (BrowserWindow* window, mApp->windows()) {
        window->sideBarManager()->sideBarRemoved(id);
    }
}

void SideBarManager::slotShowSideBar()
{
    if (QAction* act = qobject_cast<QAction*>(sender())) {
        showSideBar(act->data().toString());
    }
}

void SideBarManager::showSideBar(const QString &id, bool toggle)
{
    if (id == QLatin1String("None")) {
        return;
    }

    if (!m_sideBar) {
        m_sideBar = m_window->addSideBar();
    }

    if (id == m_activeBar) {
        if (!toggle) {
            return;
        }
        m_sideBar.data()->close();
        m_activeBar = "None";

        Settings settings;
        settings.setValue("Browser-View-Settings/SideBar", m_activeBar);
        return;
    }

    if (id == QLatin1String("Bookmarks")) {
        m_sideBar.data()->showBookmarks();
    }
    else if (id == QLatin1String("History")) {
        m_sideBar.data()->showHistory();
    }
    else {
        SideBarInterface* sidebar = s_sidebars[id].data();
        if (!sidebar) {
            m_sideBar.data()->close();
            return;
        }

        m_sideBar.data()->setTitle(sidebar->title());
        m_sideBar.data()->setWidget(sidebar->createSideBarWidget(m_window));
    }

    m_activeBar = id;

    Settings settings;
    settings.setValue("Browser-View-Settings/SideBar", m_activeBar);
}

void SideBarManager::sideBarRemoved(const QString &id)
{
    if (m_activeBar == id && m_sideBar) {
        m_sideBar.data()->setWidget(0);
        m_sideBar.data()->close();
    }
}

void SideBarManager::closeSideBar()
{
    if (mApp->isClosing()) {
        return;
    }
    m_activeBar = "None";

    Settings settings;
    settings.setValue("Browser-View-Settings/SideBar", m_activeBar);

    m_window->saveSideBarWidth();
}
