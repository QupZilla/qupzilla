/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2018 David Rosca <nowrep@gmail.com>
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
#include "autofilljsobject.h"
#include "restoremanager.h"

#include <QWebChannel>

static QHash<QString, QObject*> s_extraObjects;

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

// static
void ExternalJsObject::setupWebChannel(QWebChannel *webChannel, WebPage *page)
{
    webChannel->registerObject(QSL("qz_object"), new ExternalJsObject(page));

    for (auto it = s_extraObjects.constBegin(); it != s_extraObjects.constEnd(); ++it) {
        webChannel->registerObject(QSL("qz_") + it.key(), it.value());
    }
}

// static
void ExternalJsObject::registerExtraObject(const QString &id, QObject *object)
{
    s_extraObjects[id] = object;
}

// static
void ExternalJsObject::unregisterExtraObject(const QString &id)
{
    s_extraObjects.remove(id);
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
