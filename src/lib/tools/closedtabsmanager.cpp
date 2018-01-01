/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "mainapplication.h"
#include "qztools.h"

#include <QWebEngineHistory>

ClosedTabsManager::ClosedTabsManager()
{
}

void ClosedTabsManager::saveTab(WebTab *tab)
{
    if (mApp->isPrivate()) {
        return;
    }

    // Don't save empty tab
    if (tab->url().isEmpty() && tab->history()->items().count() == 0) {
        return;
    }

    Tab closedTab;
    closedTab.position = tab->tabIndex();
    closedTab.tabState = WebTab::SavedTab(tab);
    m_closedTabs.prepend(closedTab);
}

bool ClosedTabsManager::isClosedTabAvailable() const
{
    return !m_closedTabs.isEmpty();
}

ClosedTabsManager::Tab ClosedTabsManager::takeLastClosedTab()
{
    Tab tab;
    if (!m_closedTabs.isEmpty()) {
        tab = m_closedTabs.takeFirst();
    }
    return tab;
}

ClosedTabsManager::Tab ClosedTabsManager::takeTabAt(int index)
{
    Tab tab;
    if (QzTools::containsIndex(m_closedTabs, index)) {
        tab = m_closedTabs.takeAt(index);
    }
    return tab;
}

QVector<ClosedTabsManager::Tab> ClosedTabsManager::closedTabs() const
{
    return m_closedTabs;
}

void ClosedTabsManager::clearClosedTabs()
{
    m_closedTabs.clear();
}
