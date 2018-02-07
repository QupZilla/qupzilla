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
#ifndef CLOSEDTABSMANAGER_H
#define CLOSEDTABSMANAGER_H

#include <QVector>

#include "webtab.h"
#include "qzcommon.h"

class WebTab;

class QUPZILLA_EXPORT ClosedTabsManager
{
public:
    struct Tab {
        int position = -1;
        WebTab::SavedTab tabState;

        bool isValid() const {
            return position > -1;
        }
    };

    explicit ClosedTabsManager();

    void saveTab(WebTab *tab);
    bool isClosedTabAvailable() const;

    // Takes tab that was most recently closed
    Tab takeLastClosedTab();
    // Takes tab at given index
    Tab takeTabAt(int index);

    QVector<Tab> closedTabs() const;
    void clearClosedTabs();

private:
    QVector<Tab> m_closedTabs;
};

// Hint to Qt to use std::realloc on item moving
Q_DECLARE_TYPEINFO(ClosedTabsManager::Tab, Q_MOVABLE_TYPE);

#endif // CLOSEDTABSMANAGER_H
