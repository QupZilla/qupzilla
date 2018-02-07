/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2018 David Rosca <nowrep@gmail.com>
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
#include "gm_plugin.h"
#include "gm_manager.h"
#include "browserwindow.h"
#include "webpage.h"
#include "pluginproxy.h"
#include "mainapplication.h"
#include "emptynetworkreply.h"
#include "tabwidget.h"
#include "webtab.h"

#include <QTranslator>

GM_Plugin::GM_Plugin()
    : QObject()
    , m_manager(0)
{
}

PluginSpec GM_Plugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "GreaseMonkey";
    spec.info = "Userscripts for QupZilla";
    spec.description = "Provides support for userscripts";
    spec.version = "0.9.4";
    spec.author = "David Rosca <nowrep@gmail.com>";
    spec.icon = QIcon(":gm/data/icon.svg").pixmap(32);
    spec.hasSettings = true;

    return spec;
}

void GM_Plugin::init(InitState state, const QString &settingsPath)
{
    m_manager = new GM_Manager(settingsPath, this);

    connect(mApp->plugins(), SIGNAL(mainWindowCreated(BrowserWindow*)), m_manager, SLOT(mainWindowCreated(BrowserWindow*)));
    connect(mApp->plugins(), SIGNAL(mainWindowDeleted(BrowserWindow*)), m_manager, SLOT(mainWindowDeleted(BrowserWindow*)));

    // Make sure userscripts works also with already created WebPages
    if (state == LateInitState) {
        foreach (BrowserWindow *window, mApp->windows()) {
            m_manager->mainWindowCreated(window);
        }
    }
}

void GM_Plugin::unload()
{
    m_manager->unloadPlugin();
    delete m_manager;
}

bool GM_Plugin::testPlugin()
{
    // Require the version that the plugin was built with
    return (Qz::VERSION == QLatin1String(QUPZILLA_VERSION));
}

QTranslator* GM_Plugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator(this);
    translator->load(locale, ":/gm/locale/");
    return translator;
}

void GM_Plugin::showSettings(QWidget* parent)
{
    m_manager->showSettings(parent);
}

bool GM_Plugin::acceptNavigationRequest(WebPage *page, const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    Q_UNUSED(page)
    Q_UNUSED(isMainFrame)

    if (type == QWebEnginePage::NavigationTypeLinkClicked && url.toString().endsWith(QLatin1String(".user.js"))) {
        m_manager->downloadScript(url);
        return false;
    }
    return true;
}
