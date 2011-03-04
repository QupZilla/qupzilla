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
#include "networkmanager.h"
#include "qupzilla.h"
#include "autofillmodel.h"
#include "networkmanagerproxy.h"
#include "mainapplication.h"

NetworkManager::NetworkManager(QupZilla* mainClass, QObject *parent) :
    NetworkManagerProxy(mainClass, parent)
    ,p_QupZilla(mainClass)
{
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(authentication(QNetworkReply*, QAuthenticator* )));
    connect(this, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslError(QNetworkReply*,QList<QSslError>)));

    loadSettings();
}

void NetworkManager::loadSettings()
{
    QSettings settings(mApp->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");

    if (settings.value("AllowLocalCache", true).toBool()) {
        m_diskCache = new QNetworkDiskCache(this);
        m_diskCache->setCacheDirectory(mApp->getActiveProfil()+"/networkcache");
        m_diskCache->setMaximumCacheSize(settings.value("MaximumCacheSize",50).toInt() * 1024*1024); //MegaBytes
        setCache(m_diskCache);
    }
    settings.endGroup();
}

void NetworkManager::sslError(QNetworkReply *reply, QList<QSslError> errors)
{
    QString title = tr("SSL Certificate Error!");
    QString text1 = tr("The page you trying to access has following errors in SSL Certificate:");

    QStringList actions;

    foreach (QSslError error, errors) {
        if (m_certExceptions.contains(error.certificate()))
            continue;
        if (error.error() == QSslError::NoError) //Weird behavior on Windows
            continue;

        QSslCertificate cert = error.certificate();
        actions.append(tr("<b>Organization: </b>") + cert.subjectInfo(QSslCertificate::Organization));
        actions.append(tr("<b>Domain Name: </b>") + cert.subjectInfo(QSslCertificate::CommonName));
        actions.append(tr("<b>Expiration Date: </b>") + cert.expiryDate().toString("hh:mm:ss dddd d. MMMM yyyy"));
        actions.append(tr("<b>Error: </b>") + error.errorString());
    }

    QString text2 = tr("Would you like to make exception for this certificate?");
    QString message = QString(QLatin1String("<b>%1</b><p>%2</p><ul><li>%3</li></ul><p>%4</p>")).arg(title, text1, actions.join(QLatin1String("</li><li>")), text2);

    if (!actions.isEmpty())  {
        QMessageBox::StandardButton button = QMessageBox::critical(p_QupZilla, tr("SSL Certificate Error"),
                                                               message, QMessageBox::Yes | QMessageBox::No);
        if (button != QMessageBox::Yes)
            return;
    }

    foreach (QSslError error, errors) {
        if (m_certExceptions.contains(error.certificate()))
            continue;
        m_certExceptions.append(error.certificate());
    }

    reply->ignoreSslErrors(errors);
}

void NetworkManager::authentication(QNetworkReply* reply, QAuthenticator* auth)
{
    QDialog* dialog = new QDialog(p_QupZilla);
    dialog->setWindowTitle(tr("Authorization required"));

    QFormLayout* formLa = new QFormLayout(dialog);

    QLabel* label = new QLabel(dialog);
    QLabel* userLab = new QLabel(dialog);
    QLabel* passLab = new QLabel(dialog);
    userLab->setText(tr("Username: "));
    passLab->setText(tr("Password: "));

    QLineEdit* user = new QLineEdit(dialog);
    QLineEdit* pass = new QLineEdit(dialog);
    QCheckBox* save = new QCheckBox(dialog);
    save->setText(tr("Save username and password on this site"));
    pass->setEchoMode(QLineEdit::Password);

    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));

    label->setText(tr("A username and password are being requested by %1. "
                      "The site says: \"%2\"").arg(reply->url().toEncoded(), auth->realm()));
    formLa->addRow(label);

    formLa->addRow(userLab, user);
    formLa->addRow(passLab, pass);
    formLa->addRow(save);

    formLa->addWidget(box);
    AutoFillModel* fill = mApp->autoFill();
    if (fill->isStored(reply->url())) {
        save->setChecked(true);
        user->setText(fill->getUsername(reply->url()));
        pass->setText(fill->getPassword(reply->url()));
    }
    emit wantsFocus(reply->url());

    //Do not save when private browsing is enabled
    if (mApp->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        save->setVisible(false);

    if (!dialog->exec() == QDialog::Accepted)
        return;
    auth->setUser(user->text());
    auth->setPassword(pass->text());

    if (save->isChecked())
        fill->addEntry(reply->url(), user->text(), pass->text());
}

QNetworkReply *NetworkManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    if (op == PostOperation && outgoingData) {
            QByteArray outgoingDataByteArray = outgoingData->peek(1024 * 1024);
            mApp->autoFill()->post(request, outgoingDataByteArray);
    }

    QNetworkRequest req = request;
    req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    QNetworkReply* reply = QNetworkAccessManager::createRequest(op, req, outgoingData);
    //emit requestCreated(op, request, reply);
    return reply;
}

void NetworkManager::saveCertExceptions()
{
    QFile file(mApp->getActiveProfil()+"sslexceptions.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);

    int count = m_certExceptions.count();
    stream << count;

    for (int i = 0; i < count; i++) {
         stream << m_certExceptions.at(i).toPem();
    }

    file.close();
}

void NetworkManager::loadCertExceptions()
{
    QFile file(mApp->getActiveProfil()+"sslexceptions.dat");
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);

    int count;
    stream >> count;
    QByteArray cert;

    for (int i = 0; i < count; i++) {
         stream >> cert;
         m_certExceptions.append(QSslCertificate::fromData(cert));
    }

    file.close();
}
