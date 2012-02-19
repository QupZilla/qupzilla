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

#include <QtPlugin>
#include <QIcon>
#include <QTranslator>
#include <QMenu>
#include <QNetworkRequest>
#include <QWebView>
#include <QWebHitTestResult>

struct PluginSpec {
    QString name;
    QString info;
    QString description;
    QString author;
    QString version;
    QIcon icon;
    bool hasSettings;

    bool operator==(const PluginSpec &other) {
        return (this->name == other.name &&
                this->info == other.info &&
                this->description == other.description &&
                this->author == other.author &&
                this->version == other.version);
    }
};

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

    virtual void populateToolsMenu(QMenu* menu) { Q_UNUSED(menu) }
    virtual void populateHelpMenu(QMenu* menu) { Q_UNUSED(menu) }
    virtual void populateWebViewMenu(QMenu* menu, QWebView* view, const QWebHitTestResult &r) { Q_UNUSED(menu) Q_UNUSED(view) Q_UNUSED(r) }

    virtual void formSent(const QNetworkRequest &request, const QByteArray &outgoingData) { Q_UNUSED(request) Q_UNUSED(outgoingData)}
    virtual void pageLoaded(QWebView* view) { Q_UNUSED(view) }
    virtual void downloadRequested(QWidget* requestWidget) { Q_UNUSED(requestWidget) }
    virtual QNetworkReply* createNetworkRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
    { Q_UNUSED(op) Q_UNUSED(request) Q_UNUSED(outgoingData) return 0; }
};

Q_DECLARE_INTERFACE(PluginInterface, "QupZilla.Browser.PluginInterface/1.1")


#endif // PLUGININTERFACE_H
