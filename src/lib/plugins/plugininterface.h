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
#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QPixmap>
#include <QtPlugin>
#include <QWebHitTestResult>

#include "qz_namespace.h"

struct PluginSpec {
    QString name;
    QString info;
    QString description;
    QString author;
    QString version;
    QPixmap icon;
    bool hasSettings;

    bool operator==(const PluginSpec &other) {
        return (this->name == other.name &&
                this->info == other.info &&
                this->description == other.description &&
                this->author == other.author &&
                this->version == other.version);
    }
};

class QTranslator;
class QMenu;
class QWebHitTestResult;
class QMouseEvent;
class QKeyEvent;

class WebView;

class PluginInterface
{
public:
    //Plugin Necessary Init Functions
    virtual PluginSpec pluginSpec() = 0;

    virtual void init(const QString &settingsPath) = 0;
    virtual void unload() = 0;
    virtual bool testPlugin() = 0;
    //End Plugin Necessary Init Functions

    virtual ~PluginInterface() { }
    virtual QTranslator* getTranslator(const QString &locale) { Q_UNUSED(locale) return 0; }
    virtual void showSettings(QWidget* parent = 0) { Q_UNUSED(parent) }

    virtual void populateWebViewMenu(QMenu* menu, WebView* view, const QWebHitTestResult &r) { Q_UNUSED(menu) Q_UNUSED(view) Q_UNUSED(r) }

    virtual bool mouseDoubleClick(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event) { Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false; }
    virtual bool mousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event) { Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false; }
    virtual bool mouseRelease(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event) { Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false; }
    virtual bool mouseMove(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event) { Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false; }

    virtual bool keyPress(const Qz::ObjectName &type, QObject* obj, QKeyEvent* event) { Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false; }
    virtual bool keyRelease(const Qz::ObjectName &type, QObject* obj, QKeyEvent* event) { Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false; }
};

Q_DECLARE_INTERFACE(PluginInterface, "QupZilla.Browser.PluginInterface/1.1")

#endif // PLUGININTERFACE_H
