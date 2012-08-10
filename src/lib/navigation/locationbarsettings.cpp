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
#include "locationbarsettings.h"
#include "mainapplication.h"
#include "settings.h"

bool LocationBarSettings::selectAllOnDoubleClick = false;
bool LocationBarSettings::selectAllOnClick = false;
bool LocationBarSettings::addCountryWithAlt = false;
bool LocationBarSettings::showSearchSuggestions = false;

LocationBarSettings::LocationBarSettings()
{
    loadSettings();
}

void LocationBarSettings::loadSettings()
{
    Settings settings;
    settings.beginGroup("AddressBar");
    selectAllOnDoubleClick = settings.value("SelectAllTextOnDoubleClick", true).toBool();
    selectAllOnClick = settings.value("SelectAllTextOnClick", false).toBool();
    addCountryWithAlt = settings.value("AddCountryDomainWithAltKey", true).toBool();
    settings.endGroup();

    settings.beginGroup("SearchEngines");
    showSearchSuggestions = settings.value("showSuggestions", true).toBool();
    settings.endGroup();
}
