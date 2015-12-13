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
#include "closedtabsmanager.h"
#include "webtab.h"
#include "qztools.h"
#include "mainapplication.h"

#include <QWebEngineHistory>
#include <QWebEngineSettings>

ClosedTabsManager::ClosedTabsManager()
{
}

void ClosedTabsManager::saveTab(WebTab* tab, int position)
{
    if (mApp->isPrivate()) {
        return;
    }

    // Don't save empty tab
    if (tab->url().isEmpty() && tab->history()->items().count() == 0) {
        return;
    }

    Tab closedTab;
    closedTab.url = tab->url();
    closedTab.title = tab->title();
    closedTab.icon = tab->icon();
    closedTab.position = position;
    closedTab.history = tab->historyData();
    closedTab.zoomLevel = tab->zoomLevel();

    m_closedTabs.prepend(closedTab);
}

bool ClosedTabsManager::isClosedTabAvailable()
{
    return !m_closedTabs.isEmpty();
}

ClosedTabsManager::Tab ClosedTabsManager::takeLastClosedTab()
{
    Tab tab;
    tab.position = -1;

    if (m_closedTabs.count() > 0) {
        tab = m_closedTabs.takeFirst();
    }

    return tab;
}

ClosedTabsManager::Tab ClosedTabsManager::takeTabAt(int index)
{
    Tab tab;
    tab.position = -1;

    QLinkedList<Tab>::iterator it;
    int i = 0;

    for (it = m_closedTabs.begin(); it != m_closedTabs.end(); ++it, ++i) {
        if (i == index) {
            tab = *it;
            m_closedTabs.erase(it);
            break;
        }
    }

    return tab;
}

QLinkedList<ClosedTabsManager::Tab> ClosedTabsManager::allClosedTabs()
{
    return m_closedTabs;
}

void ClosedTabsManager::clearList()
{
    m_closedTabs.clear();
}
