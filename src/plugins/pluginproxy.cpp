/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "pluginproxy.h"
#include "plugininterface.h"
#include "mainapplication.h"
#include "speeddial.h"
#include "settings.h"

PluginProxy::PluginProxy()
    : Plugins()
    , m_speedDial(new SpeedDial(this))
{
    c2f_loadSettings();
}

void PluginProxy::populateWebViewMenu(QMenu* menu, WebView* view, const QWebHitTestResult &r)
{
    if (!menu || !view || m_loadedPlugins.count() == 0) {
        return;
    }

    menu->addSeparator();
    int count = menu->actions().count();

    foreach(PluginInterface * iPlugin, m_loadedPlugins) {
        iPlugin->populateWebViewMenu(menu, view, r);
    }

    if (menu->actions().count() == count) {
        menu->removeAction(menu->actions().at(count - 1));
    }
}

void PluginProxy::c2f_loadSettings()
{
    Settings settings;
    settings.beginGroup("ClickToFlash");
    c2f_whitelist = settings.value("whitelist", QStringList()).toStringList();
    c2f_enabled = settings.value("Enabled", true).toBool();
    settings.endGroup();
}

void PluginProxy::c2f_saveSettings()
{
    Settings settings;
    settings.beginGroup("ClickToFlash");
    settings.setValue("whitelist", c2f_whitelist);
    settings.setValue("Enabled", c2f_enabled);
    settings.endGroup();
}
