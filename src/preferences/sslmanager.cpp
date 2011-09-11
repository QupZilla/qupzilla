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
#include "sslmanager.h"
#include "ui_sslmanager.h"
#include "networkmanager.h"
#include "mainapplication.h"

SSLManager::SSLManager(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SSLManager)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    refresh();

    connect(ui->list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(showCertificateInfo()));
    connect(ui->infoButton, SIGNAL(clicked()), this, SLOT(showCertificateInfo()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteCertificate()));
    connect(ui->ignoreAll, SIGNAL(clicked(bool)), this, SLOT(ignoreAll(bool)));

    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    ui->ignoreAll->setChecked( settings.value("IgnoreAllSSLWarnings", false).toBool() );
    settings.endGroup();
}

void SSLManager::refresh()
{
    ui->list->setUpdatesEnabled(false);
    ui->list->clear();
    m_certs = mApp->networkManager()->getCertExceptions();
    foreach (QSslCertificate cert, m_certs) {
        QListWidgetItem* item = new QListWidgetItem(ui->list);
        item->setText( cert.subjectInfo(QSslCertificate::Organization) + " " + cert.subjectInfo(QSslCertificate::CommonName) );
        item->setWhatsThis(QString::number(m_certs.indexOf(cert)));
        ui->list->addItem(item);
    }
    ui->list->setCurrentRow(0);
    ui->list->setUpdatesEnabled(true);
}

void SSLManager::showCertificateInfo()
{
    QListWidgetItem* item = ui->list->currentItem();
    if (!item)
        return;

    QSslCertificate cert = m_certs.at(item->whatsThis().toInt());
    QStringList actions;
    actions.append(tr("<b>Organization: </b>") + cert.subjectInfo(QSslCertificate::Organization));
    actions.append(tr("<b>Domain Name: </b>") + cert.subjectInfo(QSslCertificate::CommonName));
    actions.append(tr("<b>Locality Name: </b>") + cert.subjectInfo(QSslCertificate::LocalityName));
    actions.append(tr("<b>Country Name: </b>") + cert.subjectInfo(QSslCertificate::CountryName));
    actions.append(tr("<b>Verified by: </b>") + cert.subjectInfo(QSslCertificate::OrganizationalUnitName));
    actions.append(tr("<b>Expiration Date: </b>") + cert.expiryDate().toString("hh:mm:ss dddd d. MMMM yyyy"));

    QString message = QString(QLatin1String("<ul><li>%3</li></ul>")).arg(actions.join(QLatin1String("</li><li>")));

    QMessageBox mes;
    mes.setIcon(QMessageBox::Information);
    mes.setWindowTitle(tr("SSL Certificate Informations"));
    mes.setText(message);
    mes.setDetailedText(cert.toPem());
    mes.exec();
}

void SSLManager::deleteCertificate()
{
    QListWidgetItem* item = ui->list->currentItem();
    if (!item)
        return;

    QSslCertificate cert = m_certs.at(item->whatsThis().toInt());
    m_certs.removeOne(cert);
    mApp->networkManager()->setCertExceptions(m_certs);
    refresh();
}

void SSLManager::ignoreAll(bool state)
{
    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("IgnoreAllSSLWarnings", state);
    settings.endGroup();
    mApp->networkManager()->loadSettings();
}

SSLManager::~SSLManager()
{
    delete ui;
}
