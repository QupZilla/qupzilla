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
#include "webhistoryinterface.h"
#include "mainapplication.h"
#include "history.h"

#if QTWEBENGINE_DISABLED

WebHistoryInterface::WebHistoryInterface(QObject* parent)
    : QWebEngineHistoryInterface(parent)
{
}

void WebHistoryInterface::addHistoryEntry(const QString &url)
{
    m_clickedLinks.insert(url);
}

bool WebHistoryInterface::historyContains(const QString &url) const
{
    return m_clickedLinks.find(url) != m_clickedLinks.end();
}

#endif
