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

void RecoveryJsObject::startNewSession()
{
    BrowserWindow *window = getBrowserWindow();
    if (!window)
        return;

    m_page->load(window->homepageUrl());

    mApp->destroyRestoreManager();
}

void RecoveryJsObject::restoreSession()
{
    BrowserWindow *window = getBrowserWindow();
    if (!window)
        return;

    bool ok = mApp->restoreSession(window , m_manager->restoreData());

    if (!ok)
        startNewSession();
}

BrowserWindow *RecoveryJsObject::getBrowserWindow() const
{
    TabbedWebView *view = qobject_cast<TabbedWebView*>(m_page->view());
    return view ? view->browserWindow() : Q_NULLPTR;
}
