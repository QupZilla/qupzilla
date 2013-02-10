/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "autofill.h"
#include "networkmanagerproxy.h"
#include "mainapplication.h"
#include "webpage.h"
#include "tabbedwebview.h"
#include "pluginproxy.h"
#include "adblockmanager.h"
#include "networkproxyfactory.h"
#include "certificateinfowidget.h"
#include "qztools.h"
#include "acceptlanguage.h"
#include "cabundleupdater.h"
#include "settings.h"
#include "schemehandlers/adblockschemehandler.h"
#include "schemehandlers/qupzillaschemehandler.h"
#include "schemehandlers/fileschemehandler.h"
#include "schemehandlers/ftpschemehandler.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QNetworkDiskCache>
#include <QDir>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QDateTime>
#include <QMessageBox>
#include <QAuthenticator>
#include <QDirIterator>
#include <QDebug>

static QString fileNameForCert(const QSslCertificate &cert)
{
    QString certFileName = CertificateInfoWidget::certificateItemText(cert);
    certFileName.remove(QLatin1Char(' '));
    certFileName.append(QLatin1String(".crt"));
    certFileName = QzTools::filterCharsFromFilename(certFileName);

    while (certFileName.startsWith(QLatin1Char('.'))) {
        certFileName = certFileName.mid(1);
    }

    return certFileName;
}

NetworkManager::NetworkManager(QupZilla* mainClass, QObject* parent)
    : NetworkManagerProxy(parent)
    , m_adblockManager(0)
    , p_QupZilla(mainClass)
    , m_ignoreAllWarnings(false)
{
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(authentication(QNetworkReply*, QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(QNetworkProxy, QAuthenticator*)), this, SLOT(proxyAuthentication(QNetworkProxy, QAuthenticator*)));
    connect(this, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)), this, SLOT(sslError(QNetworkReply*, QList<QSslError>)));
    connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(setSSLConfiguration(QNetworkReply*)));

    m_schemeHandlers["qupzilla"] = new QupZillaSchemeHandler();
    m_schemeHandlers["abp"] = new AdBlockSchemeHandler();
    m_schemeHandlers["file"] = new FileSchemeHandler();
    m_schemeHandlers["ftp"] = new FtpSchemeHandler();

    m_proxyFactory = new NetworkProxyFactory();
    setProxyFactory(m_proxyFactory);
    loadSettings();
}

void NetworkManager::loadSettings()
{
    Settings settings;

    if (settings.value("Web-Browser-Settings/AllowLocalCache", true).toBool() && !mApp->isPrivateSession()) {
        QNetworkDiskCache* cache = mApp->networkCache();
        cache->setMaximumCacheSize(settings.value("MaximumCacheSize", 50).toInt() * 1024 * 1024); //MegaBytes
        setCache(cache);
    }

    settings.beginGroup("Web-Browser-Settings");
    m_doNotTrack = settings.value("DoNotTrack", false).toBool();
    m_sendReferer = settings.value("SendReferer", true).toBool();
    settings.endGroup();
    m_acceptLanguage = AcceptLanguage::generateHeader(settings.value("Language/acceptLanguage", AcceptLanguage::defaultLanguage()).toStringList());

#ifdef Q_OS_WIN
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
        if (!QzTools::isCertificateValid(cert) || reply->property("downReply").toBool()) {
            return;
        }

        QNetworkRequest request = reply->request();
        QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
        WebPage* webPage = static_cast<WebPage*>(v.value<void*>());
        if (!WebPage::isPointerSafeToUse(webPage)) {
            return;
        }

        if (webPage->url().host() == reply->url().host()) {
            webPage->setSSLCertificate(cert);
        }
    }
}

inline uint qHash(const QSslCertificate &cert)
{
    return qHash(cert.toPem());
}

void NetworkManager::sslError(QNetworkReply* reply, QList<QSslError> errors)
{
    if (m_ignoreAllWarnings || reply->property("downReply").toBool()) {
        reply->ignoreSslErrors(errors);
        return;
    }

    QNetworkRequest request = reply->request();
    QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
    WebPage* webPage = static_cast<WebPage*>(v.value<void*>());
    if (!WebPage::isPointerSafeToUse(webPage)) {
        return;
    }

    QHash<QSslCertificate, QStringList> errorHash;
    foreach(const QSslError & error, errors) {
        // Weird behavior on Windows
        if (error.error() == QSslError::NoError) {
            continue;
        }

        const QSslCertificate &cert = error.certificate();

        if (errorHash.contains(cert)) {
            errorHash[cert].append(error.errorString());
        }
        else {
            errorHash.insert(cert, QStringList(error.errorString()));
        }
    }

    // User already rejected those certs on this page
    if (webPage->containsRejectedCerts(errorHash.keys())) {
        return;
    }

    QString title = tr("SSL Certificate Error!");
    QString text1 = tr("The page you are trying to access has the following errors in the SSL certificate:");

    QString certs;

    QHash<QSslCertificate, QStringList>::const_iterator i = errorHash.constBegin();
    while (i != errorHash.constEnd()) {
        const QSslCertificate &cert = i.key();
        const QStringList &errors = i.value();

        if (m_localCerts.contains(cert) || errors.isEmpty()) {
            ++i;
            continue;
        }

        certs += "<ul><li>";
        certs += tr("<b>Organization: </b>") + CertificateInfoWidget::clearCertSpecialSymbols(cert.subjectInfo(QSslCertificate::Organization));
        certs += "</li><li>";
        certs += tr("<b>Domain Name: </b>") + CertificateInfoWidget::clearCertSpecialSymbols(cert.subjectInfo(QSslCertificate::CommonName));
        certs += "</li><li>";
        certs += tr("<b>Expiration Date: </b>") + cert.expiryDate().toString("hh:mm:ss dddd d. MMMM yyyy");
        certs += "</li></ul>";

        certs += "<ul>";
        foreach(const QString & error, errors) {
            certs += "<li>";
            certs += tr("<b>Error: </b>") + error;
            certs += "</li>";
        }
        certs += "</ul>";

        ++i;
    }

    QString text2 = tr("Would you like to make an exception for this certificate?");
    QString message = QString("<b>%1</b><p>%2</p>%3<p>%4</p>").arg(title, text1, certs, text2);

    if (!certs.isEmpty())  {
        QMessageBox::StandardButton button = QMessageBox::critical(webPage->view(), tr("SSL Certificate Error!"), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (button == QMessageBox::No) {
            // To prevent asking user more than once for the same certificate
            webPage->addRejectedCerts(errorHash.keys());
            return;
        }

        foreach(const QSslCertificate & cert, errorHash.keys()) {
            if (!m_localCerts.contains(cert)) {
                addLocalCertificate(cert);
            }
        }
    }

    reply->ignoreSslErrors(errors);
}

void NetworkManager::authentication(QNetworkReply* reply, QAuthenticator* auth)
{
    QDialog* dialog = new QDialog(p_QupZilla);
    dialog->setWindowTitle(tr("Authorisation required"));

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
                      "The site says: \"%2\"").arg(reply->url().toEncoded(), QzTools::escape(auth->realm())));
    formLa->addRow(label);

    formLa->addRow(userLab, user);
    formLa->addRow(passLab, pass);
    formLa->addRow(save);

    formLa->addWidget(box);
    bool shouldUpdateEntry = false;

    AutoFill* fill = mApp->autoFill();
    QString storedUser;
    QString storedPassword;
    if (fill->isStored(reply->url())) {
        const AutoFillData &data = fill->getFirstFormData(reply->url());

        if (data.isValid()) {
            save->setChecked(true);
            shouldUpdateEntry = true;
            storedUser = data.username;
            storedPassword = data.password;
            user->setText(storedUser);
            pass->setText(storedPassword);
        }
    }

    // Try to set the originating WebTab as a current tab
    QWebFrame* frame = qobject_cast<QWebFrame*>(reply->request().originatingObject());
    if (frame) {
        WebPage* page = qobject_cast<WebPage*>(frame->page());
        if (page) {
            TabbedWebView* view = qobject_cast<TabbedWebView*>(page->view());
            if (view) {
                view->setAsCurrentTab();
            }
        }
    }

    // Do not save when private browsing is enabled
    if (mApp->isPrivateSession()) {
        save->setVisible(false);
    }

    if (dialog->exec() != QDialog::Accepted) {
        return;
    }

    auth->setUser(user->text());
    auth->setPassword(pass->text());

    if (save->isChecked()) {
        if (shouldUpdateEntry) {
            if (storedUser != user->text() || storedPassword != pass->text()) {
                fill->updateEntry(reply->url(), user->text(), pass->text());
            }
        }
        else {
            fill->addEntry(reply->url(), user->text(), pass->text());
        }
    }
}

void NetworkManager::ftpAuthentication(const QUrl &url, QAuthenticator* auth)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    FtpDownloader* ftp = 0;
    if (!reply) {
        ftp = qobject_cast<FtpDownloader*>(sender());
    }

    if (!auth) {
        auth = FTP_AUTHENTICATOR(url);
    }

    QString lastUser = auth->user();
    QString lastPass = auth->password();

    if (lastUser.isEmpty() && lastPass.isEmpty()) {
        // The auth is empty but url contains user's info
        lastUser = url.userName();
        lastPass = url.password();
    }

    QDialog* dialog = new QDialog(mApp->getWindow());
    dialog->setWindowTitle(tr("FTP authorisation required"));

    QFormLayout* formLa = new QFormLayout(dialog);

    QLabel* label = new QLabel(dialog);
    QLabel* userLab = new QLabel(dialog);
    QLabel* passLab = new QLabel(dialog);
    userLab->setText(tr("Username: "));
    passLab->setText(tr("Password: "));

    QCheckBox* anonymousLogin = new QCheckBox(dialog);
    QLineEdit* user = new QLineEdit(lastUser, dialog);
    QLineEdit* pass = new QLineEdit(lastPass, dialog);
    anonymousLogin->setText(tr("Login anonymously"));
    connect(anonymousLogin, SIGNAL(toggled(bool)), user, SLOT(setDisabled(bool)));
    connect(anonymousLogin, SIGNAL(toggled(bool)), pass, SLOT(setDisabled(bool)));
    anonymousLogin->setChecked(lastUser.isEmpty() && lastPass.isEmpty());
    pass->setEchoMode(QLineEdit::Password);

    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));

    int port = 21;
    if (url.port() != -1) {
        port = url.port();
    }

    label->setText(tr("A username and password are being requested by %1:%2.")
                   .arg(url.host(), QString::number(port)));

    formLa->addRow(label);

    formLa->addRow(anonymousLogin);
    formLa->addRow(userLab, user);
    formLa->addRow(passLab, pass);

    formLa->addWidget(box);

    if (dialog->exec() != QDialog::Accepted) {
        if (reply) {
            reply->abort();
            // is it safe?
            reply->deleteLater();
        }
        else if (ftp) {
            ftp->abort();
            ftp->close();
        }
        return;
    }

    if (!anonymousLogin->isChecked()) {
        auth->setUser(user->text());
        auth->setPassword(pass->text());
    }
    else {
        auth->setUser(QString());
        auth->setPassword(QString());
    }
}

void NetworkManager::proxyAuthentication(const QNetworkProxy &proxy, QAuthenticator* auth)
{
    QDialog* dialog = new QDialog(p_QupZilla);
    dialog->setWindowTitle(tr("Proxy authorisation required"));

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

    // SchemeHandlers
    if (m_schemeHandlers.contains(req.url().scheme())) {
        reply = m_schemeHandlers[req.url().scheme()]->createRequest(op, req, outgoingData);
        if (reply) {
            if (req.url().scheme() == "ftp") {
                QVariant v = req.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
                WebPage* webPage = static_cast<WebPage*>(v.value<void*>());
                if (webPage) {
                    connect(reply, SIGNAL(downloadRequest(const QNetworkRequest &)),
                            webPage, SLOT(downloadRequested(const QNetworkRequest &)));
                }
                connect(reply, SIGNAL(ftpAuthenticationRequierd(const QUrl &, QAuthenticator*)),
                        this, SLOT(ftpAuthentication(const QUrl &, QAuthenticator*)));
            }
            return reply;
        }
    }

    // Plugins
    reply = mApp->plugins()->createRequest(op, request, outgoingData);
    if (reply) {
        return reply;
    }

    if (req.rawHeader("X-QupZilla-UserLoadAction") == QByteArray("1")) {
        req.setRawHeader("X-QupZilla-UserLoadAction", QByteArray());
        req.setAttribute(QNetworkRequest::Attribute(QNetworkRequest::User + 151), QString());
    }
    else {
        req.setAttribute(QNetworkRequest::Attribute(QNetworkRequest::User + 151), req.rawHeader("Referer"));
    }

    if (m_doNotTrack) {
        req.setRawHeader("DNT", QByteArray("1"));
    }

    if (!m_sendReferer) {
        req.setRawHeader("Referer", QByteArray());
    }

    req.setRawHeader("Accept-Language", m_acceptLanguage);

    req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
//    if (req.attribute(QNetworkRequest::CacheLoadControlAttribute).toInt() == QNetworkRequest::PreferNetwork) {
//        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
//    }

    // Adblock
    if (op == QNetworkAccessManager::GetOperation) {
        if (!m_adblockManager) {
            m_adblockManager = AdBlockManager::instance();
        }
        reply = m_adblockManager->block(req);
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

    // Delete cert file from profile
    bool deleted = false;
    QDirIterator it(mApp->currentProfilePath() + "certificates", QDir::Files, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
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
        qWarning() << "NetworkManager::removeLocalCertificate cannot remove certificate";
    }
}

void NetworkManager::addLocalCertificate(const QSslCertificate &cert)
{
//    if (!cert.isValid()) {
//        return;
//    }

    m_localCerts.append(cert);
    QSslSocket::addDefaultCaCertificate(cert);

    QDir dir(mApp->currentProfilePath());
    if (!dir.exists("certificates")) {
        dir.mkdir("certificates");
    }

    QString certFileName = fileNameForCert(cert);
    QString fileName = QzTools::ensureUniqueFilename(mApp->currentProfilePath() + "certificates/" + certFileName);

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
#ifdef Q_OS_WIN
        // Used from Qt 4.7.4 qsslcertificate.cpp and modified because QSslCertificate::fromPath
        // is kind of a bugged on Windows, it does work only with full path to cert file
        QDirIterator it(path, QDir::Files, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            if (!filePath.endsWith(QLatin1String(".crt"))) {
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
#ifdef Q_OS_WIN
    QDirIterator it_(mApp->currentProfilePath() + "certificates", QDir::Files, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
    while (it_.hasNext()) {
        QString filePath = it_.next();
        if (!filePath.endsWith(QLatin1String(".crt"))) {
            continue;
        }

        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_localCerts += QSslCertificate::fromData(file.readAll(), QSsl::Pem);
        }
    }
#else
    m_localCerts += QSslCertificate::fromPath(mApp->currentProfilePath() + "certificates/*.crt", QSsl::Pem, QRegExp::Wildcard);
#endif

    QSslSocket::setDefaultCaCertificates(m_caCerts + m_localCerts);

    new CaBundleUpdater(this, this);
}

void NetworkManager::disconnectObjects()
{
    disconnect(this);
}
