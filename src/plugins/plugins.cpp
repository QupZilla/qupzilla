/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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

Plugins::Plugins(QObject* parent)
    : QObject(parent)
{
    loadSettings();
}

void Plugins::loadSettings()
{
    m_allowedPluginFileNames.clear();

    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Plugin-Settings");
    m_pluginsEnabled = settings.value("EnablePlugins", true).toBool();
    m_allowedPluginFileNames = settings.value("AllowedPlugins", QStringList()).toStringList();
    settings.endGroup();
}

void Plugins::loadPlugins()
{
    if (!m_pluginsEnabled) {
        return;
    }

    m_availablePluginFileNames.clear();
    loadedPlugins.clear();

    QDir pluginsDir = QDir(mApp->PLUGINSDIR);

    foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
        m_availablePluginFileNames.append(fileName);

        if (!m_allowedPluginFileNames.contains(fileName)) {
            continue;
        }

        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject* plugin = loader.instance();
        if (plugin) {
            PluginInterface* iPlugin = qobject_cast<PluginInterface*>(plugin);
            iPlugin->init(mApp->getActiveProfilPath() + "plugins.ini");
            if (!iPlugin->testPlugin()) {
                loader.unload();
                continue;
            }

            qApp->installTranslator(iPlugin->getTranslator(mApp->getActiveLanguage()));
            loadedPlugins.append(iPlugin);
            m_loadedPluginFileNames.append(fileName);
        }
    }

    std::cout << loadedPlugins.count() << " plugins loaded" << std::endl;
}

PluginInterface* Plugins::getPlugin(QString pluginFileName)
{
    QString path = mApp->PLUGINSDIR + pluginFileName;
    if (!QFile::exists(path)) {
        return 0;
    }
    QPluginLoader loader(path);
    QObject* plugin = loader.instance();

    if (plugin) {
        PluginInterface* iPlugin = qobject_cast<PluginInterface*>(plugin);
        return iPlugin;
    }
    else {
        return 0;
    }
}
