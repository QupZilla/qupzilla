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
#ifndef PLUGINPROXY_H
#define PLUGINPROXY_H

#include "plugins.h"
#include "qz_namespace.h"

class QupZilla;
class WebPage;

class QT_QUPZILLA_EXPORT PluginProxy : public Plugins
{
    Q_OBJECT
public:
    enum EventHandlerType { MouseDoubleClickHandler, MousePressHandler, MouseReleaseHandler,
                            MouseMoveHandler, KeyPressHandler, KeyReleaseHandler,
                            WheelEventHandler
                          };

    explicit PluginProxy();

    void registerAppEventHandler(const EventHandlerType &type, PluginInterface* obj);

    void populateWebViewMenu(QMenu* menu, WebView* view, const QWebHitTestResult &r);

    bool processMouseDoubleClick(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool processMousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool processMouseRelease(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool processMouseMove(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);

    bool processWheelEvent(const Qz::ObjectName &type, QObject* obj, QWheelEvent* event);

    bool processKeyPress(const Qz::ObjectName &type, QObject* obj, QKeyEvent* event);
    bool processKeyRelease(const Qz::ObjectName &type, QObject* obj, QKeyEvent* event);

    QNetworkReply* createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData);

    void emitWebPageCreated(WebPage* page);
    void emitWebPageDeleted(WebPage* page);

    void emitMainWindowCreated(QupZilla* window);
    void emitMainWindowDeleted(QupZilla* window);

signals:
    void webPageCreated(WebPage* page);
    void webPageDeleted(WebPage* page);

    void mainWindowCreated(QupZilla* window);
    void mainWindowDeleted(QupZilla* window);

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

#define QZ_REGISTER_EVENT_HANDLER(Type) mApp->plugins()->registerAppEventHandler(Type, this);
#define QZ_REGISTER_SCHEME_HANDLER(Scheme, Object) mApp->networkManager()->registerSchemeHandler(Scheme, Object);

#endif // PLUGINPROXY_H
