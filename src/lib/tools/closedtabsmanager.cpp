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
#include "closedtabsmanager.h"
#include "webtab.h"
#include "mainapplication.h"

#include <QWebHistory>
#include <QWebSettings>

ClosedTabsManager::ClosedTabsManager()
{
}

void ClosedTabsManager::saveView(WebTab* tab, int position)
{
    if (mApp->isPrivateSession() || (tab->url().isEmpty() && tab->history()->items().count() == 0)) {
        return;
    }

    Tab closedTab;
    closedTab.url = tab->url();
    closedTab.title = tab->title();
    closedTab.position = position;
    closedTab.history = tab->historyData();

    m_closedTabs.prepend(closedTab);
}

ClosedTabsManager::Tab ClosedTabsManager::getFirstClosedTab()
{
    Tab tab;
    if (m_closedTabs.count() > 0) {
        tab = m_closedTabs.takeFirst();
    }

    return tab;
}

ClosedTabsManager::Tab ClosedTabsManager::getTabAt(int index)
{
    Tab tab;
    if (m_closedTabs.count() > 0 && m_closedTabs.count() > index) {
        tab = m_closedTabs.takeAt(index);
    }

    return tab;
}

bool ClosedTabsManager::isClosedTabAvailable()
{
    return (m_closedTabs.count() != 0);
}

void ClosedTabsManager::clearList()
{
    m_closedTabs.clear();
}
