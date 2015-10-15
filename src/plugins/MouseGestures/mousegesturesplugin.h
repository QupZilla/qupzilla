/* ============================================================
* Mouse Gestures plugin for QupZilla
* Copyright (C) 2012-2014  David Rosca <nowrep@gmail.com>
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
#ifndef MOUSEGESTURESPLUGIN_H
#define MOUSEGESTURESPLUGIN_H

#include "plugininterface.h"

class MouseGestures;
class MouseGesturesPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)
    Q_PLUGIN_METADATA(IID "QupZilla.Browser.plugin.MouseGestures")

public:
    MouseGesturesPlugin();
    PluginSpec pluginSpec();

    void init(InitState state, const QString &settingsPath);
    void unload();
    bool testPlugin();

    QTranslator* getTranslator(const QString &locale);
    void showSettings(QWidget* parent = 0);

    bool mousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool mouseRelease(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool mouseMove(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);

private:
    MouseGestures* m_gestures;

};

#endif // MOUSEGESTURESPLUGIN_H
