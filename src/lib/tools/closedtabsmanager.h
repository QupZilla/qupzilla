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
#ifndef CLOSEDTABSMANAGER_H
#define CLOSEDTABSMANAGER_H

#include <QObject>
#include <QUrl>
#include <QWebHistory>

class WebView;
class ClosedTabsManager : public QObject
{
public:
    explicit ClosedTabsManager(QObject* parent = 0);
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

    void saveView(WebView* view, int position);
    ClosedTabsManager::Tab getFirstClosedTab();
    ClosedTabsManager::Tab getTabAt(int index);

    bool isClosedTabAvailable();
    void clearList();

    QList<ClosedTabsManager::Tab> allClosedTabs() { return m_closedTabs; }

private:
    QList<ClosedTabsManager::Tab> m_closedTabs;

};

#endif // CLOSEDTABSMANAGER_H
