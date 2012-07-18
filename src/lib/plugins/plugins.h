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
#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <QObject>
#include <QVariant>
#include <QWeakPointer>

#include "qz_namespace.h"
#include "plugininterface.h"

class QPluginLoader;

class SpeedDial;

class QT_QUPZILLA_EXPORT Plugins : public QObject
{
    Q_OBJECT
public:
    struct Plugin {
        QString fileName;
        QString fullPath;
        PluginSpec pluginSpec;
        QPluginLoader* pluginLoader;
        PluginInterface* instance;

        Plugin() {
            pluginLoader = 0;
            instance = 0;
        }

        bool isLoaded() const {
            return instance;
        }

        bool operator==(const Plugin &other) const {
            return (this->fileName == other.fileName &&
                    this->fullPath == other.fullPath &&
                    this->pluginSpec == other.pluginSpec &&
                    this->instance == other.instance);
        }
    };

    explicit Plugins(QObject* parent = 0);

    QList<Plugin> getAvailablePlugins();

    bool loadPlugin(Plugin* plugin);
    void unloadPlugin(Plugin* plugin);

    void shutdown();

    // CLick2Flash
    void c2f_loadSettings();
    void c2f_saveSettings();
    void c2f_addWhitelist(QString page) { c2f_whitelist.append(page); }
    void c2f_removeWhitelist(QString page) { c2f_whitelist.removeOne(page); }
    void c2f_setEnabled(bool en) { c2f_enabled = en; }
    bool c2f_isEnabled() { return c2f_enabled; }
    QStringList c2f_getWhiteList() { return c2f_whitelist; }

    // SpeedDial
    SpeedDial* speedDial() { return m_speedDial; }

public slots:
    void loadSettings();

    void loadPlugins();

protected:
    QList<PluginInterface*> m_loadedPlugins;

signals:
    void pluginUnloaded(PluginInterface* plugin);

private:
    bool alreadySpecInAvailable(const PluginSpec &spec);
    PluginInterface* initPlugin(PluginInterface* interface, QPluginLoader* loader);

    void refreshLoadedPlugins();
    void loadAvailablePlugins();

    QList<Plugin> m_availablePlugins;
    QStringList m_allowedPlugins;

    bool m_pluginsEnabled;
    bool m_pluginsLoaded;

    SpeedDial* m_speedDial;
    QStringList c2f_whitelist;
    bool c2f_enabled;
};

Q_DECLARE_METATYPE(Plugins::Plugin)

#endif // PLUGINLOADER_H
