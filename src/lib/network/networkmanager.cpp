/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "webpage.h"
#include "pluginproxy.h"
#include "adblockmanager.h"
#include "adblocknetwork.h"
#include "networkproxyfactory.h"
#include "qupzillaschemehandler.h"
#include "certificateinfowidget.h"
#include "globalfunctions.h"
#include "acceptlanguage.h"
#include "cabundleupdater.h"
#include "settings.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QTextDocument>
#include <QNetworkDiskCache>
#include <QDir>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QDateTime>
#include <QMessageBox>
#include <QAuthenticator>
#include <QDirIterator>
#include <QDebug>

QString fileNameForCert(const QSslCertificate &cert)
{
    QString certFileName = CertificateInfoWidget::certificateItemText(cert);
    certFileName.remove(" ");
    certFileName.append(".crt");
    certFileName = qz_filterCharsFromFilename(certFileName);
    return certFileName;
}

NetworkManager::NetworkManager(QupZilla* mainClass, QObject* parent)
    : NetworkManagerProxy(parent)
    , m_adblockNetwork(0)
    , p_QupZilla(mainClass)
    , m_ignoreAllWarnings(false)
{
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(authentication(QNetworkReply*, QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(QNetworkProxy, QAuthenticator*)), this, SLOT(proxyAuthentication(QNetworkProxy, QAuthenticator*)));
    connect(this, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)), this, SLOT(sslError(QNetworkReply*, QList<QSslError>)));
    connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(setSSLConfiguration(QNetworkReply*)));

    m_schemeHandlers["qupzilla"] = new QupZillaSchemeHandler;

    m_proxyFactory = new NetworkProxyFactory();
    setProxyFactory(m_proxyFactory);
    loadSettings();
}

void NetworkManager::loadSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");

    if (settings.value("AllowLocalCache", true).toBool()) {
        m_diskCache = mApp->networkCache();
        m_diskCache->setCacheDirectory(mApp->getActiveProfilPath() + "/networkcache");
        m_diskCache->setMaximumCacheSize(settings.value("MaximumCacheSize", 50).toInt() * 1024 * 1024); //MegaBytes
        setCache(m_diskCache);
    }
    m_doNotTrack = settings.value("DoNotTrack", false).toBool();
    m_sendReferer = settings.value("SendReferer", true).toBool();
    settings.endGroup();
    m_acceptLanguage = AcceptLanguage::generateHeader(settings.value("Language/acceptLanguage", AcceptLanguage::defaultLanguage()).toStringList());

#ifdef Q_WS_WIN
    // From doc:
    // QSslSocket::VerifyNone ... The connection will still be encrypted, and your socket
    // will still send its local certificate to the peer if it's requested.

    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(config);
#endif

    QString certDir = mApp->PROFILEDIR + "certificates";
    QString bundlePath = certDir + "/ca-bundle.crt";
    QString bundleVersionPath = certDir + "/bundle_version";

    if (!QDir(certDir).exists()) {
        QDir dir(mApp->PROFILEDIR);
        dir.mkdir("certificates");
    }

    if (!QFile::exists(bundlePath)) {
        QFile(":data/ca-bundle.crt").copy(bundlePath);
        QFile(bundlePath).setPermissions(QFile::ReadUser | QFile::WriteUser);

        QFile(":data/bundle_version").copy(bundleVersionPath);
        QFile(bundleVersionPath).setPermissions(QFile::ReadUser | QFile::WriteUser);
    }

    QSslSocket::setDefaultCaCertificates(QSslCertificate::fromPath(bundlePath));

    m_proxyFactory->loadSettings();
}

void NetworkManager::setSSLConfiguration(QNetworkReply* reply)
{
    if (!reply->sslConfiguration().isNull()) {
        QSslCertificate cert = reply->sslConfiguration().peerCertificate();
        if (!cert.isValid() || reply->property("downReply").toBool()) {
            return;
        }

        QNetworkRequest request = reply->request();
        QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
        WebPage* webPage = static_cast<WebPage*>(v.value<void*>());
        if (!webPage) {
            return;
        }

        if (webPage->url().host() == reply->url().host()) {
            webPage->setSSLCertificate(cert);
        }
    }
}

void NetworkManager::sslError(QNetworkReply* reply, QList<QSslError> errors)
{
    if (m_ignoreAllWarnings) {
        reply->ignoreSslErrors(errors);
        return;
    }

    if (reply->property("downReply").toBool()) {
        return;
    }

    int errorsIgnored = 0;
    foreach(const QSslError & error, errors) {
        if (m_ignoredCerts.contains(error.certificate())) {
            ++errorsIgnored;
        }
    }

    if (errorsIgnored == errors.count()) {
        return;
    }

    QNetworkRequest request = reply->request();
    QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
    WebPage* webPage = static_cast<WebPage*>(v.value<void*>());
    if (!webPage) {
        return;
    }

    QString title = tr("SSL Certificate Error!");
    QString text1 = tr("The page you are trying to access has the following errors in the SSL certificate:");

    QString certs;

    foreach(const QSslError & error, errors) {
        if (m_localCerts.contains(error.certificate())) {
            continue;
        }
        if (error.error() == QSslError::NoError) { //Weird behavior on Windows
            continue;
        }

        QSslCertificate cert = error.certificate();
        certs.append("<ul><li>");
        certs.append(tr("<b>Organization: </b>") + CertificateInfoWidget::clearCertSpecialSymbols(cert.subjectInfo(QSslCertificate::Organization)));
        certs.append("</li><li>");
        certs.append(tr("<b>Domain Name: </b>") + CertificateInfoWidget::clearCertSpecialSymbols(cert.subjectInfo(QSslCertificate::CommonName)));
        certs.append("</li><li>");
        certs.append(tr("<b>Expiration Date: </b>") + cert.expiryDate().toString("hh:mm:ss dddd d. MMMM yyyy"));
        certs.append("</li><li>");
        certs.append(tr("<b>Error: </b>") + error.errorString());
        certs.append("</li></ul>");
    }

    QString text2 = tr("Would you like to make an exception for this certificate?");
    QString message = QString("<b>%1</b><p>%2</p>%3<p>%4</p>").arg(title, text1, certs, text2);

    if (!certs.isEmpty())  {
        if (QMessageBox::critical(webPage->view(), tr("SSL Certificate Error!"), message,
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
            return;
        }

        foreach(const QSslError & error, errors) {
            if (!m_localCerts.contains(error.certificate())) {
                addLocalCertificate(error.certificate());
            }
        }
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
                      "The site says: \"%2\"").arg(reply->url().toEncoded(), Qt::escape(auth->realm())));
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
    if (mApp->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled)) {
        save->setVisible(false);
    }

    if (dialog->exec() != QDialog::Accepted) {
        return;
    }

    auth->setUser(user->text());
    auth->setPassword(pass->text());

    if (save->isChecked()) {
        fill->addEntry(reply->url(), user->text(), pass->text());
    }
}

void NetworkManager::proxyAuthentication(const QNetworkProxy &proxy, QAuthenticator* auth)
{
    QDialog* dialog = new QDialog(p_QupZilla);
    dialog->setWindowTitle(tr("Proxy authorization required"));

    QFormLayout* formLa = new QFormLayout(dialog);

    QLabel* label = new QLabel(dialog);
    QLabel* userLab = new QLabel(dialog);
    QLabel* passLab = new QLabel(dialog);
    userLab->setText(tr("Username: "));
    passLab->setText(tr("Password: "));

    QLineEdit* user = new QLineEdit(dialog);
    QLineEdit* pass = new QLineEdit(dialog);
    pass->setEchoMode(QLineEdit::Password);

    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));

    label->setText(tr("A username and password are being requested by proxy %1. ").arg(proxy.hostName()));
    formLa->addRow(label);
    formLa->addRow(userLab, user);
    formLa->addRow(passLab, pass);
    formLa->addWidget(box);

    if (dialog->exec() != QDialog::Accepted) {
        return;
    }

    auth->setUser(user->text());
    auth->setPassword(pass->text());
}

QNetworkReply* NetworkManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
{
    if (op == PostOperation && outgoingData) {
        QByteArray outgoingDataByteArray = outgoingData->peek(1024 * 1024);
        mApp->autoFill()->post(request, outgoingDataByteArray);
    }

    QNetworkRequest req = request;
    QNetworkReply* reply = 0;

    //SchemeHandlers
    if (m_schemeHandlers.contains(req.url().scheme())) {
        reply = m_schemeHandlers[req.url().scheme()]->createRequest(op, req, outgoingData);
        if (reply) {
            return reply;
        }
    }

    if (m_doNotTrack) {
        req.setRawHeader("DNT", QByteArray("1"));
    }

    if (!m_sendReferer) {
        req.setRawHeader("Referer", QByteArray());
    }

    req.setRawHeader("Accept-Language", m_acceptLanguage);

    req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    if (req.attribute(QNetworkRequest::CacheLoadControlAttribute).toInt() == QNetworkRequest::PreferNetwork) {
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    }

    // Adblock
    if (op == QNetworkAccessManager::GetOperation) {
        if (!m_adblockNetwork) {
            m_adblockNetwork = AdBlockManager::instance()->network();
        }
        reply = m_adblockNetwork->block(req);
        if (reply) {
            return reply;
        }
    }

    reply = QNetworkAccessManager::createRequest(op, req, outgoingData);
    return reply;
}

void NetworkManager::removeLocalCertificate(const QSslCertificate &cert)
{
    m_localCerts.removeOne(cert);
    QList<QSslCertificate> certs = QSslSocket::defaultCaCertificates();
    certs.removeOne(cert);
    QSslSocket::setDefaultCaCertificates(certs);

    //Delete cert file from profile
    bool deleted = false;
    QDirIterator it(mApp->getActiveProfilPath() + "certificates", QDir::Files, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString &filePath = it.next();
        const QList<QSslCertificate> &certs = QSslCertificate::fromPath(filePath);
        if (certs.isEmpty()) {
            continue;
        }

        const QSslCertificate &cert_ = certs.at(0);
        if (cert == cert_) {
            QFile file(filePath);
            if (!file.remove()) {
                qWarning() << "NetworkManager::removeLocalCertificate cannot remove file" << filePath;
            }

            deleted = true;
            break;
        }
    }

    if (!deleted) {
        qWarning() << "NetworkManager::removeLocalCertificate cannot remove certificate" << cert;
    }
}

void NetworkManager::addLocalCertificate(const QSslCertificate &cert)
{
//    if (!cert.isValid()) {
//        return;
//    }

    m_localCerts.append(cert);
    QSslSocket::addDefaultCaCertificate(cert);

    QDir dir(mApp->getActiveProfilPath());
    if (!dir.exists("certificates")) {
        dir.mkdir("certificates");
    }

    QString certFileName = fileNameForCert(cert);
    QString fileName = qz_ensureUniqueFilename(mApp->getActiveProfilPath() + "certificates/" + certFileName);

    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        file.write(cert.toPem());
        file.close();
    }
    else {
        qWarning() << "NetworkManager::addLocalCertificate cannot write to file: " << fileName;
    }
}

bool NetworkManager::registerSchemeHandler(const QString &scheme, SchemeHandler* handler)
{
    if (m_schemeHandlers.contains(scheme)) {
        return false;
    }

    m_schemeHandlers[scheme] = handler;
    return true;
}

void NetworkManager::saveCertificates()
{
    Settings settings;
    settings.beginGroup("SSL-Configuration");
    settings.setValue("CACertPaths", m_certPaths);
    settings.setValue("IgnoreAllSSLWarnings", m_ignoreAllWarnings);
    settings.endGroup();
}

void NetworkManager::loadCertificates()
{
    Settings settings;
    settings.beginGroup("SSL-Configuration");
    m_certPaths = settings.value("CACertPaths", QStringList()).toStringList();
    m_ignoreAllWarnings = settings.value("IgnoreAllSSLWarnings", false).toBool();
    settings.endGroup();

    //CA Certificates
    m_caCerts = QSslSocket::defaultCaCertificates();
    foreach(const QString & path, m_certPaths) {
#ifdef Q_WS_WIN
        // Used from Qt 4.7.4 qsslcertificate.cpp and modified because QSslCertificate::fromPath
        // is kind of a bugged on Windows, it does work only with full path to cert file
        QDirIterator it(path, QDir::Files, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            if (!filePath.endsWith(".crt")) {
                continue;
            }

            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                m_caCerts += QSslCertificate::fromData(file.readAll(), QSsl::Pem);
            }
        }
#else
        m_caCerts += QSslCertificate::fromPath(path + "/*.crt", QSsl::Pem, QRegExp::Wildcard);
#endif
    }
    //Local Certificates
#ifdef Q_WS_WIN
    QDirIterator it_(mApp->getActiveProfilPath() + "certificates", QDir::Files, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
    while (it_.hasNext()) {
        QString filePath = it_.next();
        if (!filePath.endsWith(".crt")) {
            continue;
        }

        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_localCerts += QSslCertificate::fromData(file.readAll(), QSsl::Pem);
        }
    }
#else
    m_localCerts += QSslCertificate::fromPath(mApp->getActiveProfilPath() + "certificates/*.crt", QSsl::Pem, QRegExp::Wildcard);
#endif

    QSslSocket::setDefaultCaCertificates(m_caCerts + m_localCerts);

    new CaBundleUpdater(this, this);
}

void NetworkManager::disconnectObjects()
{
    disconnect(this);
}
