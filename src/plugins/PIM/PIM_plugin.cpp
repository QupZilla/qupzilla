/* ============================================================
* Personal Information Manager plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
* Copyright (C) 2012  Mladen Pejaković <pejakm@gmail.com>
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
#include "PIM_plugin.h"
#include "PIM_handler.h"
#include "PIM_settings.h"
#include "mainapplication.h"
#include "pluginproxy.h"
#include "qupzilla.h"
#include "webview.h"

#include <QTranslator>

PIM_Plugin::PIM_Plugin()
    : QObject()
    , m_handler(0)
{
}

PluginSpec PIM_Plugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "PIM";
    spec.info = "Personal Information Manager";
    spec.description = "Adds ability for Qupzilla to store some personal data";
    spec.version = "0.1.1";
    spec.author = QString::fromUtf8("Mladen Pejaković <pejakm@gmail.com>");
    spec.icon = QPixmap(":/PIM/data/PIM.png");
    spec.hasSettings = true;

    return spec;
}

void PIM_Plugin::init(const QString &sPath)
{
    m_handler = new PIM_Handler(sPath, this);

    QZ_REGISTER_EVENT_HANDLER(PluginProxy::KeyPressHandler);

    connect(mApp->plugins(), SIGNAL(webPageCreated(WebPage*)), m_handler, SLOT(webPageCreated(WebPage*)));
}

void PIM_Plugin::unload()
{
    m_handler->deleteLater();
}

bool PIM_Plugin::testPlugin()
{
    // Let's be sure, require latest version of QupZilla

    return (QupZilla::VERSION == "1.3.1");
}

QTranslator* PIM_Plugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator();
    translator->load(locale, ":/PIM/locale/");
    return translator;
}

void PIM_Plugin::showSettings(QWidget* parent)
{
    m_handler->showSettings(parent);
}

void PIM_Plugin::populateWebViewMenu(QMenu* menu, WebView* view, const QWebHitTestResult &r)
{
    m_handler->populateWebViewMenu(menu, view, r);
}

bool PIM_Plugin::keyPress(const Qz::ObjectName &type, QObject* obj, QKeyEvent* event)
{
    if (type == Qz::ON_WebView) {
        WebView* view = qobject_cast<WebView*>(obj);
        return m_handler->keyPress(view, event);
    }

    return false;
}

Q_EXPORT_PLUGIN2(PIM, PIM_Plugin)
