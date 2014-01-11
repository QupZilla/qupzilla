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
#ifndef CLOSEDTABSMANAGER_H
#define CLOSEDTABSMANAGER_H

#include <QUrl>
#include <QVector>

#include "qz_namespace.h"

class WebTab;

class QT_QUPZILLA_EXPORT ClosedTabsManager
{
public:
    struct Tab {
        QUrl url;
        QByteArray history;
        QString title;
        int position;

        bool operator==(const Tab &a) const {
            return (a.url == url &&
                    a.history == history &&
                    a.position == position);
        }
    };

    explicit ClosedTabsManager();

    void saveView(WebTab* tab, int position);
    ClosedTabsManager::Tab getFirstClosedTab();
    ClosedTabsManager::Tab getTabAt(int index);

    bool isClosedTabAvailable();
    void clearList();

    QVector<ClosedTabsManager::Tab> allClosedTabs();

private:
    QVector<ClosedTabsManager::Tab> m_closedTabs;

};

// Hint to QVector to use std::realloc on item moving
Q_DECLARE_TYPEINFO(ClosedTabsManager::Tab, Q_MOVABLE_TYPE);

#endif // CLOSEDTABSMANAGER_H
