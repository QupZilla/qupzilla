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
#ifndef PACTEST_H
#define PACTEST_H

#include <QObject>

#include "pac/proxyautoconfig.h"

class ProxyAutoConfig_Tst : public ProxyAutoConfig
{
public:
    QScriptValue evaluate(const QString &source)
    {
        return ProxyAutoConfig::evaluate(source);
    }
};

class PacTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void isPlainHostNameTest_data();
    void isPlainHostNameTest();

    void dnsDomainIsTest_data();
    void dnsDomainIsTest();

    void localHostOrDomainIs_data();
    void localHostOrDomainIs();

    void isResolvableTest_data();
    void isResolvableTest();

    void isInNetTest_data();
    void isInNetTest();

    void dnsResolveTest_data();
    void dnsResolveTest();

    // myIpAddress - how to test it?

    void dnsDomainLevelsTest_data();
    void dnsDomainLevelsTest();

    void shExpMatchTest_data();
    void shExpMatchTest();

    void dateTimeTest();

private:
    ProxyAutoConfig_Tst *m_runner;
};

#endif // PACTEST_H
