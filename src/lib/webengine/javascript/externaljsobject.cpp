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
#include "webpage.h"
#include "searchenginesmanager.h"
#include "autofilljsobject.h"
#include "restoremanager.h"

ExternalJsObject::ExternalJsObject(WebPage *page)
    : QObject(page)
    , m_page(page)
    , m_autoFill(new AutoFillJsObject(this))
{
}

WebPage *ExternalJsObject::page() const
{
    return m_page;
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

QObject *ExternalJsObject::speedDial() const
{
    if (m_page->url().toString() != QL1S("qupzilla:speeddial"))
        return Q_NULLPTR;

    return mApp->plugins()->speedDial();
}

QObject *ExternalJsObject::autoFill() const
{
    return m_autoFill;
}

QObject *ExternalJsObject::recovery() const
{
    if (!mApp->restoreManager() || m_page->url().toString() != QL1S("qupzilla:restore"))
        return Q_NULLPTR;

    return mApp->restoreManager()->recoveryObject(m_page);
}
