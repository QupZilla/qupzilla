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
#include "websettings.h"
#include "settings.h"

int WebSettings::defaultZoom = 100;
bool WebSettings::loadTabsOnActivation = false;

Qz::NewTabPositionFlag WebSettings::newTabPosition;
QStringList WebSettings::autoOpenProtocols;
QStringList WebSettings::blockedProtocols;

void WebSettings::loadSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");

    defaultZoom = settings.value("DefaultZoom", 100).toInt();
    loadTabsOnActivation = settings.value("LoadTabsOnActivation", false).toBool();

    autoOpenProtocols = settings.value("AutomaticallyOpenProtocols", QStringList()).toStringList();
    blockedProtocols = settings.value("BlockOpeningProtocols", QStringList()).toStringList();

    settings.endGroup();

    settings.beginGroup("Browser-Tabs-Settings");
    newTabPosition = settings.value("OpenNewTabsSelected", false).toBool() ? Qz::NT_SelectedTab : Qz::NT_NotSelectedTab;
    settings.endGroup();
}

void WebSettings::saveSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");

    settings.setValue("AutomaticallyOpenProtocols", autoOpenProtocols);
    settings.setValue("BlockOpeningProtocols", blockedProtocols);

    settings.endGroup();
}
