/* ============================================================
* VerticalTabs plugin for QupZilla
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#pragma once

#include "plugininterface.h"

class BrowserWindow;

class VerticalTabsController;

class VerticalTabsPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)
    Q_PLUGIN_METADATA(IID "QupZilla.Browser.plugin.VerticalTabs")

public:
    explicit VerticalTabsPlugin();

    PluginSpec pluginSpec() override;
    void init(InitState state, const QString &settingsPath) override;
    void unload() override;
    bool testPlugin() override;
    QTranslator* getTranslator(const QString &locale) override;
    void showSettings(QWidget *parent = nullptr) override;
    bool keyPress(const Qz::ObjectName &type, QObject *obj, QKeyEvent *event) override;

    enum ViewType {
        TabListView,
        TabTreeView
    };

    ViewType viewType() const;
    void setViewType(ViewType type);

    bool replaceTabBar() const;
    void setReplaceTabBar(bool replace);

    enum AddChildBehavior {
        AppendChild,
        PrependChild
    };

    AddChildBehavior addChildBehavior() const;
    void setAddChildBehavior(AddChildBehavior behavior);

    QString theme() const;
    void setTheme(const QString &theme);

    QString styleSheet() const;

signals:
    void viewTypeChanged(ViewType type);
    void styleSheetChanged(const QString &styleSheet);

private:
    void mainWindowCreated(BrowserWindow *window);
    void setTabBarVisible(bool visible);
    void setWebTabBehavior(AddChildBehavior behavior);
    void loadStyleSheet(const QString &theme);

    QString m_settingsPath;
    VerticalTabsController *m_controller = nullptr;
    ViewType m_viewType = TabListView;
    bool m_replaceTabBar = false;
    AddChildBehavior m_addChildBehavior = AppendChild;
    QString m_theme;
    QString m_styleSheet;
};
