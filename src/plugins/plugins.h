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
#include <QtPlugin>
#include <QPluginLoader>
#include <QDir>

#include <QTimer>
#include <iostream>

class PluginInterface;
class Plugins : public QObject
{
    Q_OBJECT
public:
    explicit Plugins(QObject* parent = 0);

    QStringList getAvailablePlugins() { return m_availablePluginFileNames; }
    QStringList getAllowedPlugins() { return m_allowedPluginFileNames; }
    PluginInterface* getPlugin(QString pluginFileName);
    //void setPluginsAllowed(bool state) { pluginsEnabled = state; qDebug() << state;}

public slots:
    void loadSettings();
    void loadPlugins();

protected:
    QList<PluginInterface* > loadedPlugins;

private:
    QStringList m_availablePluginFileNames;
    QStringList m_allowedPluginFileNames;
    QStringList m_loadedPluginFileNames;
    bool m_pluginsEnabled;
};

#endif // PLUGINLOADER_H
