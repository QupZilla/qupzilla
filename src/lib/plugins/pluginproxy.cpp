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
#include "settings.h"

#include <QMenu>

PluginProxy::PluginProxy()
    : Plugins()
{
}

void PluginProxy::unloadPlugin(Plugins::Plugin* plugin)
{
    m_mousePressHandlers.removeOne(plugin->instance);
    m_mouseReleaseHandlers.removeOne(plugin->instance);
    m_mouseMoveHandlers.removeOne(plugin->instance);

    m_keyPressHandlers.removeOne(plugin->instance);
    m_keyReleaseHandlers.removeOne(plugin->instance);

    Plugins::unloadPlugin(plugin);
}

void PluginProxy::registerAppEventHandler(const PluginProxy::EventHandlerType &type, PluginInterface* obj)
{
    switch (type) {
    case MouseDoubleClickHandler:
        if (!m_mouseDoubleClickHandlers.contains(obj)) {
            m_mouseDoubleClickHandlers.append(obj);
        }
        break;

    case MousePressHandler:
        if (!m_mousePressHandlers.contains(obj)) {
            m_mousePressHandlers.append(obj);
        }
        break;

    case MouseReleaseHandler:
        if (!m_mouseReleaseHandlers.contains(obj)) {
            m_mouseReleaseHandlers.append(obj);
        }
        break;

    case MouseMoveHandler:
        if (!m_mouseMoveHandlers.contains(obj)) {
            m_mouseMoveHandlers.append(obj);
        }
        break;

    case KeyPressHandler:
        if (!m_keyPressHandlers.contains(obj)) {
            m_keyPressHandlers.append(obj);
        }
        break;

    case KeyReleaseHandler:
        if (!m_keyReleaseHandlers.contains(obj)) {
            m_keyReleaseHandlers.append(obj);
        }
        break;

    default:
        qWarning("PluginProxy::registerAppEventHandler registering unknown event handler type");
        break;
    }
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

bool PluginProxy::processMouseDoubleClick(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    bool accepted = false;

    foreach(PluginInterface * iPlugin, m_mouseDoubleClickHandlers) {
        if (iPlugin->mouseDoubleClick(type, obj, event)) {
            accepted = true;
        }
    }

    return accepted;
}

bool PluginProxy::processMousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    bool accepted = false;

    foreach(PluginInterface * iPlugin, m_mousePressHandlers) {
        if (iPlugin->mousePress(type, obj, event)) {
            accepted = true;
        }
    }

    return accepted;
}

bool PluginProxy::processMouseRelease(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    bool accepted = false;

    foreach(PluginInterface * iPlugin, m_mouseReleaseHandlers) {
        if (iPlugin->mouseRelease(type, obj, event)) {
            accepted = true;
        }
    }

    return accepted;
}

bool PluginProxy::processMouseMove(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    bool accepted = false;

    foreach(PluginInterface * iPlugin, m_mouseMoveHandlers) {
        if (iPlugin->mouseMove(type, obj, event)) {
            accepted = true;
        }
    }

    return accepted;
}

bool PluginProxy::processKeyPress(const Qz::ObjectName &type, QObject* obj, QKeyEvent* event)
{
    bool accepted = false;

    foreach(PluginInterface * iPlugin, m_keyPressHandlers) {
        if (iPlugin->keyPress(type, obj, event)) {
            accepted = true;
        }
    }

    return accepted;
}

bool PluginProxy::processKeyRelease(const Qz::ObjectName &type, QObject* obj, QKeyEvent* event)
{
    bool accepted = false;

    foreach(PluginInterface * iPlugin, m_keyReleaseHandlers) {
        if (iPlugin->keyRelease(type, obj, event)) {
            accepted = true;
        }
    }

    return accepted;
}

void PluginProxy::emitWebViewCreated(WebView* view)
{
    emit webViewCreated(view);
}

void PluginProxy::emitWebViewDeleted(WebView* view)
{
    emit webViewDeleted(view);
}

void PluginProxy::emitMainWindowCreated(QupZilla* window)
{
    emit mainWindowCreated(window);
}

void PluginProxy::emitMainWindowDeleted(QupZilla* window)
{
    emit mainWindowDeleted(window);
}

