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
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QNetworkAccessManager>
#include <QWebEngineCertificateError>

#include "qzcommon.h"

class QUPZILLA_EXPORT NetworkManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = Q_NULLPTR);

    bool certificateError(const QWebEngineCertificateError &error, QWidget *parent = Q_NULLPTR);
    void authentication(const QUrl &url, QAuthenticator *auth, QWidget *parent = Q_NULLPTR);
    void proxyAuthentication(const QString &proxyHost, QAuthenticator *auth, QWidget *parent = Q_NULLPTR);

private:
    QHash<QString, QWebEngineCertificateError::Error> m_ignoredSslErrors;
};

#if QTWEBENGINE_DISABLED

#include <QSslError>
#include <QStringList>

#include "qzcommon.h"
#include "networkmanagerproxy.h"

class AdBlockManager;
class NetworkProxyFactory;
class QupZillaSchemeHandler;
class SchemeHandler;

class QUPZILLA_EXPORT NetworkManager : public NetworkManagerProxy
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject* parent = 0);
    QNetworkReply* createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData);

    void loadSettings();
    void saveSettings();
    void loadCertificates();

    QList<QSslCertificate> getCaCertificates() { return m_caCerts; }
    QList<QSslCertificate> getLocalCertificates() { return m_localCerts; }

    void removeLocalCertificate(const QSslCertificate &cert);
    void addLocalCertificate(const QSslCertificate &cert);

    void setCertificatePaths(const QStringList &paths) { m_certPaths = paths; }
    QStringList certificatePaths() { return m_certPaths; }

    bool isIgnoringAllWarnings() const;
    void setIgnoreAllWarnings(bool state);

    bool isDisablingWeakCiphers() const;
    void setDisableWeakCiphers(bool state);

    NetworkProxyFactory* proxyFactory() const;

    bool registerSchemeHandler(const QString &scheme, SchemeHandler* handler);
    bool unregisterSchemeHandler(const QString &scheme, SchemeHandler* handler);

signals:
    void sslDialogClosed();

private slots:
    void ftpAuthentication(const QUrl &url, QAuthenticator* auth);
    void sslError(QNetworkReply* reply, QList<QSslError> errors);
    void setSSLConfiguration(QNetworkReply* reply);

private:
    void disableWeakCiphers(bool disable);

    AdBlockManager* m_adblockManager;
    NetworkProxyFactory* m_proxyFactory;

    QStringList m_certPaths;
    QList<QSslCertificate> m_caCerts;
    QList<QSslCertificate> m_localCerts;
    QList<QSslCertificate> m_tempAllowedCerts;

    QHash<QString, SchemeHandler*> m_schemeHandlers;
    QByteArray m_acceptLanguage;

    bool m_ignoreAllWarnings;
    bool m_disableWeakCiphers;
    bool m_doNotTrack;
    bool m_sendReferer;
};

#endif

#endif // NETWORKMANAGER_H
