/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "qzsettings.h"
#include "settings.h"

QzSettings::QzSettings()
{
    loadSettings();
}

void QzSettings::loadSettings()
{
    Settings settings;
    settings.beginGroup("AddressBar");
    selectAllOnDoubleClick = settings.value("SelectAllTextOnDoubleClick", true).toBool();
    selectAllOnClick = settings.value("SelectAllTextOnClick", false).toBool();
    addCountryWithAlt = settings.value("AddCountryDomainWithAltKey", true).toBool();
    showLoadingProgress = settings.value("ShowLoadingProgress", false).toBool();
    showLocationSuggestions = settings.value("showSuggestions", 0).toInt();
    showSwitchTab = settings.value("showSwitchTab", true).toBool();
    settings.endGroup();

    settings.beginGroup("SearchEngines");
    showSearchSuggestions = settings.value("showSuggestions", true).toBool();
    searchOnEngineChange = settings.value("SearchOnEngineChange", true).toBool();
    searchWithDefaultEngine = settings.value("SearchWithDefaultEngine", false).toBool();
    settings.endGroup();

    settings.beginGroup("Web-Browser-Settings");
    defaultZoom = settings.value("DefaultZoom", 100).toInt();
    loadTabsOnActivation = settings.value("LoadTabsOnActivation", false).toBool();
    autoOpenProtocols = settings.value("AutomaticallyOpenProtocols", QStringList()).toStringList();
    blockedProtocols = settings.value("BlockOpeningProtocols", QStringList()).toStringList();
    allowJsGeometryChange = settings.value("allowJavaScriptGeometryChange", true).toBool();
    allowJsHideMenuBar = settings.value("allowJavaScriptHideMenuBar", true).toBool();
    allowJsHideStatusBar = settings.value("allowJavaScriptHideStatusBar", true).toBool();
    allowJsHideToolBar = settings.value("allowJavaScriptHideToolBar", true).toBool();
    settings.endGroup();

    settings.beginGroup("Browser-Tabs-Settings");
    newTabPosition = settings.value("OpenNewTabsSelected", false).toBool() ? Qz::NT_SelectedTab : Qz::NT_NotSelectedTab;
    tabsOnTop = settings.value("TabsOnTop", false).toBool();
    settings.endGroup();
}

void QzSettings::saveSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("AutomaticallyOpenProtocols", autoOpenProtocols);
    settings.setValue("BlockOpeningProtocols", blockedProtocols);
    settings.endGroup();

    settings.beginGroup("Browser-Tabs-Settings");
    settings.setValue("TabsOnTop", tabsOnTop);
    settings.endGroup();
}


