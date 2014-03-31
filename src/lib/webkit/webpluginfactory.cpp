/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "webpluginfactory.h"
#include "clicktoflash.h"
#include "mainapplication.h"
#include "pluginproxy.h"
#include "adblockmanager.h"
#include "webpage.h"

#include <QNetworkRequest>

WebPluginFactory::WebPluginFactory(WebPage* page)
    : QWebPluginFactory(page)
    , m_page(page)
{
}

QObject* WebPluginFactory::create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const
{
    if (url.isEmpty()) {
        return new QObject();
    }

    // AdBlock
    AdBlockManager* manager = AdBlockManager::instance();
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::Attribute(QNetworkRequest::User + 150), QString("object"));
    if (manager->isEnabled() && manager->block(request)) {
        return new QObject();
    }

    QString mime = mimeType.trimmed(); // Fixing bad behaviour when mimeType contains spaces
    if (mime.isEmpty()) {
        if (url.toString().endsWith(QLatin1String(".swf"))) {
            mime = "application/x-shockwave-flash";
        }
        else {
            return 0;
        }
    }

    if (mime != QLatin1String("application/x-shockwave-flash")) {
        if (mime != QLatin1String("application/futuresplash") && mime != QLatin1String("application/x-java-applet")) {
            qDebug()  << "WebPluginFactory::create creating object of mimeType : " << mime;
        }
        return 0;
    }

    if (!mApp->plugins()->c2f_isEnabled()) {
        return 0;
    }

    // Click2Flash whitelist
    QStringList whitelist = mApp->plugins()->c2f_getWhiteList();
    if (whitelist.contains(url.host()) || whitelist.contains("www." + url.host()) || whitelist.contains(url.host().remove(QLatin1String("www.")))) {
        return 0;
    }

    // Click2Flash already accepted
    if (ClickToFlash::isAlreadyAccepted(url, argumentNames, argumentValues)) {
        return 0;
    }

    ClickToFlash* ctf = new ClickToFlash(url, argumentNames, argumentValues, m_page);
    return ctf;
}

QList<QWebPluginFactory::Plugin> WebPluginFactory::plugins() const
{
    QList<Plugin> plugins;
    return plugins;

//    QList<QWebPluginFactory::Plugin> plugins;
//    QWebPluginFactory::Plugin plugin;
//    QWebPluginFactory::MimeType mimeType;

//    mimeType.fileExtensions << "swf";
//    mimeType.name = "application/x-shockwave-flash";

//    plugin.name = "ClickToFlashPlugin";
//    plugin.mimeTypes.append(mimeType);
//    plugins.append(plugin);
//    return plugins;
}
