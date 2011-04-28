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
#include "sidebar.h"
#include "docktitlebarwidget.h"
#include "bookmarkssidebar.h"
#include "historysidebar.h"
#include "qupzilla.h"

SideBar::SideBar(QWidget* parent) :
    QDockWidget(parent)
   ,m_activeWidget(None)
{
    setObjectName("SideBar");
    setWindowTitle(tr("SideBar"));
    setAttribute(Qt::WA_DeleteOnClose);
    m_titleBar = new DockTitleBarWidget("", this);
    setTitleBarWidget(m_titleBar);
    setFeatures(0);
}

void SideBar::showBookmarks()
{
    m_titleBar->setTitle(tr("Bookmarks"));
    BookmarksSideBar* bar = new BookmarksSideBar((QupZilla*)parentWidget(), this);
    setWidget(bar);
    m_activeWidget = Bookmarks;

    QSettings settings(mApp->getActiveProfil() + "settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/SideBar", "Bookmarks");
}

void SideBar::showHistory()
{
    m_titleBar->setTitle(tr("History"));
    HistorySideBar* bar = new HistorySideBar((QupZilla*)parentWidget(), this);
    setWidget(bar);
    m_activeWidget = History;

    QSettings settings(mApp->getActiveProfil() + "settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/SideBar", "History");
}

void SideBar::showRSS()
{

}

void SideBar::close()
{
    QSettings settings(mApp->getActiveProfil() + "settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/SideBar", "None");

    QDockWidget::close();
}
