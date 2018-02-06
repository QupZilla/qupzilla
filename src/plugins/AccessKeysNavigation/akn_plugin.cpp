/* ============================================================
* Access Keys Navigation plugin for QupZilla
* Copyright (C) 2012-2014  David Rosca <nowrep@gmail.com>
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
#include "akn_plugin.h"
#include "akn_handler.h"
#include "akn_settings.h"
#include "mainapplication.h"
#include "pluginproxy.h"
#include "browserwindow.h"

#include <QTranslator>

AKN_Plugin::AKN_Plugin()
    : QObject()
    , m_handler(0)
{
}

PluginSpec AKN_Plugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "Access Keys Navigation";
    spec.info = "Access keys navigation for QupZilla";
    spec.description = "Provides support for navigating in webpages by keyboard shortcuts";
    spec.version = "0.4.3";
    spec.author = "David Rosca <nowrep@gmail.com>";
    spec.icon = QPixmap(":/accesskeysnavigation/data/icon.png");
    spec.hasSettings = true;

    return spec;
}

void AKN_Plugin::init(InitState state, const QString &sPath)
{
    Q_UNUSED(state)

    m_handler = new AKN_Handler(sPath, this);

    mApp->plugins->registerAppEventHandler(PluginProxy::KeyPressHandler, this);
}

void AKN_Plugin::unload()
{
    delete m_settings.data();

    m_handler->deleteLater();
}

bool AKN_Plugin::testPlugin()
{
    // Require the version that the plugin was built with
    return (Qz::VERSION == QLatin1String(QUPZILLA_VERSION));
}

QTranslator* AKN_Plugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator(this);
    translator->load(locale, ":/accesskeysnavigation/locale/");
    return translator;
}

void AKN_Plugin::showSettings(QWidget* parent)
{
    if (!m_settings) {
        m_settings = new AKN_Settings(m_handler, parent);
    }

    m_settings.data()->show();
    m_settings.data()->raise();
}

bool AKN_Plugin::keyPress(const Qz::ObjectName &type, QObject* obj, QKeyEvent* event)
{
    if (type == Qz::ON_WebView) {
        return m_handler->handleKeyPress(obj, event);
    }

    return false;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(AccessKeysNavigation, AKN_Plugin)
#endif
