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

#ifdef PORTABLE_BUILD
#define DEFAULT_ENABLE_PLUGINS false
#else
#define DEFAULT_ENABLE_PLUGINS true
#endif

Plugins::Plugins(QObject* parent)
    : QObject(parent)
    , m_pluginsLoaded(false)
{
    loadSettings();
}

void Plugins::loadPlugin(Plugins::Plugin* plugin)
{
    if (plugin->isLoaded()) {
        return;
    }

    plugin->pluginLoader->setFileName(plugin->fullPath);
    PluginInterface* iPlugin = qobject_cast<PluginInterface*>(plugin->pluginLoader->instance());
    if (!iPlugin) {
        return;
    }

    m_availablePlugins.removeOne(*plugin);
    plugin->instance = initPlugin(iPlugin, plugin->pluginLoader);
    m_availablePlugins.append(*plugin);

    refreshLoadedPlugins();
}

void Plugins::unloadPlugin(Plugins::Plugin* plugin)
{
    if (!plugin->isLoaded()) {
        return;
    }

    plugin->instance->unload();
    plugin->pluginLoader->unload();

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
    m_allowedPluginFileNames = settings.value("AllowedPlugins", QStringList()).toStringList();
    settings.endGroup();
}

void Plugins::loadPlugins()
{
    if (!m_pluginsEnabled || m_pluginsLoaded) {
        return;
    }

    m_pluginsLoaded = true;

    QStringList dirs;
    dirs << mApp->DATADIR + "plugins/"
#ifdef Q_WS_X11
         << "/usr/lib/qupzilla/"
#endif
         << mApp->PROFILEDIR + "plugins/";

    foreach(const QString & dir, dirs) {
        QDir pluginsDir = QDir(dir);
        foreach(const QString & fileName, pluginsDir.entryList(QDir::Files)) {
            QPluginLoader* loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
            PluginInterface* iPlugin = qobject_cast<PluginInterface*>(loader->instance());
            if (!iPlugin) {
                continue;
            }

            Plugin plugin;
            plugin.fileName = fileName;
            plugin.fullPath = pluginsDir.absoluteFilePath(fileName);
            plugin.pluginSpec = iPlugin->pluginSpec();
            plugin.pluginLoader = loader;
            plugin.instance = 0;

            if (m_allowedPluginFileNames.contains(fileName)) {
                plugin.instance = initPlugin(iPlugin, loader);
            }
            else {
                loader->unload();
            }

            m_availablePlugins.append(plugin);
        }
    }

    refreshLoadedPlugins();

    std::cout << m_loadedPlugins.count() << " plugins loaded"  << std::endl;
}

PluginInterface* Plugins::initPlugin(PluginInterface* interface, QPluginLoader* loader)
{
    if (!interface) {
        return 0;
    }

    interface->init(mApp->getActiveProfilPath() + "plugins.ini");

    if (!interface->testPlugin()) {
        interface->unload();
        loader->unload();
        return 0;
    }

    qApp->installTranslator(interface->getTranslator(mApp->getActiveLanguage()));
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
