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
#include "webpage.h"
#include "tabbedwebview.h"
#include "tabwidget.h"
#include "qupzilla.h"
#include "downloadmanager.h"
#include "webpluginfactory.h"
#include "mainapplication.h"
#ifdef NONBLOCK_JS_DIALOGS
#include "ui_jsconfirm.h"
#include "ui_jsalert.h"
#include "ui_jsprompt.h"
#endif
#include "ui_closedialog.h"
#include "widget.h"
#include "globalfunctions.h"
#include "pluginproxy.h"
#include "speeddial.h"
#include "popupwebpage.h"
#include "popupwebview.h"
#include "networkmanagerproxy.h"

QString WebPage::UserAgent = "";
QString WebPage::m_lastUploadLocation = QDir::homePath();

WebPage::WebPage(QupZilla* mainClass)
    : QWebPage()
    , p_QupZilla(mainClass)
    , m_view(0)
    , m_speedDial(mApp->plugins()->speedDial())
    , m_fileWatcher(0)
    , m_runningLoop(0)
    , m_blockAlerts(false)
    , m_secureStatus(false)
    , m_isClosing(false)
{
    m_networkProxy = new NetworkManagerProxy(this);
    m_networkProxy->setPrimaryNetworkAccessManager(mApp->networkManager());
    m_networkProxy->setPage(this);
    setNetworkAccessManager(m_networkProxy);

    setForwardUnsupportedContent(true);
    setPluginFactory(new WebPluginFactory(this));
    history()->setMaximumItemCount(20);

    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(handleUnsupportedContent(QNetworkReply*)));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(progress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(finished()));
    connect(this, SIGNAL(printRequested(QWebFrame*)), this, SLOT(printFrame(QWebFrame*)));
    connect(this, SIGNAL(downloadRequested(QNetworkRequest)), this, SLOT(downloadRequested(QNetworkRequest)));

    connect(mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()));
}

QUrl WebPage::url() const
{
    return mainFrame()->url();
}

void WebPage::setWebView(TabbedWebView* view)
{
    if (m_view == view) {
        return;
    }

    if (m_view) {
        delete m_view;
        m_view = 0;
    }

    m_view = view;
    m_view->setWebPage(this);

    connect(m_view, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged(QUrl)));
}

void WebPage::scheduleAdjustPage()
{
    if (m_view && m_view->isLoading()) {
        m_adjustingScheduled = true;
    }
    else {
        mainFrame()->setZoomFactor(mainFrame()->zoomFactor() + 1);
        mainFrame()->setZoomFactor(mainFrame()->zoomFactor() - 1);
    }
}

bool WebPage::isRunningLoop()
{
    return m_runningLoop;
}

void WebPage::urlChanged(const QUrl &url)
{
    Q_UNUSED(url)
    m_adBlockedEntries.clear();
    m_blockAlerts = false;
}

void WebPage::progress(int prog)
{
    Q_UNUSED(prog)
    bool secStatus = sslCertificate().isValid();

    if (secStatus != m_secureStatus) {
        m_secureStatus = secStatus;
        emit privacyChanged(sslCertificate().isValid());
    }
}

void WebPage::finished()
{
    progress(100);

    if (m_adjustingScheduled) {
        m_adjustingScheduled = false;
        mainFrame()->setZoomFactor(mainFrame()->zoomFactor() + 1);
        mainFrame()->setZoomFactor(mainFrame()->zoomFactor() - 1);
    }

    if (url().scheme() == "file") {
        if (!m_fileWatcher) {
            m_fileWatcher = new QFileSystemWatcher(this);
            connect(m_fileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(watchedFileChanged(QString)));
        }

        QString filePath = url().toLocalFile();

        if (QFile::exists(filePath) && !m_fileWatcher->files().contains(filePath)) {
            m_fileWatcher->addPath(filePath);
        }
    }
    else if (m_fileWatcher && !m_fileWatcher->files().isEmpty()) {
        m_fileWatcher->removePaths(m_fileWatcher->files());
    }

    QTimer::singleShot(100, this, SLOT(cleanBlockedObjects()));
}

void WebPage::watchedFileChanged(const QString &file)
{
    if (url().toLocalFile() == file) {
        triggerAction(QWebPage::Reload);
    }
}

void WebPage::printFrame(QWebFrame* frame)
{
    WebView* webView = qobject_cast<WebView*>(view());
    if (!webView) {
        return;
    }

    webView->printPage(frame);
}

void WebPage::addJavaScriptObject()
{
    if (url().toString() != "qupzilla:speeddial") {
        return;
    }

    mainFrame()->addToJavaScriptWindowObject("speeddial", m_speedDial);
    m_speedDial->addWebFrame(mainFrame());
}

void WebPage::handleUnsupportedContent(QNetworkReply* reply)
{
    if (!reply) {
        return;
    }
    QUrl url = reply->url();

    switch (reply->error()) {
    case QNetworkReply::NoError:
        if (reply->header(QNetworkRequest::ContentTypeHeader).isValid()) {
            DownloadManager* dManager = mApp->downManager();
            dManager->handleUnsupportedContent(reply, this);
            return;
        }
        break;
    case QNetworkReply::ProtocolUnknownError:
        qDebug() << url << "ProtocolUnknowError";
        QDesktopServices::openUrl(url);
        return;
        break;
    default:
        break;
    }
    qDebug() << "WebPage::UnsupportedContent error" << reply->errorString();
}

void WebPage::downloadRequested(const QNetworkRequest &request)
{
    DownloadManager* dManager = mApp->downManager();
    dManager->download(request, this);
}


void WebPage::setSSLCertificate(const QSslCertificate &cert)
{
//    if (cert != m_SslCert)
    m_SslCert = cert;
}

QSslCertificate WebPage::sslCertificate()
{
    if (url().scheme() == "https" &&
            m_SslCert.subjectInfo(QSslCertificate::CommonName).remove("*").contains(QRegExp(url().host()))) {
        return m_SslCert;
    }
    else {
        return QSslCertificate();
    }
}

bool WebPage::acceptNavigationRequest(QWebFrame* frame, const QNetworkRequest &request, NavigationType type)
{
    m_lastRequest = request;
    m_lastRequestType = type;
    QString scheme = request.url().scheme();
    if (scheme == "mailto" || scheme == "ftp") {
        QDesktopServices::openUrl(request.url());
        return false;
    }

    if (type == QWebPage::NavigationTypeFormResubmitted) {
        QString message = tr("To show this page, QupZilla must resend request which do it again \n"
                             "(like searching on making an shoping, which has been already done.)");
        bool result = (QMessageBox::question(view(), tr("Confirm form resubmission"),
                                             message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes);
        if (!result) {
            return false;
        }
    }

    bool accept = QWebPage::acceptNavigationRequest(frame, request, type);
    return accept;
}

void WebPage::populateNetworkRequest(QNetworkRequest &request)
{
    WebPage* pagePointer = this;

    QVariant variant = qVariantFromValue((void*) pagePointer);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100), variant);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 101), m_lastRequestType);
}

QWebPage* WebPage::createWindow(QWebPage::WebWindowType type)
{
#if 0
    Q_UNUSED(type);
    int index = p_QupZilla->tabWidget()->addView(QUrl(), TabWidget::CleanSelectedPage);
    return p_QupZilla->weView(index)->page();
#endif

    return new PopupWebPage(type, p_QupZilla);
}

void WebPage::addAdBlockRule(const QString &filter, const QUrl &url)
{
    AdBlockedEntry entry;
    entry.rule = filter;
    entry.url = url;

    if (!m_adBlockedEntries.contains(entry)) {
        m_adBlockedEntries.append(entry);
    }
}

void WebPage::cleanBlockedObjects()
{
    QStringList findingStrings;

    foreach(AdBlockedEntry entry, m_adBlockedEntries) {
        if (entry.url.toString().endsWith(".js")) {
            continue;
        }

        findingStrings.append(entry.url.toString());
        QUrl mainFrameUrl = url();
        if (entry.url.scheme() == mainFrameUrl.scheme() && entry.url.host() == mainFrameUrl.host()) {
            //May be relative url
            QString relativeUrl = qz_makeRelativeUrl(mainFrameUrl, entry.url).toString();
            findingStrings.append(relativeUrl);
            if (relativeUrl.startsWith("/")) {
                findingStrings.append(relativeUrl.right(relativeUrl.size() - 1));
            }
        }
    }

    QWebElement docElement = mainFrame()->documentElement();
    QWebElementCollection elements;
    foreach(QString s, findingStrings)
    elements.append(docElement.findAll("*[src=\"" + s + "\"]"));
    foreach(QWebElement element, elements)
    element.setAttribute("style", "display:none;");
}

QString WebPage::userAgentForUrl(const QUrl &url) const
{
    if (UserAgent.isEmpty()) {
        UserAgent = QWebPage::userAgentForUrl(url);
#ifdef Q_WS_MAC
#ifdef __i386__ || __x86_64__
        UserAgent.replace("PPC Mac OS X", "Intel Mac OS X");
#endif
#endif
    }

    return UserAgent;
}

bool WebPage::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension)

    return true;
}

bool WebPage::extension(Extension extension, const ExtensionOption* option, ExtensionReturn* output)
{
    if (extension == ChooseMultipleFilesExtension) {
        const QWebPage::ChooseMultipleFilesExtensionOption* exOption = static_cast<const QWebPage::ChooseMultipleFilesExtensionOption*>(option);
        QWebPage::ChooseMultipleFilesExtensionReturn* exReturn = static_cast<QWebPage::ChooseMultipleFilesExtensionReturn*>(output);

        if (!exOption || !exReturn) {
            return QWebPage::extension(extension, option, output);
        }

        QString suggestedFileName;
        if (!exOption->suggestedFileNames.isEmpty()) {
            suggestedFileName = exOption->suggestedFileNames.first();
        }

        exReturn->fileNames = QFileDialog::getOpenFileNames(0, tr("Select files to upload..."), suggestedFileName);
        return true;
    }

    const ErrorPageExtensionOption* exOption = static_cast<const QWebPage::ErrorPageExtensionOption*>(option);
    ErrorPageExtensionReturn* exReturn = static_cast<QWebPage::ErrorPageExtensionReturn*>(output);

    if (!exOption || !exReturn) {
        return QWebPage::extension(extension, option, output);
    }

    WebPage* erPage = qobject_cast<WebPage*>(exOption->frame->page());

    if (!erPage) {
        return QWebPage::extension(extension, option, output);
    }

    QString errorString;
    if (exOption->domain == QWebPage::QtNetwork) {
        switch (exOption->error) {
        case QNetworkReply::ConnectionRefusedError:
            errorString = tr("Server refused the connection");
            break;
        case QNetworkReply::RemoteHostClosedError:
            errorString = tr("Server closed the connection");
            break;
        case QNetworkReply::HostNotFoundError:
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
            errorString = tr("Proxy host name not found");
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
        case QNetworkReply::ContentAccessDenied:
            if (exOption->errorString.startsWith("AdBlockRule")) {
                if (exOption->frame != erPage->mainFrame()) { //Content in <iframe>
                    QWebElement docElement = erPage->mainFrame()->documentElement();

                    QWebElementCollection elements;
                    elements.append(docElement.findAll("iframe"));
                    foreach(QWebElement element, elements) {
                        QString src = element.attribute("src");
                        if (exOption->url.toString().contains(src)) {
                            element.setAttribute("style", "display:none;");
                        }
                    }
                    return false;
                }
                else {   //The whole page is blocked
                    QString rule = exOption->errorString;
                    rule.remove("AdBlockRule:");

                    QFile file(":/html/adblockPage.html");
                    file.open(QFile::ReadOnly);
                    QString errString = file.readAll();
                    errString.replace("%TITLE%", tr("AdBlocked Content"));
                    errString.replace("%IMAGE%", "qrc:html/adblock_big.png");
                    errString.replace("%FAVICON%", "qrc:html/adblock_big.png");

                    errString.replace("%RULE%", tr("Blocked by rule <i>%1</i>").arg(rule));

                    exReturn->baseUrl = exOption->url.toString();
                    exReturn->content = errString.toUtf8();
                    return true;
                }
            }
            errorString = tr("Content Access Denied");
            break;
        default:
            qDebug() << "Content error: " << exOption->errorString;
            return false;
        }
    }
    else if (exOption->domain == QWebPage::Http) {
        errorString = tr("Error code %1").arg(exOption->error);
    }
    else if (exOption->domain == QWebPage::WebKit) {
        return false;    // Downloads
    }

    QString loadedUrl = exOption->url.toString();
    exReturn->baseUrl = loadedUrl;

    QFile file(":/html/errorPage.html");
    file.open(QFile::ReadOnly);
    QString errString = file.readAll();
    errString.replace("%TITLE%", tr("Failed loading page"));

    errString.replace("%IMAGE%", qz_pixmapToByteArray(MainApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(45, 45)));
    errString.replace("%FAVICON%", qz_pixmapToByteArray(MainApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(16, 16)));
    errString.replace("%BOX-BORDER%", "qrc:html/box-border.png");

    errString.replace("%HEADING%", errorString);
    errString.replace("%HEADING2%", tr("QupZilla can't load page from %1.").arg(QUrl(loadedUrl).host()));
    errString.replace("%LI-1%", tr("Check the address for typing errors such as <b>ww.</b>example.com instead of <b>www.</b>example.com"));
    errString.replace("%LI-2%", tr("If you are unable to load any pages, check your computer's network connection."));
    errString.replace("%LI-3%", tr("If your computer or network is protected by a firewall or proxy, make sure that QupZilla is permitted to access the Web."));
    errString.replace("%TRY-AGAIN%", tr("Try Again"));

    exReturn->content = errString.toUtf8();
    return true;
}

bool WebPage::javaScriptPrompt(QWebFrame* originatingFrame, const QString &msg, const QString &defaultValue, QString* result)
{
#ifndef NONBLOCK_JS_DIALOGS
    return QWebPage::javaScriptPrompt(originatingFrame, msg, defaultValue, result);
#else
    if (m_runningLoop) {
        return false;
    }

    WebView* webView = qobject_cast<WebView*>(originatingFrame->page()->view());
    ResizableFrame* widget = new ResizableFrame(webView->overlayForJsAlert());

    widget->setObjectName("jsFrame");
    Ui_jsPrompt* ui = new Ui_jsPrompt();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->lineEdit->setText(defaultValue);
    ui->lineEdit->setFocus();
    widget->resize(originatingFrame->page()->viewportSize());
    widget->show();

    connect(webView, SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));
    connect(ui->lineEdit, SIGNAL(returnPressed()), ui->buttonBox->button(QDialogButtonBox::Ok), SLOT(animateClick()));

    QEventLoop eLoop;
    m_runningLoop = &eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));

    if (eLoop.exec() == 1 || m_isClosing) {
        return result;
    }
    m_runningLoop = 0;

    QString x = ui->lineEdit->text();
    bool _result = ui->buttonBox->clickedButtonRole() == QDialogButtonBox::AcceptRole;
    *result = x;

    delete widget;
    webView->setFocus();

    return _result;
#endif
}

bool WebPage::javaScriptConfirm(QWebFrame* originatingFrame, const QString &msg)
{
#ifndef NONBLOCK_JS_DIALOGS
    return QWebPage::javaScriptConfirm(originatingFrame, msg);
#else
    if (m_runningLoop) {
        return false;
    }

    WebView* webView = qobject_cast<WebView*>(originatingFrame->page()->view());
    ResizableFrame* widget = new ResizableFrame(webView->overlayForJsAlert());

    widget->setObjectName("jsFrame");
    Ui_jsConfirm* ui = new Ui_jsConfirm();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();
    widget->resize(originatingFrame->page()->viewportSize());
    widget->show();

    connect(webView, SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));

    QEventLoop eLoop;
    m_runningLoop = &eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));

    if (eLoop.exec() == 1 || m_isClosing) {
        return false;
    }
    m_runningLoop = 0;

    bool result = ui->buttonBox->clickedButtonRole() == QDialogButtonBox::AcceptRole;

    delete widget;
    webView->setFocus();

    return result;
#endif
}

void WebPage::javaScriptAlert(QWebFrame* originatingFrame, const QString &msg)
{
    if (m_blockAlerts || m_runningLoop) {
        return;
    }

#ifndef NONBLOCK_JS_DIALOGS
    QDialog dialog(view());
    Ui_CloseDialog* ui = new Ui_CloseDialog();
    ui->setupUi(&dialog);
    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    ui->dontAskAgain->setText(tr("Prevent this page from creating additional dialogs"));
    ui->textLabel->setText(Qt::escape(msg));
    ui->iconLabel->setPixmap(mApp->style()->standardPixmap(QStyle::SP_MessageBoxInformation));
    dialog.setWindowTitle(tr("JavaScript alert - %1").arg(originatingFrame->url().host()));
    dialog.exec();

    if (ui->dontAskAgain->isChecked()) {
        m_blockAlerts = true;
    }
#else
    WebView* webView = qobject_cast<WebView*>(originatingFrame->page()->view());
    ResizableFrame* widget = new ResizableFrame(webView->overlayForJsAlert());

    widget->setObjectName("jsFrame");
    Ui_jsAlert* ui = new Ui_jsAlert();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();
    widget->resize(originatingFrame->page()->viewportSize());
    widget->show();

    connect(webView, SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));

    QEventLoop eLoop;
    m_runningLoop = &eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));

    if (eLoop.exec() == 1 || m_isClosing) {
        return;
    }
    m_runningLoop = 0;

    m_blockAlerts = ui->preventAlerts->isChecked();

    delete widget;

    webView->setFocus();
#endif
}

QString WebPage::chooseFile(QWebFrame* originatingFrame, const QString &oldFile)
{
    QString suggFileName;
    if (oldFile.isEmpty()) {
        suggFileName = m_lastUploadLocation;
    }
    else {
        suggFileName = oldFile;
    }
    QString fileName = QFileDialog::getOpenFileName(originatingFrame->page()->view(), tr("Choose file..."), suggFileName);

    if (!fileName.isEmpty()) {
        m_lastUploadLocation = fileName;
    }

    return fileName;
}

void WebPage::disconnectObjects()
{
    if (m_runningLoop) {
        m_runningLoop->exit(1);
        m_runningLoop = 0;
    }

    disconnect(this);
    m_networkProxy->disconnectObjects();
}

WebPage::~WebPage()
{
    if (m_runningLoop) {
        m_runningLoop->exit(1);
        m_runningLoop = 0;
    }
}
