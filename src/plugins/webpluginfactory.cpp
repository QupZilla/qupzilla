/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "webpage.h"

WebPluginFactory::WebPluginFactory(QObject* parent)
    : QWebPluginFactory(parent)
    , m_page(0)
{
}

QObject* WebPluginFactory::create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const
{
    QString mime = mimeType.trimmed(); //Fixing bad behaviour when mimeType contains spaces
    if (mime.isEmpty()) {
        return 0;
    }

    if (mime != "application/x-shockwave-flash") {
        if (mime != "application/futuresplash" && mime != "application/x-java-applet") {
            qDebug()  << "WebPluginFactory::create missing mimeType handler for: " << mime;
        }
        return 0;
    }

    if (!mApp->plugins()->c2f_isEnabled()) {
        return 0;
    }

    // Click2Flash whitelist
    QStringList whitelist = mApp->plugins()->c2f_getWhiteList();
    if (whitelist.contains(url.host()) || whitelist.contains("www." + url.host()) || whitelist.contains(url.host().remove("www."))) {
        return 0;
    }

    // Click2Flash already accepted
    if (ClickToFlash::isAlreadyAccepted(url, argumentNames, argumentValues)) {
        return 0;
    }

    WebPluginFactory* factory = const_cast<WebPluginFactory*>(this);
    if (!factory) {
        return 0;
    }

    WebPage* page = factory->parentPage();
    if (!page) {
        return 0;
    }


    ClickToFlash* ctf = new ClickToFlash(url, argumentNames, argumentValues, page);
    return ctf;
}

WebPage* WebPluginFactory::parentPage()
{
    if (m_page) {
        return m_page;
    }

    WebPage* page = qobject_cast<WebPage*> (parent());
    return page;
}

QList<QWebPluginFactory::Plugin> WebPluginFactory::plugins() const
{
//    QList<QWebPluginFactory::Plugin> plugins;
//    return plugins;

//    QWebPluginFactory::Plugin plugin;
//    plugin.name = QLatin1String("ClickToFlashPlugin");               //   Mmmm, it should return this,
//    QWebPluginFactory::MimeType mimeType;                            //   but with WebKit 533.3, click2flash
//    mimeType.fileExtensions << QLatin1String("swf");                 //   fails on some pages, like youtube.com
//    mimeType.name = QLatin1String("application/x-shockwave-flash");  //   so we will return empty QList
//    plugin.mimeTypes.append(mimeType);                               //   On some pages it also force to load non-flash
//    plugins.append(plugin);                                          //   content -> in most cases advertisements.
//    return plugins;                                                  //   Not bad to have it hidden :-)

    QList<QWebPluginFactory::Plugin> plugins;
    QWebPluginFactory::Plugin plugin;
    QWebPluginFactory::MimeType mimeType;

    mimeType.fileExtensions << "swf";
    mimeType.name = "application/x-shockwave-flash";

    plugin.name = "ClickToFlashPlugin";
    plugin.mimeTypes.append(mimeType);
    plugins.append(plugin);
    return plugins;
}
