/* ============================================================
* QupZilla - QtWebEngine based browser
* Copyright (C) 2015-2016 David Rosca <nowrep@gmail.com>
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
#include "gm_urlinterceptor.h"
#include "gm_manager.h"

GM_UrlInterceptor::GM_UrlInterceptor(GM_Manager *manager)
    : UrlInterceptor(manager)
    , m_manager(manager)
{
}

void GM_UrlInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    if (info.navigationType() != QWebEngineUrlRequestInfo::NavigationTypeLink)
        return;

    if (info.requestUrl().toString().endsWith(QLatin1String(".user.js"))) {
        m_manager->downloadScript(info.requestUrl());
        info.block(true);
    }
}

