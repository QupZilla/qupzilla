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
#include "proxyautoconfig.h"
#include "pacdatetime.h"
#include "qztools.h"
#include "qzregexp.h"

#include <QScriptEngine>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QHostInfo>
#include <QRegExp>

/**
 * Class implementing the proxy auto-configuration (PAC) JavaScript api.
 *
 * Based on qt-examples: https://gitorious.org/qt-examples/qt-examples/blobs/master/pac-files
 */
ProxyAutoConfig::ProxyAutoConfig(QObject* parent)
    : QObject(parent)
    , m_engine(new QScriptEngine(this))
{
    install();
}

void ProxyAutoConfig::setConfig(const QString &config)
{
    m_engine->evaluate(config);
}

// string findProxyForUrl url host
QString ProxyAutoConfig::findProxyForUrl(const QString &url, const QString &host)
{
    QScriptValue global = m_engine->globalObject();
    QScriptValue fun = global.property("FindProxyForURL");
    if (!fun.isFunction()) {
        return QString("DIRECT");
    }

    QScriptValueList args;
    args << m_engine->toScriptValue(url) << m_engine->toScriptValue(host);

    QScriptValue val = fun.call(global, args);

    if (val.isError()) {
        qWarning() << "PAC Error:" << val.toString();
        return QString("DIRECT");
    }

    return val.toString();
}

QScriptValue ProxyAutoConfig::evaluate(const QString &source)
{
    return m_engine->evaluate(source);
}

void ProxyAutoConfig::install()
{
    QScriptValue globalObject = m_engine->globalObject();

    QScriptValue fun;

    fun = m_engine->newFunction(debug);
    globalObject.setProperty("debug", fun);

    fun = m_engine->newFunction(isPlainHostName);
    globalObject.setProperty("isPlainHostName", fun);

    fun = m_engine->newFunction(dnsDomainIs);
    globalObject.setProperty("dnsDomainIs", fun);

    fun = m_engine->newFunction(localHostOrDomainIs);
    globalObject.setProperty("localHostOrDomainIs", fun);

    fun = m_engine->newFunction(isResolvable);
    globalObject.setProperty("isResolvable", fun);

    fun = m_engine->newFunction(isInNet);
    globalObject.setProperty("isInNet", fun);

    fun = m_engine->newFunction(dnsResolve);
    globalObject.setProperty("dnsResolve", fun);

    fun = m_engine->newFunction(myIpAddress);
    globalObject.setProperty("myIpAddress", fun);

    fun = m_engine->newFunction(dnsDomainLevels);
    globalObject.setProperty("dnsDomainLevels", fun);

    fun = m_engine->newFunction(shExpMatch);
    globalObject.setProperty("shExpMatch", fun);

    m_engine->evaluate(PAC_DATETIME_JAVASCRIPT);
}

QScriptValue ProxyAutoConfig::debug(QScriptContext* context, QScriptEngine* engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError("Debug takes one argument");
    }
    qDebug() << context->argument(0).toString();
    return engine->undefinedValue();
}

// bool isPlainHostName host
QScriptValue ProxyAutoConfig::isPlainHostName(QScriptContext* context, QScriptEngine* engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError("isPlainHostName takes one argument");
    }

    bool ret = !context->argument(0).toString().contains(QLatin1Char('.'));
    return QScriptValue(engine, ret);
}

// bool dnsDomainIs host domain
QScriptValue ProxyAutoConfig::dnsDomainIs(QScriptContext* context, QScriptEngine* engine)
{
    if (context->argumentCount() != 2) {
        return context->throwError("dnsDomainIs takes two arguments");
    }

    QString host = context->argument(0).toString();
    QString domain = context->argument(1).toString();

    if (host.startsWith(QLatin1Char('.'))) {
        host = host.mid(1);
    }

    if (domain.startsWith(QLatin1Char('.'))) {
        domain = domain.mid(1);
    }

    return QScriptValue(engine, QzTools::matchDomain(domain, host));
}

// bool localHostOrDomainIs host hostdom
QScriptValue ProxyAutoConfig::localHostOrDomainIs(QScriptContext* context, QScriptEngine* engine)
{
    if (context->argumentCount() != 2) {
        return context->throwError("localHostOrDomainIs takes two arguments");
    }

    QString host = context->argument(0).toString();
    QString hostdom = context->argument(1).toString();
    bool ret = !host.contains(QLatin1Char('.')) ? hostdom.startsWith(host) : host == hostdom;

    return QScriptValue(engine, ret);
}

static QList<QHostAddress> hostResolve(const QString &host)
{
    QHostInfo info = QHostInfo::fromName(host);
    return info.addresses();
}

// bool isResolvable host
QScriptValue ProxyAutoConfig::isResolvable(QScriptContext* context, QScriptEngine* engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError("isResolvable takes one arguments");
    }

    QString host = context->argument(0).toString();
    return QScriptValue(engine, !hostResolve(host).isEmpty());
}

// bool isInNet host pattern mask
QScriptValue ProxyAutoConfig::isInNet(QScriptContext* context, QScriptEngine* engine)
{
    if (context->argumentCount() != 3) {
        return context->throwError("isInNet takes three arguments");
    }

    QHostAddress host(context->argument(0).toString());
    QHostAddress pattern(context->argument(1).toString());
    QHostAddress mask(context->argument(2).toString());

    if (host.isNull()) {
        QList<QHostAddress> addresses = hostResolve(context->argument(0).toString());
        host = addresses.isEmpty() ? QHostAddress() : addresses.first();
    }

    if ((pattern.toIPv4Address() & mask.toIPv4Address()) == (host.toIPv4Address() & mask.toIPv4Address())) {
        return QScriptValue(engine, true);
    }

    return QScriptValue(engine, false);
}

// string dnsResolve hostname
QScriptValue ProxyAutoConfig::dnsResolve(QScriptContext* context, QScriptEngine* engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError("dnsResolve takes one arguments");
    }

    QString host = context->argument(0).toString();
    QList<QHostAddress> addresses = hostResolve(host);
    if (addresses.isEmpty()) {
        return engine->nullValue();
    }

    return QScriptValue(engine, addresses.first().toString());
}

// string myIpAddress
QScriptValue ProxyAutoConfig::myIpAddress(QScriptContext* context, QScriptEngine* engine)
{
    if (context->argumentCount() != 0) {
        return context->throwError("myIpAddress takes no arguments");
    }

    foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
        if (address != QHostAddress::LocalHost && address != QHostAddress::LocalHostIPv6)
            return QScriptValue(engine, address.toString());
    }

    return engine->undefinedValue();
}

// int dnsDomainLevels host
QScriptValue ProxyAutoConfig::dnsDomainLevels(QScriptContext* context, QScriptEngine* engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError("dnsDomainLevels takes one argument");
    }

    QString host = context->argument(0).toString();

    return QScriptValue(engine, host.count(QLatin1Char('.')));
}

// bool shExpMatch str shexp
QScriptValue ProxyAutoConfig::shExpMatch(QScriptContext* context, QScriptEngine* engine)
{
    if (context->argumentCount() != 2) {
        return context->throwError("shExpMatch takes two arguments");
    }

    QString str = context->argument(0).toString();
    QString shexp = context->argument(1).toString();

    shexp.replace(QLatin1Char('.'), QLatin1String("\\."))
    .replace(QLatin1Char('*'), QLatin1String(".*"))
    .replace(QLatin1Char('?'), QLatin1Char('.'));
    shexp = QString("^%1$").arg(shexp);

    QzRegExp re(shexp);
    bool ret = re.indexIn(str) != -1;

    return QScriptValue(engine, ret);
}
