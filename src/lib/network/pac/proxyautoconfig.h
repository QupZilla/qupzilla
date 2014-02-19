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
#ifndef PROXYAUTOCONFIG_H
#define PROXYAUTOCONFIG_H

#include <QObject>
#include <QScriptValue>

#include "qz_namespace.h"

class QScriptContext;
class QScriptEngine;

/**
 * Class implementing the proxy auto-configuration (PAC) JavaScript api.
 *
 * Based on qt-examples: https://gitorious.org/qt-examples/qt-examples/blobs/master/pac-files
 */
class QUPZILLA_EXPORT ProxyAutoConfig : public QObject
{
    Q_OBJECT

public:
    explicit ProxyAutoConfig(QObject* parent = 0);

    // Call this to set the script to be executed. Note that the argument should be
    // the content of the .pac file to be used, not the URL where it is located.
    void setConfig(const QString &config);

    // Returns the result
    QString findProxyForUrl(const QString &url, const QString &host);

protected:
    QScriptValue evaluate(const QString &source);

private:
    void install();

    // Debug
    static QScriptValue debug(QScriptContext* context, QScriptEngine* engine);

    // Hostname based conditions
    static QScriptValue isPlainHostName(QScriptContext* context, QScriptEngine* engine);
    static QScriptValue dnsDomainIs(QScriptContext* context, QScriptEngine* engine);
    static QScriptValue localHostOrDomainIs(QScriptContext* context, QScriptEngine* engine);
    static QScriptValue isResolvable(QScriptContext* context, QScriptEngine* engine);
    static QScriptValue isInNet(QScriptContext* context, QScriptEngine* engine);

    // Related utility functions
    static QScriptValue dnsResolve(QScriptContext* context, QScriptEngine* engine);
    static QScriptValue myIpAddress(QScriptContext* context, QScriptEngine* engine);
    static QScriptValue dnsDomainLevels(QScriptContext* context, QScriptEngine* engine);

    // URL/hostname based conditions
    static QScriptValue shExpMatch(QScriptContext* context, QScriptEngine* engine);

    // Time based conditions
    // Implemented in JavaScript

private:
    QScriptEngine* m_engine;
};

#endif // PROXYAUTOCONFIG_H
