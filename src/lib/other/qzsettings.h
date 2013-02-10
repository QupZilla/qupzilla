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
#ifndef QZSETTINGS_H
#define QZSETTINGS_H

#include "qz_namespace.h"
#include "settings.h"

#include <QStringList>

class QT_QUPZILLA_EXPORT QzSettings
{
public:
    QzSettings();

    void loadSettings();
    void saveSettings();

    // AddressBar
    bool selectAllOnDoubleClick;
    bool selectAllOnClick;
    bool addCountryWithAlt;
    bool showLoadingProgress;
    int showLocationSuggestions;
    bool showSwitchTab;

    // SearchEngines
    bool showSearchSuggestions;
    bool searchOnEngineChange;
    bool searchWithDefaultEngine;

    // Web-Browser-Settings
    int defaultZoom;
    bool loadTabsOnActivation;
    bool allowJsGeometryChange;
    bool allowJsHideMenuBar;
    bool allowJsHideStatusBar;
    bool allowJsHideToolBar;

    QStringList autoOpenProtocols;
    QStringList blockedProtocols;

    // Browser-Tabs-Settings
    Qz::NewTabPositionFlag newTabPosition;
    bool tabsOnTop;
};

#define qzSettings Settings::staticSettings()

#endif // QZSETTINGS_H
