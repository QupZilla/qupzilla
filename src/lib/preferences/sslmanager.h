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
#ifndef SSLMANAGER_H
#define SSLMANAGER_H

#include <QDialog>
#include <QSslCertificate>

#include "qzcommon.h"

namespace Ui
{
class SSLManager;
}

class QUPZILLA_EXPORT SSLManager : public QDialog
{
    Q_OBJECT

public:
    explicit SSLManager(QWidget* parent = 0);
    ~SSLManager();

private slots:
    void showLocalCertInfo();
    void showCaCertInfo();
    void addLocalCertificate();

    void deleteCertificate();
    void ignoreAll(bool state);
    void disableWeakCiphers(bool state);

    void addPath();
    void deletePath();

private:
    void closeEvent(QCloseEvent* e);

    void refreshLocalList();
    void refreshCAList();
    void refreshPaths();

    void showCertificateInfo(const QSslCertificate &cert);

    Ui::SSLManager* ui;
    QList<QSslCertificate> m_localCerts;
    QList<QSslCertificate> m_caCerts;
};

#endif // SSLMANAGER_H
