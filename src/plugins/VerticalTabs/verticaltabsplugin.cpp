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
#include "verticaltabsplugin.h"
#include "verticaltabssettings.h"
#include "verticaltabscontroller.h"
#include "verticaltabsschemehandler.h"

#include "browserwindow.h"
#include "pluginproxy.h"
#include "mainapplication.h"
#include "tabwidget.h"
#include "tabbar.h"
#include "sidebar.h"
#include "networkmanager.h"

#include <QSettings>
#include <QTranslator>

VerticalTabsPlugin::VerticalTabsPlugin()
    : QObject()
{
}

PluginSpec VerticalTabsPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = QSL("Vertical Tabs");
    spec.info = QSL("Vertical tabs for QupZilla");
    spec.description = QSL("Adds ability to show tabs in sidebar");
    spec.version = QSL("0.1.0");
    spec.author = QSL("David Rosca <nowrep@gmail.com>");
    spec.icon = QIcon(QSL(":verticaltabs/data/icon.svg")).pixmap(32);
    spec.hasSettings = true;
    return spec;
}

void VerticalTabsPlugin::init(InitState state, const QString &settingsPath)
{
    m_settingsPath = settingsPath + QL1S("/extensions.ini");

    QSettings settings(m_settingsPath, QSettings::IniFormat);
    settings.beginGroup(QSL("VerticalTabs"));
    m_viewType = static_cast<ViewType>(settings.value(QSL("ViewType"), TabListView).toInt());
    m_replaceTabBar = settings.value(QSL("ReplaceTabBar"), false).toBool();
    m_addChildBehavior = static_cast<AddChildBehavior>(settings.value(QSL("AddChildBehavior"), AppendChild).toInt());
    m_theme = settings.value(QSL("Theme"), QSL(":verticaltabs/data/themes/default.css")).toString();
    settings.endGroup();

    m_controller = new VerticalTabsController(this);
    SideBarManager::addSidebar(QSL("VerticalTabs"), m_controller);

    mApp->plugins()->registerAppEventHandler(PluginProxy::KeyPressHandler, this);
    mApp->networkManager()->registerExtensionSchemeHandler(QSL("verticaltabs"), new VerticalTabsSchemeHandler);

    setWebTabBehavior(m_addChildBehavior);
    loadStyleSheet(m_theme);

    connect(mApp->plugins(), &PluginProxy::mainWindowCreated, this, &VerticalTabsPlugin::mainWindowCreated);

    if (state == LateInitState) {
        const auto windows = mApp->windows();
        for (BrowserWindow *window : windows) {
            mainWindowCreated(window);
        }
    }
}

void VerticalTabsPlugin::unload()
{
    setTabBarVisible(true);

    SideBarManager::removeSidebar(m_controller);
    delete m_controller;
    m_controller = nullptr;

    mApp->networkManager()->unregisterExtensionSchemeHandler(QSL("verticaltabs"));
}

bool VerticalTabsPlugin::testPlugin()
{
    return (Qz::VERSION == QSL(QUPZILLA_VERSION));
}

QTranslator *VerticalTabsPlugin::getTranslator(const QString &locale)
{
    QTranslator *translator = new QTranslator(this);
    translator->load(locale, QSL(":/verticaltabs/locale/"));
    return translator;
}

void VerticalTabsPlugin::showSettings(QWidget *parent)
{
    VerticalTabsSettings *settings = new VerticalTabsSettings(this, parent);
    settings->exec();
}

bool VerticalTabsPlugin::keyPress(const Qz::ObjectName &type, QObject *obj, QKeyEvent *event)
{
    if (type == Qz::ON_TabWidget) {
        return m_controller->handleKeyPress(event, static_cast<TabWidget*>(obj));
    }
    return false;
}

VerticalTabsPlugin::ViewType VerticalTabsPlugin::viewType() const
{
    return m_viewType;
}

void VerticalTabsPlugin::setViewType(ViewType type)
{
    if (m_viewType == type) {
        return;
    }

    m_viewType = type;

    QSettings settings(m_settingsPath, QSettings::IniFormat);
    settings.setValue(QSL("VerticalTabs/ViewType"), m_viewType);

    emit viewTypeChanged(m_viewType);
}

bool VerticalTabsPlugin::replaceTabBar() const
{
    return m_replaceTabBar;
}

void VerticalTabsPlugin::setReplaceTabBar(bool replace)
{
    if (m_replaceTabBar == replace) {
        return;
    }

    m_replaceTabBar = replace;
    setTabBarVisible(!m_replaceTabBar);

    QSettings settings(m_settingsPath, QSettings::IniFormat);
    settings.setValue(QSL("VerticalTabs/ReplaceTabBar"), m_replaceTabBar);
}

VerticalTabsPlugin::AddChildBehavior VerticalTabsPlugin::addChildBehavior() const
{
    return m_addChildBehavior;
}

void VerticalTabsPlugin::setAddChildBehavior(AddChildBehavior behavior)
{
    if (m_addChildBehavior == behavior) {
        return;
    }

    m_addChildBehavior = behavior;
    setWebTabBehavior(m_addChildBehavior);

    QSettings settings(m_settingsPath, QSettings::IniFormat);
    settings.setValue(QSL("VerticalTabs/AddChildBehavior"), m_addChildBehavior);
}

QString VerticalTabsPlugin::theme() const
{
    return m_theme;
}

void VerticalTabsPlugin::setTheme(const QString &theme)
{
    if (theme.isEmpty()) {
        return;
    }

    // Don't check if same to allow live reloading stylesheet

    m_theme = theme;
    loadStyleSheet(m_theme);

    QSettings settings(m_settingsPath, QSettings::IniFormat);
    settings.setValue(QSL("VerticalTabs/Theme"), m_theme);
}

QString VerticalTabsPlugin::styleSheet() const
{
    return m_styleSheet;
}

void VerticalTabsPlugin::mainWindowCreated(BrowserWindow *window)
{
    if (window->sideBarManager()->activeSideBar().isEmpty()) {
        window->sideBarManager()->showSideBar(QSL("VerticalTabs"));
    }
    setTabBarVisible(!m_replaceTabBar);
}

void VerticalTabsPlugin::setTabBarVisible(bool visible)
{
    const auto windows = mApp->windows();
    for (BrowserWindow *window : windows) {
        window->tabWidget()->tabBar()->setForceHidden(!visible);
    }
}

void VerticalTabsPlugin::setWebTabBehavior(AddChildBehavior behavior)
{
    WebTab::setAddChildBehavior(behavior == AppendChild ? WebTab::AppendChild : WebTab::PrependChild);
}

void VerticalTabsPlugin::loadStyleSheet(const QString &theme)
{
    QFile file(theme);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open stylesheet file" << theme;
        file.setFileName(QSL(":verticaltabs/data/themes/default.css"));
        file.open(QFile::ReadOnly);
    }

    m_styleSheet = QString::fromUtf8(file.readAll());
    emit styleSheetChanged(m_styleSheet);
}
