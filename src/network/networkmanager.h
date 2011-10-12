/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QNetworkAccessManager>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QUrl>
#include <QFormLayout>
#include <QCheckBox>
#include <QSslError>
#include <QNetworkDiskCache>
#include <QSslSocket>
#include <QSslConfiguration>

#include "networkmanagerproxy.h"

class QupZilla;
class AdBlockNetwork;
class NetworkProxyFactory;
class QupZillaSchemeHandler;
class NetworkManager : public NetworkManagerProxy
{
    Q_OBJECT
public:
    explicit NetworkManager(QupZilla* mainClass, QObject* parent = 0);
    QNetworkReply* createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData);

    void saveCertificates();
    void loadCertificates();

    QList<QSslCertificate> getCaCertificates() { return m_caCerts; }
    QList<QSslCertificate> getLocalCertificates() { return m_localCerts; }

    void removeLocalCertificate(const QSslCertificate &cert);
    void addLocalCertificate(const QSslCertificate &cert);

    void setCertificatePaths(const QStringList &paths) { m_certPaths = paths; }
    QStringList certificatePaths() { return m_certPaths; }

    void setIgnoreAllWarnings(bool state) { m_ignoreAllWarnings = state; }
    bool isIgnoringAllWarnings() { return m_ignoreAllWarnings; }

    void loadSettings();

signals:
    void finishLoading(bool state);
    void wantsFocus(const QUrl &url);
    void sslDialogClosed();

private slots:
    void authentication(QNetworkReply* reply, QAuthenticator* auth);
    void proxyAuthentication(const QNetworkProxy& proxy,QAuthenticator* auth);
    void sslError(QNetworkReply* reply, QList<QSslError> errors);
    void setSSLConfiguration(QNetworkReply* reply);

private:
    AdBlockNetwork* m_adblockNetwork;
    QupZilla* p_QupZilla;
    QNetworkDiskCache* m_diskCache;
    NetworkProxyFactory* m_proxyFactory;
    QupZillaSchemeHandler* m_qupzillaSchemeHandler;

    QStringList m_certPaths;
    QList<QSslCertificate> m_caCerts;
    QList<QSslCertificate> m_localCerts;

    bool m_ignoreAllWarnings;
    bool m_doNotTrack;
};

#endif // NETWORKMANAGER_H
