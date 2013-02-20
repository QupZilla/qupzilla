/* ============================================================
* Private Window Plugin for QupZilla
* Copyright (C) 2013  Oliver Gerlich <oliver.gerlich@gmx.de>
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
#include "privatewindowplugin.h"
#include "qupzilla.h"
#include "webview.h"
#include "pluginproxy.h"
#include "mainapplication.h"

#include <QTranslator>
#include <QWebHitTestResult>
#include <QMenu>

PrivateWindowPlugin::PrivateWindowPlugin()
    : QObject()
{
}

PluginSpec PrivateWindowPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "Private Window Plugin";
    spec.info = "Open in new Private Window Plugin";
    spec.description = "Provides a context menu item to open link in new private window";
    spec.version = "0.1";
    spec.author = "Oliver Gerlich <oliver.gerlich@gmx.de>";
    spec.hasSettings = false;

    return spec;
}

void PrivateWindowPlugin::init(const QString &/*sPath*/)
{
}

void PrivateWindowPlugin::unload()
{
}

bool PrivateWindowPlugin::testPlugin()
{
    return (QupZilla::VERSION == QLatin1String("1.3.5"));
}

QTranslator* PrivateWindowPlugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator(this);
    translator->load(locale, ":/privatewindowplugin/locale/");
    return translator;
}

void PrivateWindowPlugin::populateWebViewMenu(QMenu* menu, WebView* /*view*/, const QWebHitTestResult &r)
{
    if (!r.linkUrl().isEmpty()) {
        // newAction will be deleted automatically when menu is deleted:
        QAction* newAction = new QAction(tr("Open link in new &private browser"), menu);
        newAction->connect(newAction, SIGNAL(triggered()), this, SLOT(openUrlInNewPrivateBrowser()));
        newAction->setData(r.linkUrl());

        // insert action before second separator in menu:
        int i = 0;
        foreach (QAction* action, menu->actions()) {
            if (action->isSeparator() && i > 0) {
                menu->insertAction(action, newAction);
                break;
            }
            i++;
        }
    }
}

void PrivateWindowPlugin::openUrlInNewPrivateBrowser()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        mApp->startPrivateBrowsing(action->data().toUrl());
    }
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(PrivateWindow, PrivateWindowPlugin)
#endif
