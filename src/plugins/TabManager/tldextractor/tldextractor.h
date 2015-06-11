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
#ifndef TLDEXTRACTOR_H
#define TLDEXTRACTOR_H

#define TLDExtractor_Version "1.0"

#include <QHash>
#include <QObject>
#include <QStringList>

class TLDExtractor : public QObject
{
    Q_OBJECT

public:
    static TLDExtractor* instance();
    ~TLDExtractor();

    bool isDataLoaded();

    struct HostParts {
        QString host;
        QString tld;
        QString domain;
        QString registrableDomain;
        QString subdomain;
    };

    QString TLD(const QString &host);
    QString domain(const QString &host);
    QString registrableDomain(const QString &host);
    QString subdomain(const QString &host);

    HostParts splitParts(const QString &host);

    QStringList dataSearchPaths() const;
    void setDataSearchPaths(const QStringList &searchPaths = TLDExtractor::defaultDataSearchPaths());

    bool test();

private:
    Q_DISABLE_COPY(TLDExtractor)

    static TLDExtractor* s_instance;
    TLDExtractor(QObject* parent = 0);

    static QStringList defaultDataSearchPaths();

    void loadData();
    bool parseData(const QString &dataFile, bool loadPrivateDomains = false);

    QString domainHelper(const QString &host, const QString &tldPart);
    QString registrableDomainHelper(const QString &domainPart, const QString &tldPart);
    QString subdomainHelper(const QString &host, const QString &registrablePart);

    QString normalizedHost(const QString &host) const;

    bool checkPublicSuffix(const QString &hostName, const QString &registrableName);

    QString m_dataFileName;
    QStringList m_dataSearchPaths;

    QMultiHash<QString, QString> m_tldHash;
};

#endif // TLDEXTRACTOR_H
