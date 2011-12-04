/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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

SideBar::SideBar(QupZilla* mainClass, QWidget* parent)
    : QWidget(parent)
    , p_QupZilla(mainClass)
    , m_activeWidget(None)
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

void SideBar::showBookmarks()
{
    m_titleBar->setTitle(tr("Bookmarks"));
    BookmarksSideBar* bar = new BookmarksSideBar(p_QupZilla);
    setWidget(bar);
    m_activeWidget = Bookmarks;

    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/SideBar", "Bookmarks");
}

void SideBar::showHistory()
{
    m_titleBar->setTitle(tr("History"));
    HistorySideBar* bar = new HistorySideBar(p_QupZilla);
    setWidget(bar);
    m_activeWidget = History;

    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/SideBar", "History");
}

void SideBar::showRSS()
{

}

void SideBar::setWidget(QWidget* widget)
{
    if (m_layout->count() == 2) {
        delete m_layout->itemAt(1)->widget();
    }

    m_layout->addWidget(widget);
}

void SideBar::close()
{
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.setValue("Browser-View-Settings/SideBar", "None");

    p_QupZilla->saveSideBarWidth();

    QWidget::close();
}

SideBar::~SideBar()
{
}
