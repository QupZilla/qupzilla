/* ============================================================
* AutoScroll - Autoscroll for QupZilla
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
#ifndef AUTOSCROLLPLUGIN_H
#define AUTOSCROLLPLUGIN_H

#include "plugininterface.h"

class AutoScroller;
class AutoScrollSettings;

class AutoScrollPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)

#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "QupZilla.Browser.plugin.TestPlugin")
#endif

public:
    explicit AutoScrollPlugin();
    PluginSpec pluginSpec();

    void init(InitState state, const QString &settingsPath);
    void unload();
    bool testPlugin();

    QTranslator* getTranslator(const QString &locale);
    void showSettings(QWidget* parent);

    bool mouseMove(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool mousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool mouseRelease(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);

private:
    AutoScroller* m_scroller;
    QPointer<AutoScrollSettings> m_settings;
};

#endif // TESTPLUGIN_H
