/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#ifndef PLUGINPROXY_H
#define PLUGINPROXY_H

#include "plugins.h"
#include "qzcommon.h"

#include <QWebEnginePage>

class WebPage;
class BrowserWindow;

class QUPZILLA_EXPORT PluginProxy : public Plugins
{
    Q_OBJECT
public:
    enum EventHandlerType { MouseDoubleClickHandler, MousePressHandler, MouseReleaseHandler,
                            MouseMoveHandler, KeyPressHandler, KeyReleaseHandler,
                            WheelEventHandler
                          };

    explicit PluginProxy(QObject *parent = nullptr);

    void registerAppEventHandler(const EventHandlerType &type, PluginInterface* obj);

    void populateWebViewMenu(QMenu* menu, WebView* view, const WebHitTestResult &r);
    void populateExtensionsMenu(QMenu *menu);

    bool processMouseDoubleClick(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool processMousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool processMouseRelease(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool processMouseMove(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);

    bool processWheelEvent(const Qz::ObjectName &type, QObject* obj, QWheelEvent* event);

    bool processKeyPress(const Qz::ObjectName &type, QObject* obj, QKeyEvent* event);
    bool processKeyRelease(const Qz::ObjectName &type, QObject* obj, QKeyEvent* event);

    bool acceptNavigationRequest(WebPage *page, const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame);

    void emitWebPageCreated(WebPage* page);
    void emitWebPageDeleted(WebPage* page);

    void emitMainWindowCreated(BrowserWindow* window);
    void emitMainWindowDeleted(BrowserWindow* window);

signals:
    void webPageCreated(WebPage* page);
    void webPageDeleted(WebPage* page);

    void mainWindowCreated(BrowserWindow* window);
    void mainWindowDeleted(BrowserWindow* window);

private slots:
    void pluginUnloaded(PluginInterface* plugin);

private:
    QList<PluginInterface*> m_mouseDoubleClickHandlers;
    QList<PluginInterface*> m_mousePressHandlers;
    QList<PluginInterface*> m_mouseReleaseHandlers;
    QList<PluginInterface*> m_mouseMoveHandlers;

    QList<PluginInterface*> m_wheelEventHandlers;

    QList<PluginInterface*> m_keyPressHandlers;
    QList<PluginInterface*> m_keyReleaseHandlers;
};

#endif // PLUGINPROXY_H
