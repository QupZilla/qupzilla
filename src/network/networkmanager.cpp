/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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

NetworkManager::NetworkManager(QupZilla* mainClass, QObject* parent)
    : NetworkManagerProxy(mainClass, parent)
    , m_adblockNetwork(0)
    , p_QupZilla(mainClass)
    , m_qupzillaSchemeHandler(new QupZillaSchemeHandler)
    , m_ignoreAllWarnings(false)
{
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(authentication(QNetworkReply*, QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(QNetworkProxy, QAuthenticator*)), this, SLOT(proxyAuthentication(QNetworkProxy, QAuthenticator*)));
    connect(this, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)), this, SLOT(sslError(QNetworkReply*, QList<QSslError>)));
    connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(setSSLConfiguration(QNetworkReply*)));

    m_proxyFactory = new NetworkProxyFactory();
    setProxyFactory(m_proxyFactory);
    loadSettings();
}

void NetworkManager::loadSettings()
{
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");

    if (settings.value("AllowLocalCache", true).toBool()) {
        m_diskCache = mApp->networkCache();
        m_diskCache->setCacheDirectory(mApp->getActiveProfilPath() + "/networkcache");
        m_diskCache->setMaximumCacheSize(settings.value("MaximumCacheSize", 50).toInt() * 1024 * 1024); //MegaBytes
        setCache(m_diskCache);
    }
    m_doNotTrack = settings.value("DoNotTrack", false).toBool();
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
        WebPage* webPage = (WebPage*)(v.value<void*>());
        v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 102));
        WebView* webView = (WebView*)(v.value<void*>());
        if (!webPage || !webView) {
            return;
        }

        if (webView->url().host() == reply->url().host()) {
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
    foreach(QSslError error, errors) {
        if (m_ignoredCerts.contains(error.certificate())) {
            ++errorsIgnored;
        }
    }

    if (errorsIgnored == errors.count()) {
        return;
    }

    QNetworkRequest request = reply->request();
    QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
    WebPage* webPage = (WebPage*)(v.value<void*>());
    if (!webPage) {
        return;
    }

    QString title = tr("SSL Certificate Error!");
    QString text1 = tr("The page you are trying to access has the following errors in the SSL certificate:");

    QString certs;

    foreach(QSslError error, errors) {
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
        if (!webPage->javaScriptConfirm(webPage->mainFrame(), message)) {
            return;
        }

        foreach(QSslError error, errors) {
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

    if (!dialog->exec() == QDialog::Accepted) {
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

    if (!dialog->exec() == QDialog::Accepted) {
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

    if (m_doNotTrack) {
        req.setRawHeader("DNT", "1");
    }

    req.setRawHeader("Accept-Language", m_acceptLanguage);

    //SchemeHandlers
    if (req.url().scheme() == "qupzilla") {
        reply = m_qupzillaSchemeHandler->createRequest(op, req, outgoingData);
    }
    if (reply) {
        return reply;
    }

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
    QString certFileName = CertificateInfoWidget::certificateItemText(cert);
    int startIndex = 0;
    QDirIterator it(mApp->getActiveProfilPath() + "certificates", QDir::Files, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = startIndex == 0 ? it.next() : it.next().mid(startIndex);
        if (!filePath.contains(certFileName)) {
            continue;
        }

        QFile file(filePath);
        file.remove();
        break;
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

    QString fileName = qz_ensureUniqueFilename(mApp->getActiveProfilPath() + "certificates/" + CertificateInfoWidget::certificateItemText(cert).remove(" ") + ".crt");

    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        file.write(cert.toPem());
        file.close();
    }
    else {
        qWarning() << "NetworkManager::addLocalCertificate cannot write to file: " << fileName;
    }
}

void NetworkManager::saveCertificates()
{
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("SSL-Configuration");
    settings.setValue("CACertPaths", m_certPaths);
    settings.setValue("IgnoreAllSSLWarnings", m_ignoreAllWarnings);
    settings.endGroup();
}

void NetworkManager::loadCertificates()
{
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("SSL-Configuration");
    m_certPaths = settings.value("CACertPaths", QStringList()).toStringList();
    m_ignoreAllWarnings = settings.value("IgnoreAllSSLWarnings", false).toBool();
    settings.endGroup();

    //CA Certificates
    m_caCerts = QSslSocket::defaultCaCertificates();
    foreach(QString path, m_certPaths) {
#ifdef Q_WS_WIN
        // Used from Qt 4.7.4 qsslcertificate.cpp and modified because QSslCertificate::fromPath
        // is kind of a bugged on Windows, it does work only with full path to cert file
        int startIndex = 0;
        QDirIterator it(path, QDir::Files, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = startIndex == 0 ? it.next() : it.next().mid(startIndex);
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
    int startIndex = 0;
    QDirIterator it_(mApp->getActiveProfilPath() + "certificates", QDir::Files, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
    while (it_.hasNext()) {
        QString filePath = startIndex == 0 ? it_.next() : it_.next().mid(startIndex);
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
}
