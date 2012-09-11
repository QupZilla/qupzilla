/* ============================================================
* Access Keys Navigation plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
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
#ifndef AKN_PLUGIN_H
#define AKN_PLUGIN_H

#include <QWeakPointer>

#include "plugininterface.h"

class AKN_Handler;
class AKN_Settings;

class AKN_Plugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)

public:
    AKN_Plugin();
    PluginSpec pluginSpec();

    void init(const QString &sPath);
    void unload();
    bool testPlugin();

    QTranslator* getTranslator(const QString &locale);
    void showSettings(QWidget* parent = 0);

    bool keyPress(const Qz::ObjectName &type, QObject* obj, QKeyEvent* event);

private:
    QWeakPointer<AKN_Settings> m_settings;
    AKN_Handler* m_handler;
};

#endif // AKN_PLUGIN_H
