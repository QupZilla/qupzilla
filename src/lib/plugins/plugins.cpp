/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "speeddial.h"
#include "settings.h"

#include <iostream>
#include <QPluginLoader>
#include <QDir>

Plugins::Plugins(QObject* parent)
    : QObject(parent)
    , m_pluginsLoaded(false)
    , m_speedDial(new SpeedDial(this))
{
    loadSettings();
}

QList<Plugins::Plugin> Plugins::getAvailablePlugins()
{
    loadAvailablePlugins();

    return m_availablePlugins;
}

bool Plugins::loadPlugin(Plugins::Plugin* plugin)
{
    if (plugin->isLoaded()) {
        return true;
    }

    plugin->pluginLoader->setFileName(plugin->fullPath);
    PluginInterface* iPlugin = qobject_cast<PluginInterface*>(plugin->pluginLoader->instance());
    if (!iPlugin) {
        return false;
    }

    m_availablePlugins.removeOne(*plugin);
    plugin->instance = initPlugin(iPlugin, plugin->pluginLoader);
    m_availablePlugins.prepend(*plugin);

    refreshLoadedPlugins();

    return plugin->isLoaded();
}

void Plugins::unloadPlugin(Plugins::Plugin* plugin)
{
    if (!plugin->isLoaded()) {
        return;
    }

    plugin->instance->unload();
    plugin->pluginLoader->unload();
    emit pluginUnloaded(plugin->instance);

    m_availablePlugins.removeOne(*plugin);
    plugin->instance = 0;
    m_availablePlugins.append(*plugin);

    refreshLoadedPlugins();
}

void Plugins::loadSettings()
{
    Settings settings;
    settings.beginGroup("Plugin-Settings");
    m_pluginsEnabled = settings.value("EnablePlugins", DEFAULT_ENABLE_PLUGINS).toBool();
    m_allowedPlugins = settings.value("AllowedPlugins", QStringList()).toStringList();
    settings.endGroup();

    c2f_loadSettings();
}

void Plugins::shutdown()
{
    c2f_saveSettings();
    m_speedDial->saveSettings();

    foreach(PluginInterface * iPlugin, m_loadedPlugins) {
        iPlugin->unload();
    }
}

void Plugins::c2f_loadSettings()
{
    Settings settings;
    settings.beginGroup("ClickToFlash");
    c2f_whitelist = settings.value("whitelist", QStringList()).toStringList();
    c2f_enabled = settings.value("Enabled", true).toBool();
    settings.endGroup();
}

void Plugins::c2f_saveSettings()
{
    Settings settings;
    settings.beginGroup("ClickToFlash");
    settings.setValue("whitelist", c2f_whitelist);
    settings.setValue("Enabled", c2f_enabled);
    settings.endGroup();
}

void Plugins::loadPlugins()
{
    if (!m_pluginsEnabled) {
        return;
    }

    QDir settingsDir(mApp->currentProfilePath() + "extensions/");
    if (!settingsDir.exists()) {
        settingsDir.mkdir(settingsDir.absolutePath());
    }

    foreach(const QString & fullPath, m_allowedPlugins) {
        QPluginLoader* loader = new QPluginLoader(fullPath);
        PluginInterface* iPlugin = qobject_cast<PluginInterface*>(loader->instance());
        if (!iPlugin) {
            continue;
        }

        Plugin plugin;
        plugin.fullPath = fullPath;
        plugin.pluginLoader = loader;
        plugin.instance = initPlugin(iPlugin, loader);

        if (plugin.isLoaded()) {
            plugin.pluginSpec = iPlugin->pluginSpec();

            m_loadedPlugins.append(plugin.instance);
            m_availablePlugins.append(plugin);
        }
    }

    refreshLoadedPlugins();

    std::cout << "QupZilla: " << m_loadedPlugins.count() << " extensions loaded"  << std::endl;
}

void Plugins::loadAvailablePlugins()
{
    if (m_pluginsLoaded) {
        return;
    }

    m_pluginsLoaded = true;

    QStringList dirs;
    dirs << mApp->DATADIR + "plugins/"
#if defined(QZ_WS_X11) && !defined(NO_SYSTEM_DATAPATH)
#ifdef USE_LIBPATH
         << USE_LIBPATH "qupzilla/"
#else
         << "/usr/lib/qupzilla/"
#endif
#endif
         << mApp->PROFILEDIR + "plugins/";

    foreach(const QString & dir, dirs) {
        QDir pluginsDir = QDir(dir);
        foreach(const QString & fileName, pluginsDir.entryList(QDir::Files)) {
            const QString absolutePath = pluginsDir.absoluteFilePath(fileName);
            if (m_allowedPlugins.contains(absolutePath)) {
                continue;
            }

            QPluginLoader* loader = new QPluginLoader(absolutePath);
            PluginInterface* iPlugin = qobject_cast<PluginInterface*>(loader->instance());
            if (!iPlugin) {
                continue;
            }

            Plugin plugin;
            plugin.fullPath = absolutePath;
            plugin.pluginSpec = iPlugin->pluginSpec();
            plugin.pluginLoader = loader;
            plugin.instance = 0;

            loader->unload();

            if (!alreadySpecInAvailable(plugin.pluginSpec)) {
                m_availablePlugins.append(plugin);
            }
        }
    }
}

PluginInterface* Plugins::initPlugin(PluginInterface* interface, QPluginLoader* loader)
{
    if (!interface) {
        return 0;
    }

    interface->init(mApp->currentProfilePath() + "extensions/");

    if (!interface->testPlugin()) {
        interface->unload();
        loader->unload();

        emit pluginUnloaded(interface);

        return 0;
    }

    qApp->installTranslator(interface->getTranslator(mApp->currentLanguageFile()));

    return interface;
}

void Plugins::refreshLoadedPlugins()
{
    m_loadedPlugins.clear();

    foreach(const Plugin & plugin, m_availablePlugins) {
        if (plugin.isLoaded()) {
            m_loadedPlugins.append(plugin.instance);
        }
    }
}

bool Plugins::alreadySpecInAvailable(const PluginSpec &spec)
{
    foreach(const Plugin & plugin, m_availablePlugins) {
        if (plugin.pluginSpec == spec) {
            return true;
        }
    }

    return false;
}
