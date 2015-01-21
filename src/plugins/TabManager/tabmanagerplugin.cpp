/* ============================================================
* TabManager plugin for QupZilla
* Copyright (C) 2013  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#include "tabmanagerplugin.h"
#include "tabmanagerwidgetcontroller.h"
#include "tabmanagerwidget.h"
#include "browserwindow.h"
#include "pluginproxy.h"
#include "mainapplication.h"
#include "sidebar.h"

#include <QInputDialog>
#include <QTranslator>
#include <QSettings>
#include <QAction>
#include <QTimer>

QString TabManagerPlugin::s_settingsPath;

TabManagerPlugin::TabManagerPlugin()
    : QObject()
    , m_controller(0)
    , m_tabManagerWidget(0)
    , m_initState(false)
{
}

PluginSpec TabManagerPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "Tab Manager";
    spec.info = "Simple yet powerful tab manager for QupZilla";
    spec.description = "Adds ability to managing tabs and windows";
    spec.version = "0.4.1";
    spec.author = "Razi Alavizadeh <s.r.alavizadeh@gmail.com>";
    spec.icon = QPixmap(":tabmanager/data/tabmanager.png");
    spec.hasSettings = true;

    return spec;
}

void TabManagerPlugin::init(InitState state, const QString &settingsPath)
{
    Q_UNUSED(state)

    m_controller = new TabManagerWidgetController(this);
    connect(mApp->plugins(), SIGNAL(mainWindowCreated(BrowserWindow*)), m_controller, SLOT(mainWindowCreated(BrowserWindow*)));
    connect(mApp->plugins(), SIGNAL(mainWindowDeleted(BrowserWindow*)), m_controller, SLOT(mainWindowDeleted(BrowserWindow*)));
    connect(mApp->plugins(), SIGNAL(webPageCreated(WebPage*)), m_controller, SIGNAL(requestRefreshTree()));
    connect(mApp->plugins(), SIGNAL(webPageDeleted(WebPage*)), m_controller, SIGNAL(requestRefreshTree(WebPage*)));

    s_settingsPath = settingsPath + QL1S("/TabManager");
    m_initState = true;

    // load settings
    QSettings settings(s_settingsPath + QL1S("/tabmanager.ini"), QSettings::IniFormat);
    settings.beginGroup("View");
    m_controller->setGroupType(TabManagerWidget::GroupType(settings.value("GroupType", TabManagerWidget::GroupByWindow).toInt()));
    m_controller->setViewType(TabManagerWidgetController::ViewType(settings.value("ViewType", TabManagerWidgetController::ShowAsWindow).toInt()));
    settings.endGroup();

    insertManagerWidget();
}

void TabManagerPlugin::unload()
{
    // save settings
    QSettings settings(s_settingsPath + QL1S("/tabmanager.ini"), QSettings::IniFormat);
    settings.beginGroup("View");
    settings.setValue("GroupType", m_controller->groupType());
    settings.setValue("ViewType", m_controller->viewType());
    settings.endGroup();

    removeManagerWidget();

    delete m_controller;
}

bool TabManagerPlugin::testPlugin()
{
    return (Qz::VERSION == QLatin1String(QUPZILLA_VERSION));
}

QTranslator* TabManagerPlugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator(this);
    translator->load(locale, ":/tabmanager/locale/");
    return translator;
}

void TabManagerPlugin::showSettings(QWidget* parent)
{
    bool ok;
    QString viewType = QInputDialog::getItem(parent, tr("Tab Manager View Type"),
                       tr("<p>Please select view type:<br />"
                          "<b>Note:</b> The \"<i>Window</i>\" type is recommended for managing lots of windows/tabs")
                       , QStringList() << tr("SideBar") << tr("Window")
                       , m_controller->viewType(), false, &ok, Qt::WindowStaysOnTopHint);
    TabManagerWidgetController::ViewType type;
    if (viewType == tr("SideBar")) {
        type = TabManagerWidgetController::ShowAsSideBar;
    }
    else {
        type = TabManagerWidgetController::ShowAsWindow;
    }

    if (ok && type != m_controller->viewType()) {
        removeManagerWidget();
        m_controller->setViewType(type);
        insertManagerWidget();

        if (type == TabManagerWidgetController::ShowAsSideBar) {
            mApp->getWindow()->sideBarManager()->showSideBar("TabManager");
        }
        else if (type == TabManagerWidgetController::ShowAsWindow) {
            // add statusbar icon
            foreach (BrowserWindow* window, mApp->windows()) {
                m_controller->addStatusBarIcon(window);
            }
        }
    }
}

void TabManagerPlugin::insertManagerWidget()
{
    if (m_controller->viewType() == TabManagerWidgetController::ShowAsSideBar) {
        SideBarManager::addSidebar("TabManager", m_controller);
    }
    else if (m_controller->viewType() == TabManagerWidgetController::ShowAsWindow) {
        if (!m_tabManagerWidget) {
            m_tabManagerWidget = m_controller->createTabManagerWidget(mApp->getWindow(), 0, true);
            m_tabManagerWidget->setWindowFlags(Qt::Window);
        }
    }

    if (m_initState) {
        foreach (BrowserWindow* window, mApp->windows()) {
            m_controller->mainWindowCreated(window, false);
        }

        m_initState = false;
    }
}

void TabManagerPlugin::removeManagerWidget()
{
    if (m_controller->viewType() == TabManagerWidgetController::ShowAsSideBar) {
        SideBarManager::removeSidebar("TabManager");
    }
    else if (m_controller->viewType() == TabManagerWidgetController::ShowAsWindow) {
        // remove statusbar icon
        foreach (BrowserWindow* window, mApp->windows()) {
            m_controller->removeStatusBarIcon(window);
        }

        m_tabManagerWidget->close();
        delete m_tabManagerWidget;
        m_tabManagerWidget = 0;
    }
}

QString TabManagerPlugin::settingsPath()
{
    return s_settingsPath;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(TabManager, TabManagerPlugin)
#endif
