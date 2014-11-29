/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#include "externaljsobject.h"
#include "mainapplication.h"
#include "pluginproxy.h"
#include "speeddial.h"
#include "searchenginesmanager.h"

ExternalJsObject::ExternalJsObject(QObject* parent)
    : QObject(parent)
    , m_onSpeedDial(false)
{
}

void ExternalJsObject::AddSearchProvider(const QString &engineUrl)
{
    mApp->searchEnginesManager()->addEngine(QUrl(engineUrl));
}

int ExternalJsObject::IsSearchProviderInstalled(const QString &engineURL)
{
    qDebug() << "NOT IMPLEMENTED: IsSearchProviderInstalled()" << engineURL;
    return 0;
}

QObject* ExternalJsObject::speedDial() const
{
    return m_onSpeedDial ? mApp->plugins()->speedDial() : 0;
}

void ExternalJsObject::setOnSpeedDial(bool on)
{
    m_onSpeedDial = on;
}
