/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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

#include "adblockplugin.h"
#include "adblockmanager.h"
#include "adblockicon.h"

#include "scripts.h"
#include "webpage.h"
#include "pluginproxy.h"
#include "browserwindow.h"
#include "navigationbar.h"
#include "mainapplication.h"

AdBlockPlugin::AdBlockPlugin(QObject *parent)
    : QObject(parent)
{
    connect(mApp, &MainApplication::aboutToQuit, AdBlockManager::instance(), &AdBlockManager::save);
    connect(mApp->plugins(), &PluginProxy::webPageCreated, this, &AdBlockPlugin::webPageCreated);
    connect(mApp->plugins(), &PluginProxy::webPageDeleted, this, &AdBlockPlugin::webPageDeleted);
    connect(mApp->plugins(), &PluginProxy::mainWindowCreated, this, &AdBlockPlugin::mainWindowCreated);
}

void AdBlockPlugin::webPageCreated(WebPage *page)
{
    connect(page, &WebPage::loadFinished, this, [=]() {
        AdBlockManager *manager = AdBlockManager::instance();
        if (!manager->isEnabled()) {
            return;
        }
        // Apply global element hiding rules
        const QString elementHiding = manager->elementHidingRules(page->url());
        if (!elementHiding.isEmpty()) {
            page->runJavaScript(Scripts::setCss(elementHiding), WebPage::SafeJsWorld);
        }
        // Apply domain-specific element hiding rules
        const QString siteElementHiding = manager->elementHidingRulesForDomain(page->url());
        if (!siteElementHiding.isEmpty()) {
            page->runJavaScript(Scripts::setCss(siteElementHiding), WebPage::SafeJsWorld);
        }
    });
}

void AdBlockPlugin::webPageDeleted(WebPage *page)
{
    AdBlockManager::instance()->clearBlockedRequestsForUrl(page->url());
}

void AdBlockPlugin::mainWindowCreated(BrowserWindow *window)
{
    window->navigationBar()->addToolButton(new AdBlockIcon(window));
}
