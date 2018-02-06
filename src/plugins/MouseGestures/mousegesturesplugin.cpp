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
#include "mousegesturesplugin.h"
#include "pluginproxy.h"
#include "mousegestures.h"
#include "mainapplication.h"
#include "browserwindow.h"

#include <QTranslator>

MouseGesturesPlugin::MouseGesturesPlugin()
    : QObject()
    , m_gestures(0)
{
}

PluginSpec MouseGesturesPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "Mouse Gestures";
    spec.info = "Mouse gestures for QupZilla";
    spec.description = "Provides support for navigating in webpages by mouse gestures";
    spec.version = "0.5.0";
    spec.author = "David Rosca <nowrep@gmail.com>";
    spec.icon = QPixmap(":/mousegestures/data/icon.png");
    spec.hasSettings = true;

    return spec;
}

void MouseGesturesPlugin::init(InitState state, const QString &settingsPath)
{
    Q_UNUSED(state)

    m_gestures = new MouseGestures(settingsPath, this);

    mApp->plugins()->registerAppEventHandler(PluginProxy::MousePressHandler, this);
    mApp->plugins()->registerAppEventHandler(PluginProxy::MouseReleaseHandler, this);
    mApp->plugins()->registerAppEventHandler(PluginProxy::MouseMoveHandler, this);
}

void MouseGesturesPlugin::unload()
{
    m_gestures->unloadPlugin();
    m_gestures->deleteLater();
}

bool MouseGesturesPlugin::testPlugin()
{
    // Require the version that the plugin was built with
    return (Qz::VERSION == QLatin1String(QUPZILLA_VERSION));
}

QTranslator* MouseGesturesPlugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator(this);
    translator->load(locale, ":/mousegestures/locale/");
    return translator;
}

void MouseGesturesPlugin::showSettings(QWidget* parent)
{
    m_gestures->showSettings(parent);
}

bool MouseGesturesPlugin::mousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    if (type == Qz::ON_WebView) {
        m_gestures->mousePress(obj, event);
    }

    return false;
}

bool MouseGesturesPlugin::mouseRelease(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    if (type == Qz::ON_WebView) {
        return m_gestures->mouseRelease(obj, event);
    }

    return false;
}

bool MouseGesturesPlugin::mouseMove(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    if (type == Qz::ON_WebView) {
        m_gestures->mouseMove(obj, event);
    }

    return false;
}
