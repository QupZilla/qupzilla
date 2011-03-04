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

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

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

#include "networkmanagerproxy.h"

class QupZilla;
class NetworkManager : public NetworkManagerProxy
{
    Q_OBJECT
public:
    explicit NetworkManager(QupZilla* mainClass, QObject *parent = 0);
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData);

    QList<QSslCertificate> getCertExceptions() { return m_certExceptions; }
    void setCertExceptions(QList<QSslCertificate> certs) { m_certExceptions = certs; }
    void saveCertExceptions();
    void loadCertExceptions();
    void loadSettings();

signals:
    void finishLoading(bool state);
    void wantsFocus(const QUrl &url);
    void sslDialogClosed();

public slots:
    void authentication(QNetworkReply* reply, QAuthenticator* auth);
    void sslError(QNetworkReply* reply, QList<QSslError> errors);

private:
    QupZilla* p_QupZilla;
    QList<QSslCertificate> m_certExceptions;
    QNetworkDiskCache* m_diskCache;

};

#endif // NETWORKMANAGER_H
