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
#pragma once

#include "plugininterface.h"

class WebPage;
class BrowserWindow;

class AdBlockPlugin : public QObject, public PluginInterface
{
    Q_OBJECT

public:
    explicit AdBlockPlugin();
    PluginSpec pluginSpec();
    void init(InitState state, const QString &settingsPath);
    void unload();
    bool testPlugin();

private:
    void webPageCreated(WebPage *page);
    void webPageDeleted(WebPage *page);
    void mainWindowCreated(BrowserWindow *window);
    bool acceptNavigationRequest(WebPage *page, const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override;

};
