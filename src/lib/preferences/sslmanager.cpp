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
#include "sslmanager.h"
#include "ui_sslmanager.h"
#include "networkmanager.h"
#include "mainapplication.h"
#include "qztools.h"
#include "certificateinfowidget.h"

#include <QFileDialog>
#include <QSslSocket>

SSLManager::SSLManager(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SSLManager)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->caList->setLayoutDirection(Qt::LeftToRight);
    ui->localList->setLayoutDirection(Qt::LeftToRight);
    ui->pathList->setLayoutDirection(Qt::LeftToRight);

    refreshLocalList();
    refreshCAList();
    refreshPaths();

    connect(ui->caList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(showCaCertInfo()));
    connect(ui->caInfoButton, SIGNAL(clicked()), this, SLOT(showCaCertInfo()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteCertificate()));

    connect(ui->localList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(showLocalCertInfo()));
    connect(ui->localInfoButton, SIGNAL(clicked()), this, SLOT(showLocalCertInfo()));
    connect(ui->addLocalCert, SIGNAL(clicked()), this, SLOT(addLocalCertificate()));

    connect(ui->addPath, SIGNAL(clicked()), this, SLOT(addPath()));
    connect(ui->deletePath, SIGNAL(clicked()), this, SLOT(deletePath()));
    connect(ui->ignoreAll, SIGNAL(clicked(bool)), this, SLOT(ignoreAll(bool)));
    connect(ui->disableWeakCiphers, SIGNAL(clicked(bool)), this, SLOT(disableWeakCiphers(bool)));

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));

    // Settings
    ui->disableWeakCiphers->setChecked(mApp->networkManager()->isDisablingWeakCiphers());
    ui->ignoreAll->setChecked(mApp->networkManager()->isIgnoringAllWarnings());
}

void SSLManager::addPath()
{
    QString path = QzTools::getExistingDirectory("SSLManager-AddPath", this, tr("Choose path..."));
    if (path.isEmpty()) {
        return;
    }

    ui->pathList->addItem(path);
}

void SSLManager::deletePath()
{
    delete ui->pathList->currentItem();
}

void SSLManager::refreshCAList()
{
    ui->caList->setUpdatesEnabled(false);
    ui->caList->clear();
    m_caCerts = QSslSocket::defaultCaCertificates();

    foreach (const QSslCertificate &cert, m_caCerts) {
        if (m_localCerts.contains(cert)) {
            continue;
        }

        QListWidgetItem* item = new QListWidgetItem(ui->caList);
        item->setText(CertificateInfoWidget::certificateItemText(cert));
        item->setData(Qt::UserRole + 10, m_caCerts.indexOf(cert));
        ui->caList->addItem(item);
    }

    ui->caList->setCurrentRow(0);
    ui->caList->setUpdatesEnabled(true);
}

void SSLManager::refreshLocalList()
{
    ui->localList->setUpdatesEnabled(false);
    ui->localList->clear();
    m_localCerts = mApp->networkManager()->getLocalCertificates();

    foreach (const QSslCertificate &cert, m_localCerts) {
        QListWidgetItem* item = new QListWidgetItem(ui->localList);
        item->setText(CertificateInfoWidget::certificateItemText(cert));
        item->setData(Qt::UserRole + 10, m_localCerts.indexOf(cert));
        ui->localList->addItem(item);
    }

    ui->localList->setCurrentRow(0);
    ui->localList->setUpdatesEnabled(true);
}

void SSLManager::refreshPaths()
{
    foreach (const QString &path, mApp->networkManager()->certificatePaths()) {
        ui->pathList->addItem(path);
    }
}

void SSLManager::showCaCertInfo()
{
    QListWidgetItem* item = ui->caList->currentItem();
    if (!item) {
        return;
    }

    QSslCertificate cert = m_caCerts.at(item->data(Qt::UserRole + 10).toInt());
    showCertificateInfo(cert);
}

void SSLManager::addLocalCertificate()
{
    const QString path = QzTools::getOpenFileName("SSLManager-AddLocalCert", this, tr("Import certificate..."), QDir::homePath(), "*.crt");

    if (path.isEmpty()) {
        return;
    }

    const QList<QSslCertificate> list = QSslCertificate::fromPath(path);
    if (list.isEmpty()) {
        return;
    }

    mApp->networkManager()->addLocalCertificate(list.at(0));

    refreshLocalList();
}

void SSLManager::showLocalCertInfo()
{
    QListWidgetItem* item = ui->localList->currentItem();
    if (!item) {
        return;
    }

    QSslCertificate cert = m_localCerts.at(item->data(Qt::UserRole + 10).toInt());
    showCertificateInfo(cert);
}

void SSLManager::showCertificateInfo(const QSslCertificate &cert)
{
    QDialog* w = new QDialog(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setWindowTitle(tr("Certificate Informations"));
    w->setLayout(new QVBoxLayout);
    CertificateInfoWidget* c = new CertificateInfoWidget(cert);
    w->layout()->addWidget(c);
    QDialogButtonBox* b = new QDialogButtonBox(w);
    b->setStandardButtons(QDialogButtonBox::Close);
    connect(b, SIGNAL(clicked(QAbstractButton*)), w, SLOT(close()));
    w->layout()->addWidget(b);
    w->resize(w->sizeHint());
    QzTools::centerWidgetToParent(w, this);
    w->show();
}

void SSLManager::deleteCertificate()
{
    QListWidgetItem* item = ui->localList->currentItem();
    if (!item) {
        return;
    }

    QSslCertificate cert = m_localCerts.at(item->data(Qt::UserRole + 10).toInt());
    m_localCerts.removeOne(cert);
    mApp->networkManager()->removeLocalCertificate(cert);
    refreshLocalList();
}

void SSLManager::ignoreAll(bool state)
{
    mApp->networkManager()->setIgnoreAllWarnings(state);
}

void SSLManager::disableWeakCiphers(bool state)
{
    mApp->networkManager()->setDisableWeakCiphers(state);
}

void SSLManager::closeEvent(QCloseEvent* e)
{
    QStringList paths;
    for (int i = 0; i < ui->pathList->count(); i++) {
        QListWidgetItem* item = ui->pathList->item(i);
        if (!item || item->text().isEmpty()) {
            continue;
        }

        paths.append(item->text());
    }

    mApp->networkManager()->setCertificatePaths(paths);
    mApp->networkManager()->saveSettings();

    QWidget::closeEvent(e);
}

SSLManager::~SSLManager()
{
    delete ui;
}
