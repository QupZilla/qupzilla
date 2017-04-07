/* ============================================================
* TabManager plugin for QupZilla
* Copyright (C) 2013-2017  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#ifndef TABMANAGERPLUGIN_H
#define TABMANAGERPLUGIN_H


#include "plugininterface.h"
#include "tabmanagerwidgetcontroller.h"

#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPointer>

class TabManagerWidget;

class TabManagerPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)
    Q_PLUGIN_METADATA(IID "QupZilla.Browser.plugin.TabManagerPlugin")

public:
    explicit TabManagerPlugin();
    PluginSpec pluginSpec();

    void init(InitState state, const QString &settingsPath);
    void unload();
    bool testPlugin();

    QTranslator* getTranslator(const QString &locale);
    void showSettings(QWidget* parent = 0);
    void  populateExtensionsMenu(QMenu* menu);

    enum ViewType {
        ShowAsSideBar = 0,
        ShowAsWindow = 1,
        Undefined = -1
    };

    void removeManagerWidget();

    ViewType viewType();
    void setViewType(ViewType type);

    static QString settingsPath();
    void saveSettings();

    bool asTabBarReplacement() const;
    void setAsTabBarReplacement(bool yes);

public slots:
    void insertManagerWidget();

private slots:
    void mainWindowCreated(BrowserWindow* window, bool refresh = true);

private:
    void setTabBarVisible(bool visible);

    TabManagerWidgetController* m_controller;
    TabManagerWidget* m_tabManagerWidget;
    static QString s_settingsPath;
    ViewType m_viewType;
    bool m_initState;
    bool m_asTabBarReplacement;
};

#endif // TABMANAGERPLUGIN_H
