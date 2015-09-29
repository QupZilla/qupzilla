/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2015  David Rosca <nowrep@gmail.com>
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
#include "webpage.h"
#include "tabbedwebview.h"
#include "browserwindow.h"
#include "pluginproxy.h"
#include "downloadmanager.h"
#include "mainapplication.h"
#include "checkboxdialog.h"
#include "widget.h"
#include "qztools.h"
#include "speeddial.h"
#include "autofill.h"
#include "popupwebview.h"
#include "popupwindow.h"
#include "networkmanagerproxy.h"
#include "adblockicon.h"
#include "adblockmanager.h"
#include "iconprovider.h"
#include "qzsettings.h"
#include "useragentmanager.h"
#include "delayedfilewatcher.h"
#include "searchenginesmanager.h"
#include "html5permissions/html5permissionsmanager.h"
#include "schemehandlers/fileschemehandler.h"
#include "javascript/externaljsobject.h"
#include "tabwidget.h"
#include "scripts.h"
#include "sslerrordialog.h"

#ifdef NONBLOCK_JS_DIALOGS
#include "ui_jsconfirm.h"
#include "ui_jsalert.h"
#include "ui_jsprompt.h"

#include <QPushButton>
#endif

#include <QAuthenticator>
#include <QDir>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QWebEngineHistory>
#include <QWebEngineSettings>
#include <QTimer>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFileDialog>
#include <QCheckBox>
#include <QWebChannel>

QString WebPage::s_lastUploadLocation = QDir::homePath();
QUrl WebPage::s_lastUnsupportedUrl;
QTime WebPage::s_lastUnsupportedUrlTime;
QStringList WebPage::s_ignoredSslErrors;

WebPage::WebPage(QObject* parent)
    : QWebEnginePage(mApp->webProfile(), parent)
    , m_fileWatcher(0)
    , m_runningLoop(0)
    , m_loadProgress(-1)
    , m_blockAlerts(false)
    , m_secureStatus(false)
    , m_adjustingScheduled(false)
{
    setupWebChannel();

    connect(this, &QWebEnginePage::loadProgress, this, &WebPage::progress);
    connect(this, &QWebEnginePage::loadFinished, this, &WebPage::finished);
    connect(this, &QWebEnginePage::urlChanged, this, &WebPage::urlChanged);
    connect(this, &QWebEnginePage::featurePermissionRequested, this, &WebPage::featurePermissionRequested);
    connect(this, &QWebEnginePage::windowCloseRequested, this, &WebPage::windowCloseRequested);
}

WebPage::~WebPage()
{
    mApp->plugins()->emitWebPageDeleted(this);

    if (m_runningLoop) {
        m_runningLoop->exit(1);
        m_runningLoop = 0;
    }
}

WebView *WebPage::view() const
{
    return static_cast<WebView*>(QWebEnginePage::view());
}

void WebPage::scheduleAdjustPage()
{
    if (view()->isLoading()) {
        m_adjustingScheduled = true;
    }
    else {
        const QSize originalSize = view()->size();
        QSize newSize(originalSize.width() - 1, originalSize.height() - 1);

        view()->resize(newSize);
        view()->resize(originalSize);
    }
}

void WebPage::addRejectedCerts(const QList<QSslCertificate> &certs)
{
    foreach (const QSslCertificate &cert, certs) {
        if (!m_rejectedSslCerts.contains(cert)) {
            m_rejectedSslCerts.append(cert);
        }
    }
}

bool WebPage::containsRejectedCerts(const QList<QSslCertificate> &certs)
{
    int matches = 0;

    foreach (const QSslCertificate &cert, certs) {
        if (m_rejectedSslCerts.contains(cert)) {
            ++matches;
        }

        if (m_sslCert == cert) {
            m_sslCert.clear();
        }
    }

    return matches == certs.count();
}

bool WebPage::isRunningLoop()
{
    return m_runningLoop;
}

bool WebPage::isLoading() const
{
    return m_loadProgress < 100;
}

void WebPage::urlChanged(const QUrl &url)
{
    Q_UNUSED(url)

    if (isLoading()) {
        m_adBlockedEntries.clear();
        m_blockAlerts = false;
    }
}

void WebPage::progress(int prog)
{
    m_loadProgress = prog;

    bool secStatus = QzTools::isCertificateValid(sslCertificate());

    if (secStatus != m_secureStatus) {
        m_secureStatus = secStatus;
        emit privacyChanged(QzTools::isCertificateValid(sslCertificate()));
    }
}

void WebPage::finished()
{
    progress(100);

    if (m_adjustingScheduled) {
        m_adjustingScheduled = false;
        setZoomFactor(zoomFactor() + 1);
        setZoomFactor(zoomFactor() - 1);
    }

    // File scheme watcher
    if (url().scheme() == QLatin1String("file")) {
        QFileInfo info(url().toLocalFile());
        if (info.isFile()) {
            if (!m_fileWatcher) {
                m_fileWatcher = new DelayedFileWatcher(this);
                connect(m_fileWatcher, SIGNAL(delayedFileChanged(QString)), this, SLOT(watchedFileChanged(QString)));
            }

            const QString filePath = url().toLocalFile();

            if (QFile::exists(filePath) && !m_fileWatcher->files().contains(filePath)) {
                m_fileWatcher->addPath(filePath);
            }
        }
    }
    else if (m_fileWatcher && !m_fileWatcher->files().isEmpty()) {
        m_fileWatcher->removePaths(m_fileWatcher->files());
    }

    // AdBlock
    cleanBlockedObjects();

    // AutoFill
    m_passwordEntries = mApp->autoFill()->completePage(this, url());
}

void WebPage::watchedFileChanged(const QString &file)
{
    if (url().toLocalFile() == file) {
        triggerAction(QWebEnginePage::Reload);
    }
}

void WebPage::handleUnknownProtocol(const QUrl &url)
{
    const QString protocol = url.scheme();

    if (protocol == QLatin1String("mailto")) {
        desktopServicesOpen(url);
        return;
    }

    if (qzSettings->blockedProtocols.contains(protocol)) {
        qDebug() << "WebPage::handleUnknownProtocol Protocol" << protocol << "is blocked!";
        return;
    }

    if (qzSettings->autoOpenProtocols.contains(protocol)) {
        desktopServicesOpen(url);
        return;
    }

    CheckBoxDialog dialog(QDialogButtonBox::Yes | QDialogButtonBox::No, view());

    const QString wrappedUrl = QzTools::alignTextToWidth(url.toString(), "<br/>", dialog.fontMetrics(), 450);
    const QString text = tr("QupZilla cannot handle <b>%1:</b> links. The requested link "
                            "is <ul><li>%2</li></ul>Do you want QupZilla to try "
                            "open this link in system application?").arg(protocol, wrappedUrl);

    dialog.setText(text);
    dialog.setCheckBoxText(tr("Remember my choice for this protocol"));
    dialog.setWindowTitle(tr("External Protocol Request"));
    dialog.setIcon(IconProvider::standardIcon(QStyle::SP_MessageBoxQuestion));

    switch (dialog.exec()) {
    case QDialog::Accepted:
        if (dialog.isChecked()) {
            qzSettings->autoOpenProtocols.append(protocol);
            qzSettings->saveSettings();
        }


        QDesktopServices::openUrl(url);
        break;

    case QDialog::Rejected:
        if (dialog.isChecked()) {
            qzSettings->blockedProtocols.append(protocol);
            qzSettings->saveSettings();
        }

        break;

    default:
        break;
    }
}

void WebPage::desktopServicesOpen(const QUrl &url)
{
    // Open same url only once in 2 secs
    const int sameUrlTimeout = 2 * 1000;

    if (s_lastUnsupportedUrl != url || s_lastUnsupportedUrlTime.isNull() || s_lastUnsupportedUrlTime.elapsed() > sameUrlTimeout) {
        s_lastUnsupportedUrl = url;
        s_lastUnsupportedUrlTime.restart();
        QDesktopServices::openUrl(url);
    }
    else {
        qWarning() << "WebPage::desktopServicesOpen Url" << url << "has already been opened!\n"
                   "Ignoring it to prevent infinite loop!";
    }
}

void WebPage::setupWebChannel()
{
    QWebChannel *old = webChannel();
    const QString objectName = QSL("qz_object");

    QWebChannel *channel = new QWebChannel(this);
    channel->registerObject(QSL("qz_object"), new ExternalJsObject(this));
    setWebChannel(channel);

    if (old) {
        delete old->registeredObjects().value(objectName);
        delete old;
    }
}

void WebPage::windowCloseRequested()
{
    view()->closeView();
}

void WebPage::authentication(const QUrl &requestUrl, QAuthenticator* auth)
{
    QDialog* dialog = new QDialog();
    dialog->setWindowTitle(tr("Authorisation required"));

    QFormLayout* formLa = new QFormLayout(dialog);

    QLabel* label = new QLabel(dialog);
    QLabel* userLab = new QLabel(dialog);
    QLabel* passLab = new QLabel(dialog);
    userLab->setText(tr("Username: "));
    passLab->setText(tr("Password: "));

    QLineEdit* user = new QLineEdit(dialog);
    QLineEdit* pass = new QLineEdit(dialog);
    pass->setEchoMode(QLineEdit::Password);
    QCheckBox* save = new QCheckBox(dialog);
    save->setText(tr("Save username and password on this site"));

    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));

    label->setText(tr("A username and password are being requested by %1. "
                      "The site says: \"%2\"").arg(requestUrl.host(), QzTools::escape(auth->realm())));

    formLa->addRow(label);
    formLa->addRow(userLab, user);
    formLa->addRow(passLab, pass);
    formLa->addRow(save);
    formLa->addWidget(box);

    AutoFill* fill = mApp->autoFill();
    QString storedUser;
    QString storedPassword;
    bool shouldUpdateEntry = false;

    if (fill->isStored(requestUrl)) {
        const QVector<PasswordEntry> &data = fill->getFormData(requestUrl);
        if (!data.isEmpty()) {
            save->setChecked(true);
            shouldUpdateEntry = true;
            storedUser = data.first().username;
            storedPassword = data.first().password;
            user->setText(storedUser);
            pass->setText(storedPassword);
        }
    }

    // Try to set the originating WebTab as a current tab
    TabbedWebView* tabView = qobject_cast<TabbedWebView*>(view());
    if (tabView) {
        tabView->setAsCurrentTab();
    }

    // Do not save when private browsing is enabled
    if (mApp->isPrivate()) {
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
                fill->updateEntry(requestUrl, user->text(), pass->text());
            }
        }
        else {
            fill->addEntry(requestUrl, user->text(), pass->text());
        }
    }
}

void WebPage::proxyAuthentication(const QUrl &requestUrl, QAuthenticator* auth, const QString &proxyHost)
{
    Q_UNUSED(requestUrl)

    QVector<PasswordEntry> passwords = mApp->autoFill()->getFormData(QUrl(proxyHost));
    if (!passwords.isEmpty()) {
        auth->setUser(passwords.at(0).username);
        auth->setPassword(passwords.at(0).password);
        return;
    }

    QDialog* dialog = new QDialog();
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
    QCheckBox* save = new QCheckBox(dialog);
    save->setText(tr("Remember username and password for this proxy."));

    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));

    label->setText(tr("A username and password are being requested by proxy %1. ").arg(proxyHost));
    formLa->addRow(label);
    formLa->addRow(userLab, user);
    formLa->addRow(passLab, pass);
    formLa->addRow(save);
    formLa->addWidget(box);

    if (dialog->exec() != QDialog::Accepted) {
        return;
    }

    if (save->isChecked()) {
        mApp->autoFill()->addEntry(QUrl(proxyHost), user->text(), pass->text());
    }

    auth->setUser(user->text());
    auth->setPassword(pass->text());
}

void WebPage::doWebSearch(const QString &text)
{
    const LoadRequest searchRequest = mApp->searchEnginesManager()->searchResult(text);
    view()->load(searchRequest);
}

void WebPage::featurePermissionRequested(const QUrl &origin, const QWebEnginePage::Feature &feature)
{
    mApp->html5PermissionsManager()->requestPermissions(this, origin, feature);
}

bool WebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    m_lastRequestUrl = url;

    if (!mApp->plugins()->acceptNavigationRequest(this, url, type, isMainFrame))
        return false;

    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);

#if QTWEBENGINE_DISABLED
    if (type == QWebEnginePage::NavigationTypeFormResubmitted) {
        // Don't show this dialog if app is still starting
        if (!view() || !view()->isVisible()) {
            return false;
        }
        QString message = tr("To display this page, QupZilla must resend the request \n"
                             "(such as a search or order confirmation) that was performed earlier.");
        bool result = (QMessageBox::question(view(), tr("Confirm form resubmission"),
                                             message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes);
        if (!result) {
            return false;
        }
    }
#endif
}

bool WebPage::certificateError(const QWebEngineCertificateError &certificateError)
{
    if (s_ignoredSslErrors.contains(certificateError.url().host()))
        return true;

    QString title = tr("SSL Certificate Error!");
    QString text1 = tr("The page you are trying to access has the following errors in the SSL certificate:");
    QString text2 = tr("Would you like to make an exception for this certificate?");

    QString message = QSL("<b>%1</b><p>%2</p><ul><li>%3</li></ul><p>%4</p>").arg(title, text1, certificateError.errorDescription(), text2);

    SslErrorDialog dialog(view());
    dialog.setText(message);
    dialog.exec();

    switch (dialog.result()) {
    case SslErrorDialog::Yes:
        // TODO: Permanent exceptions
        s_ignoredSslErrors.append(certificateError.url().host());
        return true;

    case SslErrorDialog::OnlyForThisSession:
        s_ignoredSslErrors.append(certificateError.url().host());
        return true;

    case SslErrorDialog::No:
    default:
        return false;
    }
}

QStringList WebPage::chooseFiles(QWebEnginePage::FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes)
{
    Q_UNUSED(acceptedMimeTypes);

    QStringList files;
    QString suggestedFileName = s_lastUploadLocation;
    if (!oldFiles.isEmpty())
        suggestedFileName = oldFiles.first();

    switch (mode) {
    case FileSelectOpen:
        files = QStringList(QzTools::getOpenFileName("WebPage-ChooseFile", view(), tr("Choose file..."), suggestedFileName));
        break;

    case FileSelectOpenMultiple:
        files = QzTools::getOpenFileNames("WebPage-ChooseFile", view(), tr("Choose files..."), suggestedFileName);
        break;

    default:
        files = QWebEnginePage::chooseFiles(mode, oldFiles, acceptedMimeTypes);
        break;
    }

    if (!files.isEmpty())
        s_lastUploadLocation = files.first();

    return files;
}

void WebPage::setSSLCertificate(const QSslCertificate &cert)
{
    //    if (cert != m_SslCert)
    m_sslCert = cert;
}

QSslCertificate WebPage::sslCertificate()
{
    if (url().scheme() == QLatin1String("https") && QzTools::isCertificateValid(m_sslCert)) {
        return m_sslCert;
    }

    return QSslCertificate();
}

void WebPage::addAdBlockRule(const AdBlockRule* rule, const QUrl &url)
{
    AdBlockedEntry entry;
    entry.rule = rule;
    entry.url = url;

    if (!m_adBlockedEntries.contains(entry)) {
        m_adBlockedEntries.append(entry);
    }
}

QVector<WebPage::AdBlockedEntry> WebPage::adBlockedEntries() const
{
    return m_adBlockedEntries;
}

bool WebPage::hasMultipleUsernames() const
{
    return m_passwordEntries.count() > 1;
}

QVector<PasswordEntry> WebPage::autoFillData() const
{
    return m_passwordEntries;
}

void WebPage::cleanBlockedObjects()
{
    AdBlockManager* manager = AdBlockManager::instance();
    if (!manager->isEnabled()) {
        return;
    }

#if QTWEBENGINE_DISABLED
    const QWebElement docElement = mainFrame()->documentElement();

    foreach (const AdBlockedEntry &entry, m_adBlockedEntries) {
        const QString urlString = entry.url.toString();
        if (urlString.endsWith(QLatin1String(".js")) || urlString.endsWith(QLatin1String(".css"))) {
            continue;
        }

        QString urlEnd;

        int pos = urlString.lastIndexOf(QLatin1Char('/'));
        if (pos > 8) {
            urlEnd = urlString.mid(pos + 1);
        }

        if (urlString.endsWith(QLatin1Char('/'))) {
            urlEnd = urlString.left(urlString.size() - 1);
        }

        QString selector("img[src$=\"%1\"], iframe[src$=\"%1\"],embed[src$=\"%1\"]");
        QWebElementCollection elements = docElement.findAll(selector.arg(urlEnd));

        foreach (QWebElement element, elements) {
            QString src = element.attribute("src");
            src.remove(QLatin1String("../"));

            if (urlString.contains(src)) {
                element.setStyleProperty("display", "none");
            }
        }
    }
#endif

    // Apply domain-specific element hiding rules
    const QString elementHiding = manager->elementHidingRulesForDomain(url());
    if (elementHiding.isEmpty()) {
        return;
    }

    runJavaScript(Scripts::setCss(elementHiding));
}

#if QTWEBENGINE_DISABLED
bool WebPage::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension)

    return true;
}

bool WebPage::extension(Extension extension, const ExtensionOption* option, ExtensionReturn* output)
{
    if (extension == ChooseMultipleFilesExtension) {
        const QWebEnginePage::ChooseMultipleFilesExtensionOption* exOption = static_cast<const QWebEnginePage::ChooseMultipleFilesExtensionOption*>(option);
        QWebEnginePage::ChooseMultipleFilesExtensionReturn* exReturn = static_cast<QWebEnginePage::ChooseMultipleFilesExtensionReturn*>(output);

        if (!exOption || !exReturn) {
            return QWebEnginePage::extension(extension, option, output);
        }

        QString suggestedFileName;
        if (!exOption->suggestedFileNames.isEmpty()) {
            suggestedFileName = exOption->suggestedFileNames.at(0);
        }

        exReturn->fileNames = QzTools::getOpenFileNames("WebPage-UploadFiles", 0, tr("Select files to upload..."), suggestedFileName);
        return true;
    }

    const ErrorPageExtensionOption* exOption = static_cast<const QWebEnginePage::ErrorPageExtensionOption*>(option);
    ErrorPageExtensionReturn* exReturn = static_cast<QWebEnginePage::ErrorPageExtensionReturn*>(output);

    if (!exOption || !exReturn) {
        return QWebEnginePage::extension(extension, option, output);
    }

    WebPage* erPage = qobject_cast<WebPage*>(exOption->frame->page());

    if (!erPage) {
        return QWebEnginePage::extension(extension, option, output);
    }

    QString errorString;
    if (exOption->domain == QWebEnginePage::QtNetwork) {
        switch (exOption->error) {
        case QNetworkReply::ConnectionRefusedError:
            errorString = tr("Server refused the connection");
            break;
        case QNetworkReply::RemoteHostClosedError:
            errorString = tr("Server closed the connection");
            break;
        case QNetworkReply::HostNotFoundError:
            // If a one-word host was not find, search for the text instead
            // It needs to be async to correctly refresh loading state
            if (!exOption->url.host().isEmpty() && !exOption->url.host().contains(QL1C('.'))) {
                const QString text = QzTools::fromPunycode(exOption->url.host().toUtf8());
                QMetaObject::invokeMethod(this, "doWebSearch", Qt::QueuedConnection, Q_ARG(QString, text));
                return false;
            }
            errorString = tr("Server not found");
            break;
        case QNetworkReply::TimeoutError:
            errorString = tr("Connection timed out");
            break;
        case QNetworkReply::SslHandshakeFailedError:
            errorString = tr("Untrusted connection");
            break;
        case QNetworkReply::TemporaryNetworkFailureError:
            errorString = tr("Temporary network failure");
            break;
        case QNetworkReply::ProxyConnectionRefusedError:
            errorString = tr("Proxy connection refused");
            break;
        case QNetworkReply::ProxyNotFoundError:
            errorString = tr("Proxy server not found");
            break;
        case QNetworkReply::ProxyTimeoutError:
            errorString = tr("Proxy connection timed out");
            break;
        case QNetworkReply::ProxyAuthenticationRequiredError:
            errorString = tr("Proxy authentication required");
            break;
        case QNetworkReply::ContentNotFoundError:
            errorString = tr("Content not found");
            break;
        case QNetworkReply::UnknownNetworkError:
            errorString = exOption->errorString.isEmpty() ? tr("Unknown network error") : exOption->errorString;
            break;
        case QNetworkReply::ProtocolUnknownError: {
            // Sometimes exOption->url returns just "?" instead of actual url
            const QUrl unknownProtocolUrl = (exOption->url.toString() == QLatin1String("?")) ? erPage->mainFrame()->requestedUrl() : exOption->url;
            handleUnknownProtocol(unknownProtocolUrl);
            return false;
        }
        case QNetworkReply::ContentAccessDenied:
            if (exOption->errorString.startsWith(QLatin1String("AdBlock"))) {
                if (exOption->frame != erPage->mainFrame()) {
                    // Content in <iframe>
                    QWebElement docElement = erPage->mainFrame()->documentElement();

                    QWebElementCollection elements;
                    elements.append(docElement.findAll(QSL("iframe")));

                    foreach (QWebElement element, elements) {
                        const QString src = element.attribute(QSL("src"));
                        if (!src.isEmpty() && exOption->url.toString().contains(src)) {
                            element.setStyleProperty(QSL("display"), QSL("none"));
                        }
                    }

                    return false;
                }
                else {
                    // The whole page is blocked
                    QString rule = exOption->errorString;
                    rule.remove(QLatin1String("AdBlock: "));

                    QString errString = QzTools::readAllFileContents(":/html/adblockPage.html");
                    errString.replace(QLatin1String("%TITLE%"), tr("AdBlocked Content"));
                    errString.replace(QLatin1String("%IMAGE%"), QLatin1String("qrc:html/adblock_big.png"));
                    errString.replace(QLatin1String("%FAVICON%"), QLatin1String("qrc:html/adblock_big.png"));

                    errString.replace(QLatin1String("%RULE%"), tr("Blocked by <i>%1</i>").arg(rule));
                    errString = QzTools::applyDirectionToPage(errString);

                    exReturn->baseUrl = exOption->url;
                    exReturn->content = QString(errString + "<span id=\"qupzilla-error-page\"></span>").toUtf8();

                    if (PopupWebPage* popupPage = qobject_cast<PopupWebPage*>(exOption->frame->page())) {
                        WebView* view = qobject_cast<WebView*>(popupPage->view());
                        if (view) {
                            // Closing blocked popup
                            popupPage->mainWindow()->adBlockIcon()->popupBlocked(rule, exOption->url);
                            view->closeView();
                        }
                    }
                    return true;
                }
            }
            errorString = tr("Content Access Denied");
            break;
        default:
            if (exOption->errorString != QLatin1String("QupZilla:No Error")) {
                qDebug() << "Content error: " << exOption->errorString << exOption->error;
            }
            return false;
        }
    }
    else if (exOption->domain == QWebEnginePage::Http) {
        // 200 status code = OK
        // It shouldn't be reported as an error, but sometimes it is ...
        if (exOption->error == 200) {
            return false;
        }
        errorString = tr("Error code %1").arg(exOption->error);
    }
    else if (exOption->domain == QWebEnginePage::WebKit) {
        return false;    // Downloads
    }

    const QUrl loadedUrl = exOption->url;
    exReturn->baseUrl = loadedUrl;

    QFile file(":/html/errorPage.html");
    file.open(QFile::ReadOnly);
    QString errString = file.readAll();
    errString.replace(QLatin1String("%TITLE%"), tr("Failed loading page"));

    errString.replace(QLatin1String("%IMAGE%"), QzTools::pixmapToByteArray(IconProvider::standardIcon(QStyle::SP_MessageBoxWarning).pixmap(45, 45)));
    errString.replace(QLatin1String("%FAVICON%"), QzTools::pixmapToByteArray(IconProvider::standardIcon(QStyle::SP_MessageBoxWarning).pixmap(16, 16)));
    errString.replace(QLatin1String("%BOX-BORDER%"), QLatin1String("qrc:html/box-border.png"));

    QString heading2 = loadedUrl.host().isEmpty() ? tr("QupZilla can't load page.") : tr("QupZilla can't load page from %1.").arg(loadedUrl.host());

    errString.replace(QLatin1String("%HEADING%"), errorString);
    errString.replace(QLatin1String("%HEADING2%"), heading2);
    errString.replace(QLatin1String("%LI-1%"), tr("Check the address for typing errors such as <b>ww.</b>example.com instead of <b>www.</b>example.com"));
    errString.replace(QLatin1String("%LI-2%"), tr("If you are unable to load any pages, check your computer's network connection."));
    errString.replace(QLatin1String("%LI-3%"), tr("If your computer or network is protected by a firewall or proxy, make sure that QupZilla is permitted to access the Web."));
    errString.replace(QLatin1String("%TRY-AGAIN%"), tr("Try Again"));
    errString = QzTools::applyDirectionToPage(errString);

    exReturn->content = QString(errString + "<span id=\"qupzilla-error-page\"></span>").toUtf8();
    return true;
}
#endif

bool WebPage::javaScriptPrompt(const QUrl &securityOrigin, const QString &msg, const QString &defaultValue, QString* result)
{
    Q_UNUSED(securityOrigin)

#ifndef NONBLOCK_JS_DIALOGS
    return QWebEnginePage::javaScriptPrompt(securityOrigin, msg, defaultValue, result);
#else
    if (m_runningLoop) {
        return false;
    }

    ResizableFrame* widget = new ResizableFrame(view()->overlayWidget());

    widget->setObjectName("jsFrame");
    Ui_jsPrompt* ui = new Ui_jsPrompt();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->lineEdit->setText(defaultValue);
    ui->lineEdit->setFocus();
    widget->resize(view()->size());
    widget->show();

    connect(view(), SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));
    connect(ui->lineEdit, SIGNAL(returnPressed()), ui->buttonBox->button(QDialogButtonBox::Ok), SLOT(animateClick()));

    QEventLoop eLoop;
    m_runningLoop = &eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));

    if (eLoop.exec() == 1) {
        return result;
    }
    m_runningLoop = 0;

    QString x = ui->lineEdit->text();
    bool _result = ui->buttonBox->clickedButtonRole() == QDialogButtonBox::AcceptRole;
    *result = x;

    delete widget;
    view()->setFocus();

    return _result;
#endif
}

bool WebPage::javaScriptConfirm(const QUrl &securityOrigin, const QString &msg)
{
    Q_UNUSED(securityOrigin)

#ifndef NONBLOCK_JS_DIALOGS
    return QWebEnginePage::javaScriptConfirm(securityOrigin, msg);
#else
    if (m_runningLoop) {
        return false;
    }

    ResizableFrame* widget = new ResizableFrame(view()->overlayWidget());

    widget->setObjectName("jsFrame");
    Ui_jsConfirm* ui = new Ui_jsConfirm();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();
    widget->resize(view()->size());
    widget->show();

    connect(view(), SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));

    QEventLoop eLoop;
    m_runningLoop = &eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));

    if (eLoop.exec() == 1) {
        return false;
    }
    m_runningLoop = 0;

    bool result = ui->buttonBox->clickedButtonRole() == QDialogButtonBox::AcceptRole;

    delete widget;
    view()->setFocus();

    return result;
#endif
}

void WebPage::javaScriptAlert(const QUrl &securityOrigin, const QString &msg)
{
    Q_UNUSED(securityOrigin)

    if (m_blockAlerts || m_runningLoop) {
        return;
    }

#ifndef NONBLOCK_JS_DIALOGS
    QString title = tr("JavaScript alert");
    if (!url().host().isEmpty()) {
        title.append(QString(" - %1").arg(url().host()));
    }

    CheckBoxDialog dialog(QDialogButtonBox::Ok, view());
    dialog.setWindowTitle(title);
    dialog.setText(msg);
    dialog.setCheckBoxText(tr("Prevent this page from creating additional dialogs"));
    dialog.setIcon(IconProvider::standardIcon(QStyle::SP_MessageBoxInformation));
    dialog.exec();

    m_blockAlerts = dialog.isChecked();
#else
    ResizableFrame* widget = new ResizableFrame(view()->overlayWidget());

    widget->setObjectName("jsFrame");
    Ui_jsAlert* ui = new Ui_jsAlert();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();
    widget->resize(view()->size());
    widget->show();

    connect(view(), SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));

    QEventLoop eLoop;
    m_runningLoop = &eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));

    if (eLoop.exec() == 1) {
        return;
    }
    m_runningLoop = 0;

    m_blockAlerts = ui->preventAlerts->isChecked();

    delete widget;

    view()->setFocus();
#endif
}

void WebPage::setJavaScriptEnabled(bool enabled)
{
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, enabled);
}

QWebEnginePage* WebPage::createWindow(QWebEnginePage::WebWindowType type)
{
    TabbedWebView *tView = qobject_cast<TabbedWebView*>(view());
    BrowserWindow *window = tView ? tView->browserWindow() : mApp->getWindow();

    switch (type) {
    case QWebEnginePage::WebBrowserWindow:
        // TODO: Investigate when this is called from html
        qDebug() << "WebBrowserWindow";
    case QWebEnginePage::WebBrowserTab: {
        int index = window->tabWidget()->addView(QUrl(), Qz::NT_CleanSelectedTab);
        TabbedWebView* view = window->weView(index);
        view->setPage(new WebPage);
        return view->page();
    }

    case QWebEnginePage::WebDialog: {
        PopupWebView* view = new PopupWebView;
        view->setPage(new WebPage);
        PopupWindow* popup = new PopupWindow(view);
        popup->show();
        window->addDeleteOnCloseWidget(popup);
        return view->page();
    }

    default:
        return 0;
    }
}
