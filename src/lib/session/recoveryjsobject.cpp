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
    m_page->load(static_cast<TabbedWebView*>(m_page->view())->browserWindow()->homepageUrl());

    mApp->destroyRestoreManager();
}

void RecoveryJsObject::restoreSession()
{
    if (!mApp->restoreSession(static_cast<TabbedWebView*>(m_page->view())->browserWindow() , m_manager->restoreData())) {
        startNewSession();
    }
}
