/* ============================================================
* Private Window Plugin for QupZilla
* Copyright (C) 2013  Oliver Gerlich <oliver.gerlich@gmx.de>
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
#ifndef PRIVATEWINDOWPLUGIN_H
#define PRIVATEWINDOWPLUGIN_H

#include "plugininterface.h"

class PrivateWindowPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)

#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "QupZilla.Browser.plugin.PrivateWindowPlugin")
#endif

public:
    explicit PrivateWindowPlugin();
    PluginSpec pluginSpec();

    void init(const QString &sPath);
    void unload();
    bool testPlugin();

    QTranslator* getTranslator(const QString &locale);

    void populateWebViewMenu(QMenu* menu, WebView* view, const QWebHitTestResult &r);

private slots:
    void openUrlInNewPrivateBrowser();
};

#endif // PRIVATEWINDOWPLUGIN_H
