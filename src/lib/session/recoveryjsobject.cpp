/* ============================================================
* QupZilla - QtWebEngine based browser
* Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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

#include "recoveryjsobject.h"
#include "mainapplication.h"
#include "webpage.h"
#include "tabbedwebview.h"
#include "browserwindow.h"
#include "qztools.h"

#include <QJsonObject>

RecoveryJsObject::RecoveryJsObject(RestoreManager *manager)
    : QObject()
    , m_manager(manager)
    , m_page(Q_NULLPTR)
{
}

void RecoveryJsObject::setPage(WebPage *page)
{
    Q_ASSERT(page);

    m_page = page;
}

QJsonArray RecoveryJsObject::restoreData() const
{
    QJsonArray out;

    int i = 0;
    Q_FOREACH (const RestoreManager::WindowData &w, m_manager->restoreData()) {
        int j = 0;
        QJsonArray tabs;
        Q_FOREACH (const WebTab::SavedTab &t, w.tabsState) {
            QJsonObject tab;
            tab[QSL("tab")] = j++;
            tab[QSL("icon")] = QString(QSL("data:image/png;base64,") + QzTools::pixmapToByteArray(t.icon.pixmap(16)));
            tab[QSL("title")] = t.title;
            tabs.append(tab);
        }

        QJsonObject window;
        window[QSL("window")] = i++;
        window[QSL("tabs")] = tabs;
        out.append(window);
    }

    return out;
}

void RecoveryJsObject::startNewSession()
{
    BrowserWindow *window = getBrowserWindow();
    if (!window)
        return;

    m_page->load(window->homepageUrl());

    mApp->destroyRestoreManager();
}

void RecoveryJsObject::restoreSession(const QStringList &excludeWin, const QStringList &excludeTab)
{
    Q_ASSERT(excludeWin.size() == excludeTab.size());

    RestoreData data = m_manager->restoreData();

    for (int i = 0; i < excludeWin.size(); ++i) {
        int win = excludeWin.at(i).toInt();
        int tab = excludeTab.at(i).toInt();

        if (!QzTools::containsIndex(data, win) || !QzTools::containsIndex(data.at(win).tabsState, tab))
            continue;

        RestoreManager::WindowData &wd = data[win];

        wd.tabsState.remove(tab);
        if (wd.currentTab >= tab)
            --wd.currentTab;

        if (wd.tabsState.isEmpty()) {
            data.remove(win);
            continue;
        }

        if (wd.currentTab < 0)
            wd.currentTab = wd.tabsState.size() - 1;
    }

    BrowserWindow *window = getBrowserWindow();
    if (!window)
        return;

    if (!mApp->restoreSession(window , data))
        startNewSession();
}

BrowserWindow *RecoveryJsObject::getBrowserWindow() const
{
    TabbedWebView *view = qobject_cast<TabbedWebView*>(m_page->view());
    return view ? view->browserWindow() : Q_NULLPTR;
}
