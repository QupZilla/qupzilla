/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2013  David Rosca <nowrep@gmail.com>
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
#include "tabwidget.h"
#include "webtab.h"
#include "tabbedwebview.h"

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
    spec.version = "0.4.3";
    spec.author = "David Rosca <nowrep@gmail.com>";
    spec.icon = QPixmap(":gm/data/icon.png");
    spec.hasSettings = true;

    return spec;
}

void GM_Plugin::init(InitState state, const QString &settingsPath)
{
    m_manager = new GM_Manager(settingsPath, this);
    m_settingsPath = settingsPath;

    connect(mApp->plugins(), SIGNAL(webPageCreated(WebPage*)), this, SLOT(webPageCreated(WebPage*)));
    connect(mApp->plugins(), SIGNAL(mainWindowCreated(QupZilla*)), m_manager, SLOT(mainWindowCreated(QupZilla*)));
    connect(mApp->plugins(), SIGNAL(mainWindowDeleted(QupZilla*)), m_manager, SLOT(mainWindowDeleted(QupZilla*)));

    // Make sure userscripts works also with already created WebPages
    if (state == LateInitState) {
        foreach (QupZilla* window, mApp->mainWindows()) {
            m_manager->mainWindowCreated(window);

            for (int i = 0; i < window->tabWidget()->count(); ++i) {
                WebTab* tab = qobject_cast<WebTab*>(window->tabWidget()->widget(i));
                if (tab) {
                    webPageCreated(tab->view()->page());
                }
            }
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
    return (QupZilla::VERSION == QLatin1String("1.6.5"));
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
        const QString urlString = request.url().toString(QUrl::RemoveFragment | QUrl::RemoveQuery);

        if (urlString.endsWith(QLatin1String(".user.js"))) {
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

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(GreaseMonkey, GM_Plugin)
#endif
