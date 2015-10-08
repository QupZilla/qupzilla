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

#ifndef GM_URLINTERCEPTOR_H
#define GM_URLINTERCEPTOR_H

#include "urlinterceptor.h"

class GM_Manager;

class GM_UrlInterceptor : public UrlInterceptor
{
public:
    explicit GM_UrlInterceptor(GM_Manager* manager);

    bool interceptRequest(QWebEngineUrlRequestInfo &info);

private:
    GM_Manager *m_manager;

};

#endif // GM_URLINTERCEPTOR_H
