/* ============================================================
* TLDExtractor, a simple Qt interface to extract TLD part of a host
* Copyright (C) 2014  Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#include "tldextractor.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QUrl>

TLDExtractor* TLDExtractor::s_instance = 0;

TLDExtractor::TLDExtractor(QObject* parent)
    : QObject(parent)
{
    setDataSearchPaths();
}

QStringList TLDExtractor::defaultDataSearchPaths()
{
    return QStringList() << QLatin1String(":/tldextractor/data");
}

TLDExtractor* TLDExtractor::instance()
{
    if(s_instance == 0)
    {
        s_instance = new TLDExtractor(qApp);
    }

    return s_instance;
}

TLDExtractor::~TLDExtractor()
{
    s_instance = 0;
}

bool TLDExtractor::isDataLoaded()
{
    return !m_tldHash.isEmpty();
}

QString TLDExtractor::TLD(const QString &host)
{
    if (host.isEmpty() || host.startsWith(QLatin1Char('.'))) {
        return QString();
    }

    QString cleanHost = normalizedHost(host);

    QString tldPart = cleanHost.mid(cleanHost.lastIndexOf(QLatin1Char('.')) + 1);
    cleanHost = QString::fromUtf8(QUrl::toAce(cleanHost));

    loadData();

    if (!m_tldHash.contains(tldPart)) {
        return tldPart;
    }

    QStringList tldRules = m_tldHash.values(tldPart);

    if (!tldRules.contains(tldPart)) {
        tldRules << tldPart;
    }

    int maxLabelCount = 0;
    bool isExceptionTLD = false;
    bool isWildcardTLD = false;

    foreach(QString rule, tldRules) {
        const int labelCount = rule.count(QLatin1Char('.')) + 1;

        if (rule.startsWith(QLatin1Char('!'))) {
            rule = rule.remove(0, 1);

            rule = QString::fromUtf8(QUrl::toAce(rule));
            isExceptionTLD = true;

            // matches with exception TLD
            if (cleanHost.endsWith(rule)) {
                tldPart = rule.mid(rule.indexOf(QLatin1Char('.')) + 1);
                break;
            }
        }
        else {
            isExceptionTLD = false;
        }

        if (rule.startsWith(QLatin1Char('*'))) {
            rule.remove(0, 1);

            if (rule.startsWith(QLatin1Char('.'))) {
                rule.remove(0, 1);
            }

            isWildcardTLD = true;
        }
        else {
            isWildcardTLD = false;
        }

        Q_UNUSED(isExceptionTLD)

        rule = QString::fromUtf8(QUrl::toAce(rule));
        const QString testRule = QLatin1Char('.') + rule;
        const QString testUrl = QLatin1Char('.') + cleanHost;

        if (labelCount > maxLabelCount && testUrl.endsWith(testRule)) {
            tldPart = rule;
            maxLabelCount = labelCount;

            if (isWildcardTLD) {
                QString temp = cleanHost;
                temp.remove(temp.lastIndexOf(tldPart), tldPart.size());

                if (temp.endsWith(QLatin1Char('.'))) {
                    temp.remove(temp.size() - 1, 1);
                }

                temp = temp.mid(temp.lastIndexOf(QLatin1Char('.')) + 1);

                tldPart = temp.isEmpty() ? rule : (temp + "." + rule);
            }
        }
    }

    QString temp = normalizedHost(host);
    tldPart = temp.section(QLatin1Char('.'), temp.count(QLatin1Char('.')) - tldPart.count(QLatin1Char('.')));

    return tldPart;
}

QString TLDExtractor::domain(const QString &host)
{
    const QString tldPart = TLD(host);

    return domainHelper(host, tldPart);
}

QString TLDExtractor::domainHelper(const QString &host, const QString &tldPart)
{
    if (host.isEmpty() || tldPart.isEmpty()) {
        return QString();
    }

    QString temp = normalizedHost(host);
    temp.remove(temp.lastIndexOf(tldPart), tldPart.size());

    if (temp.endsWith(QLatin1Char('.'))) {
        temp.remove(temp.size() - 1, 1);
    }

    return temp.mid(temp.lastIndexOf(QLatin1Char('.')) + 1);
}

QString TLDExtractor::registrableDomainHelper(const QString &domainPart, const QString &tldPart)
{
    if (tldPart.isEmpty() || domainPart.isEmpty()) {
        return QString();
    }
    else {
        return QString("%1.%2").arg(domainPart).arg(tldPart);
    }
}

QString TLDExtractor::subdomainHelper(const QString &host, const QString &registrablePart)
{
    if (!registrablePart.isEmpty()) {
        QString subdomain = normalizedHost(host);

        subdomain.remove(subdomain.lastIndexOf(registrablePart), registrablePart.size());

        if (subdomain.endsWith(QLatin1Char('.'))) {
            subdomain.remove(subdomain.size() - 1, 1);
        }

        return subdomain;
    }

    return QString();
}

QString TLDExtractor::registrableDomain(const QString &host)
{
    const QString tldPart = TLD(host);

    return registrableDomainHelper(domainHelper(host, tldPart), tldPart);
}

QString TLDExtractor::subdomain(const QString &host)
{
    return subdomainHelper(host, registrableDomain(host));
}

// a light function that extract all parts with just one call to TLD()
TLDExtractor::HostParts TLDExtractor::splitParts(const QString &host)
{
    HostParts hostParts;

    hostParts.host = host;
    hostParts.tld = TLD(host);
    hostParts.domain = domainHelper(host, hostParts.tld);
    hostParts.registrableDomain = registrableDomainHelper(hostParts.domain, hostParts.tld);
    hostParts.subdomain = subdomainHelper(host, hostParts.registrableDomain);

    return hostParts;
}

QStringList TLDExtractor::dataSearchPaths() const
{
    return m_dataSearchPaths;
}

void TLDExtractor::setDataSearchPaths(const QStringList &searchPaths)
{
    m_dataSearchPaths = searchPaths;

    m_dataSearchPaths << TLDExtractor::defaultDataSearchPaths();

    m_dataSearchPaths.removeDuplicates();
}

void TLDExtractor::loadData()
{
    if (isDataLoaded()) {
        return;
    }

    QString dataFileName;
    bool parsedDataFileExist = false;

    foreach(const QString &path, m_dataSearchPaths) {
        dataFileName = QFileInfo(path + QLatin1String("/effective_tld_names.dat")).absoluteFilePath();

        if (QFileInfo(dataFileName).exists()) {
            parsedDataFileExist = true;
            break;
        }
    }


    if (!parsedDataFileExist) {
        const QString tldDataFileDownloadLink = QLatin1String("http://mxr.mozilla.org/mozilla-central/source/netwerk/dns/effective_tld_names.dat?raw=1");
        QMessageBox::information(0, tr("File not found!"),
                                 tr("File \'effective_tld_names.dat\' was not found!\n"
                                    "You can download it from \'<a href=\"%1\"><b>here</b></a>\' to one of the following paths:\n%2")
                                 .arg(tldDataFileDownloadLink).arg(m_dataSearchPaths.join("\n")));

        return;
    }

    m_dataFileName = dataFileName;

    if (!parseData(dataFileName)) {
        qWarning() << "TLDExtractor: There is some parse errors for file:" << dataFileName;
    }
}

bool TLDExtractor::parseData(const QString &dataFile, bool loadPrivateDomains)
{
    m_tldHash.clear();

    QFile file(dataFile);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }

    bool seekToEndOfPrivateDomains = false;

    while (!file.atEnd()) {
        QString line = QString::fromUtf8(file.readLine().constData()).simplified();

        if (line.isEmpty()) {
            continue;
        }

        if (line.startsWith(QLatin1Char('.'))) {
            line.remove(0, 1);
        }


        if (line.startsWith(QLatin1String("//"))) {
            if (line.contains(QLatin1String("===END PRIVATE DOMAINS==="))) {
                seekToEndOfPrivateDomains = false;
            }

            if (!loadPrivateDomains && line.contains(QLatin1String("===BEGIN PRIVATE DOMAINS==="))) {
                if (m_tldHash.isEmpty()) {
                    seekToEndOfPrivateDomains = true;
                }
                else {
                    break;
                }
            }

            continue;
        }

        if (seekToEndOfPrivateDomains) {
            continue;
        }

        // Each line is only read up to the first whitespace
        line = line.left(line.indexOf(QLatin1Char(' ')));

        if (!line.contains(QLatin1Char('.'))) {
            m_tldHash.insertMulti(line, line);
        }
        else {
            QString key = line.mid(line.lastIndexOf(QLatin1Char('.')) + 1);

            m_tldHash.insertMulti(key, line);
        }
    }

    return isDataLoaded();
}

QString TLDExtractor::normalizedHost(const QString &host) const
{
    return host.toLower();
}

// methods for testing
bool TLDExtractor::test()
{
    if (!parseData(m_dataFileName, true)) {
        return false;
    }

    QString testDataFileName;
    bool testDataFileExist = false;

    foreach(const QString &path, m_dataSearchPaths) {
        testDataFileName = QFileInfo(path + QLatin1String("/test_psl.txt")).absoluteFilePath();

        if (QFileInfo(testDataFileName).exists()) {
            testDataFileExist = true;
            break;
        }
    }

    if (!testDataFileExist) {
        const QString testFileDownloadLink = QLatin1String("http://mxr.mozilla.org/mozilla-central/source/netwerk/test/unit/data/test_psl.txt?raw=1");

        QMessageBox::information(0, tr("File not found!"),
                                 tr("File \'test_psl.txt\' was not found!\n"
                                    "You can download it from \'<a href=\"%1\"><b>here</b></a>\' to one of the following paths:\n%2")
                                 .arg(testFileDownloadLink).arg(m_dataSearchPaths.join("\n")));

        return false;
    }

    QFile file(testDataFileName);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }

    QRegExp testRegExp("checkPublicSuffix\\(('([^']+)'|null), ('([^']+)'|null)\\);");
    bool allTestSuccess = true;

    while (!file.atEnd()) {
        QString line = QString::fromUtf8(file.readLine().constData()).simplified();

        if (line.startsWith(QLatin1String("//")) || line.isEmpty()) {
            continue;
        }

        line.indexOf(testRegExp);

        const QString hostName = testRegExp.cap(2);
        const QString registrableName = testRegExp.cap(4);

        if (!checkPublicSuffix(hostName, registrableName)) {
            allTestSuccess = false;
        }
    }

    if (allTestSuccess) {
        qWarning() << "TLDExtractor: Test passed successfully.";
    }
    else {
        qWarning() << "TLDExtractor: Test finished with some errors!";
    }

    // reset cache for normal use
    m_tldHash.clear();

    return allTestSuccess;
}

bool TLDExtractor::checkPublicSuffix(const QString &hostName, const QString &registrableName)
{
    if (registrableDomain(hostName) != registrableName) {
        qWarning() << "TLDExtractor Test Error: hostName:" << hostName
                   << "Correct registrableName:" << registrableName
                   << "Wrong registrableName:" << registrableDomain(hostName);

        return false;
    }

    return true;
}
