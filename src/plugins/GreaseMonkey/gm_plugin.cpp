/* ============================================================
* GreaseMonkey plugin for QupZilla
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
#include "gm_plugin.h"
#include "gm_manager.h"
#include "qupzilla.h"
#include "webpage.h"
#include "pluginproxy.h"
#include "mainapplication.h"
#include "emptynetworkreply.h"

#include <QTranslator>
#include <QNetworkRequest>

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
    spec.description = "Provides support for userscripts (www.userscripts.org)";
    spec.version = "0.2.1";
    spec.author = "David Rosca <nowrep@gmail.com>";
    spec.icon = QPixmap(":gm/data/icon.png");
    spec.hasSettings = true;

    return spec;
}

void GM_Plugin::init(const QString &sPath)
{
    m_manager = new GM_Manager(sPath, this);
    m_settingsPath = sPath;

    connect(mApp->plugins(), SIGNAL(webPageCreated(WebPage*)), this, SLOT(webPageCreated(WebPage*)));
}

void GM_Plugin::unload()
{
    m_manager->saveSettings();
    delete m_manager;
}

bool GM_Plugin::testPlugin()
{
    return (QupZilla::VERSION == "1.3.1");
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

QNetworkReply* GM_Plugin::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
{
    Q_UNUSED(outgoingData)

    if (op == QNetworkAccessManager::GetOperation && request.rawHeader("X-QupZilla-UserLoadAction") == QByteArray("1")) {
        const QString &urlString = request.url().toString(QUrl::RemoveFragment | QUrl::RemoveQuery);

        if (urlString.endsWith(".user.js")) {
            m_manager->downloadScript(request);
            return new EmptyNetworkReply;
        }
    }

    return 0;
}

void GM_Plugin::webPageCreated(WebPage* page)
{
    connect(page->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), m_manager, SLOT(pageLoadStart()));
}

Q_EXPORT_PLUGIN2(GreaseMonkey, GM_Plugin)
