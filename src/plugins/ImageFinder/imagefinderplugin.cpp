/* ============================================================
* ImageFinder plugin for QupZilla
* Copyright (C) 2016 Vladislav Tronko <innermous@gmail.com>
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
#include "imagefinderplugin.h"
#include "imagefindersettings.h"
#include "imagefinder.h"
#include "webview.h"
#include "webhittestresult.h"
#include "pluginproxy.h"
#include "mainapplication.h"
#include "enhancedmenu.h"

#include <QMenu>
#include <QTranslator>

ImageFinderPlugin::ImageFinderPlugin()
    : QObject()
    , m_finder(Q_NULLPTR)
{
}

PluginSpec ImageFinderPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "ImageFinder";
    spec.info = "Image Finder Plugin";
    spec.description = "Provides context menu with reverse image search engine support";
    spec.version = "0.2.0";
    spec.author = "Vladislav Tronko <innermous@gmail.com>";
    spec.icon = QPixmap(":/imgfinder/data/icon.png");
    spec.hasSettings = true;

    return spec;
}

void ImageFinderPlugin::init(InitState state, const QString &settingsPath)
{
    m_finder = new ImageFinder(settingsPath + QL1S("/extensions.ini"), this);

    Q_UNUSED(state);
}

void ImageFinderPlugin::unload()
{

}

bool ImageFinderPlugin::testPlugin()
{
    // Require the version that the plugin was built with
    return (Qz::VERSION == QLatin1String(QUPZILLA_VERSION));
}

QTranslator *ImageFinderPlugin::getTranslator(const QString &locale)
{
    QTranslator *translator = new QTranslator(this);
    translator->load(locale, QSL(":/imgfinder/locale/"));
    return translator;
}

void ImageFinderPlugin::showSettings(QWidget *parent)
{
    if (!m_settings) {
        m_settings = new ImageFinderSettings(m_finder, parent);
    }

    m_settings.data()->show();
    m_settings.data()->raise();
}

void ImageFinderPlugin::populateWebViewMenu(QMenu *menu, WebView *view, const WebHitTestResult &r)
{
    // Don't let local files fool you
    if (r.imageUrl().scheme() != QL1S("http") && r.imageUrl().scheme() != QL1S("https")) return;

    if (!r.imageUrl().isEmpty()) {
        QString engineName = m_finder->searchEngineName();
        Action* action = new Action(tr("Search image in ") + engineName);
        action->setIcon(QIcon(QSL(":/imgfinder/data/%1.png").arg(engineName.toLower())));
        action->setData(m_finder->getSearchQuery(r.imageUrl()));
        connect(action, SIGNAL(triggered()), view, SLOT(openUrlInSelectedTab()));
        connect(action, SIGNAL(ctrlTriggered()), view, SLOT(openUrlInBackgroundTab()));
        menu->addAction(action);
    }
}
