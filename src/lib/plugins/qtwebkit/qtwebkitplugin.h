/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#ifndef QTWEBKITPLUGIN_H
#define QTWEBKITPLUGIN_H

#include "qwebkitplatformplugin.h"

class QtWebKitPlugin : public QObject, public QWebKitPlatformPlugin
{
    Q_OBJECT
    Q_INTERFACES(QWebKitPlatformPlugin)

#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qtwebkit.QtWebKit.QtWebKitPlugins")
#endif

public:
    explicit QtWebKitPlugin();

    bool supportsExtension(Extension ext) const;
    QObject* createExtension(Extension ext) const;
};

#endif // QTWEBKITPLUGIN_H
