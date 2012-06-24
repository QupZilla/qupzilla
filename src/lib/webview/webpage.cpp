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
#include "checkboxdialog.h"
#include "widget.h"
#include "globalfunctions.h"
#include "pluginproxy.h"
#include "speeddial.h"
#include "popupwebpage.h"
#include "popupwebview.h"
#include "networkmanagerproxy.h"
#include "adblockicon.h"
#include "iconprovider.h"
#include "websettings.h"

#ifdef NONBLOCK_JS_DIALOGS
#include "ui_jsconfirm.h"
#include "ui_jsalert.h"
#include "ui_jsprompt.h"

#include <QPushButton>
#endif

#include <QDir>
#include <QWebHistory>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QNetworkReply>
#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFileDialog>
#include <QWebFrame>

QString WebPage::s_lastUploadLocation = QDir::homePath();
QString WebPage::s_userAgent;
QString WebPage::s_fakeUserAgent;
QUrl WebPage::s_lastUnsupportedUrl;
QList<WebPage*> WebPage::s_livingPages;

WebPage::WebPage(QupZilla* mainClass)
    : QWebPage()
    , p_QupZilla(mainClass)
    , m_view(0)
    , m_speedDial(mApp->plugins()->speedDial())
    , m_fileWatcher(0)
    , m_runningLoop(0)
    , m_blockAlerts(false)
    , m_secureStatus(false)
    , m_adjustingScheduled(false)
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
    connect(this, SIGNAL(windowCloseRequested()), this, SLOT(windowCloseRequested()));

    connect(mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()));

#ifdef USE_QTWEBKIT_2_2
    connect(this, SIGNAL(featurePermissionRequested(QWebFrame*, QWebPage::Feature)), this, SLOT(featurePermissionRequested(QWebFrame*, QWebPage::Feature)));
#endif

    s_livingPages.append(this);
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
    WebView* webView = qobject_cast<WebView*>(view());
    if (!webView) {
        return;
    }

    if (webView->isLoading()) {
        m_adjustingScheduled = true;
    }
    else {
        const QSize &originalSize = webView->size();
        QSize newSize(originalSize.width() - 1, originalSize.height() - 1);

        webView->resize(newSize);
        webView->resize(originalSize);
    }
}

bool WebPage::loadingError() const
{
    return !mainFrame()->findFirstElement("span[id=\"qupzilla-error-page\"]").isNull();
}

bool WebPage::isRunningLoop()
{
    return m_runningLoop;
}

void WebPage::setUserAgent(const QString &agent)
{
    if (!agent.isEmpty()) {
        s_userAgent = QString("%1 (QupZilla %2)").arg(agent, QupZilla::VERSION);
    }
    else {
        s_userAgent = agent;
    }
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

        const QString &filePath = url().toLocalFile();

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

    const QUrl &url = reply->url();

    switch (reply->error()) {
    case QNetworkReply::NoError:
        if (reply->header(QNetworkRequest::ContentTypeHeader).isValid()) {
            QString requestUrl = reply->request().url().toString(QUrl::RemoveFragment | QUrl::RemoveQuery);
            if (requestUrl.endsWith(".swf")) {
                const QWebElement &docElement = mainFrame()->documentElement();
                const QWebElement &object = docElement.findFirst(QString("object[src=\"%1\"]").arg(requestUrl));
                const QWebElement &embed = docElement.findFirst(QString("embed[src=\"%1\"]").arg(requestUrl));

                if (!object.isNull() || !embed.isNull()) {
                    qDebug() << "WebPage::UnsupportedContent" << url << "Attempt to download flash object on site!";
                    reply->deleteLater();
                    return;
                }
            }
            DownloadManager* dManager = mApp->downManager();
            dManager->handleUnsupportedContent(reply, this);
            return;
        }

    case QNetworkReply::ProtocolUnknownError: {
        qDebug() << "WebPage::UnsupportedContent" << url << "ProtocolUnknowError";

        // We will open last unsupported url only once
        // (to prevent endless loop in case QDesktopServices::openUrl decide
        // to open the url again in QupZilla )

        if (s_lastUnsupportedUrl != url) {
            s_lastUnsupportedUrl = url;
            QDesktopServices::openUrl(url);
        }

        reply->deleteLater();
        return;
    }
    default:
        break;
    }

    qDebug() << "WebPage::UnsupportedContent error" << url << reply->errorString();
    reply->deleteLater();
}

void WebPage::handleUnknownProtocol(const QUrl &url)
{
    const QString &protocol = url.scheme();

    if (WebSettings::blockedProtocols.contains(protocol)) {
        qDebug() << "WebPage::handleUnknownProtocol Protocol" << protocol << "is blocked!";
        return;
    }

    if (WebSettings::autoOpenProtocols.contains(protocol)) {
        QDesktopServices::openUrl(url);
        return;
    }

    CheckBoxDialog dialog(QDialogButtonBox::Yes | QDialogButtonBox::No, view());

    const QString &wrappedUrl = qz_alignTextToWidth(url.toString(), "<br/>", dialog.fontMetrics(), 450);
    const QString &text = tr("QupZilla cannot handle <b>%1:</b> links. The requested link "
                             "is <ul><li>%2</li></ul>Do you want QupZilla to try "
                             "open this link in system application?").arg(protocol, wrappedUrl);

    dialog.setText(text);
    dialog.setCheckBoxText(tr("Remember my choice for this protocol"));
    dialog.setWindowTitle(tr("External Protocol Request"));
    dialog.setIcon(qIconProvider->standardIcon(QStyle::SP_MessageBoxQuestion));

    switch (dialog.exec()) {
    case QDialog::Accepted:
        if (dialog.isChecked()) {
            WebSettings::autoOpenProtocols.append(protocol);
            WebSettings::saveSettings();
        }

        QDesktopServices::openUrl(url);
        break;

    case QDialog::Rejected:
        if (dialog.isChecked()) {
            WebSettings::blockedProtocols.append(protocol);
            WebSettings::saveSettings();
        }

        break;

    default:
        break;
    }
}

void WebPage::downloadRequested(const QNetworkRequest &request)
{
    DownloadManager* dManager = mApp->downManager();
    dManager->download(request, this);
}

void WebPage::windowCloseRequested()
{
    WebView* webView = qobject_cast<WebView*>(view());
    if (!webView) {
        return;
    }

    webView->closeView();
}

#ifdef USE_QTWEBKIT_2_2
void WebPage::featurePermissionRequested(QWebFrame* frame, const QWebPage::Feature &feature)
{
    // We should probably ask user here ... -,-
    setFeaturePermission(frame, feature, PermissionGrantedByUser);
}
#endif

bool WebPage::event(QEvent* event)
{
    if (event->type() == QEvent::Leave) {
        // QWebPagePrivate::leaveEvent():
        // Fake a mouse move event just outside of the widget, since all
        // the interesting mouse-out behavior like invalidating scrollbars
        // is handled by the WebKit event handler's mouseMoved function.

        // However, its implementation fake mouse move event on QCursor::pos()
        // position that is in global screen coordinates. So instead of
        // really faking it, it just creates mouse move event somewhere in
        // page. It can for example focus a link, and then link url gets
        // stuck in status bar message.

        // So we are faking mouse move event with proper coordinates for
        // so called "just outside of the widget" position

        const QPoint cursorPos = view()->mapFromGlobal(QCursor::pos());
        QPoint mousePos;

        if (cursorPos.y() < 0) {
            // Left on top
            mousePos = QPoint(cursorPos.x(), -1);
        }
        else if (cursorPos.x() < 0) {
            // Left on left
            mousePos = QPoint(-1, cursorPos.y());
        }
        else if (cursorPos.y() > view()->height()) {
            // Left on bottom
            mousePos = QPoint(cursorPos.x(), view()->height() + 1);
        }
        else {
            // Left on right
            mousePos = QPoint(view()->width() + 1, cursorPos.y());
        }

        QMouseEvent fakeEvent(QEvent::MouseMove, mousePos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        return QWebPage::event(&fakeEvent);
    }

    return QWebPage::event(event);
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
    m_lastRequestType = type;
    const QString &scheme = request.url().scheme();

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
    return new PopupWebPage(type, p_QupZilla);
}

QObject* WebPage::createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
    qDebug() << Q_FUNC_INFO;
    return pluginFactory()->create(classid, url, paramNames, paramValues);
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

    foreach(const AdBlockedEntry & entry, m_adBlockedEntries) {
        if (entry.url.toString().endsWith(".js")) {
            continue;
        }

        findingStrings.append(entry.url.toString());
        const QUrl &mainFrameUrl = url();

        if (entry.url.scheme() == mainFrameUrl.scheme() && entry.url.host() == mainFrameUrl.host()) {
            //May be relative url
            QString relativeUrl = qz_makeRelativeUrl(mainFrameUrl, entry.url).toString();
            findingStrings.append(relativeUrl);
            if (relativeUrl.startsWith("/")) {
                findingStrings.append(relativeUrl.right(relativeUrl.size() - 1));
            }
        }
    }

    const QWebElement &docElement = mainFrame()->documentElement();
    QWebElementCollection elements;

    foreach(const QString & s, findingStrings) {
        elements.append(docElement.findAll("*[src=\"" + s + "\"]"));
    }

    foreach(QWebElement element, elements) {
        element.setStyleProperty("visibility", "hidden");
    }
}

QString WebPage::userAgentForUrl(const QUrl &url) const
{
    const QString &host = url.host();

    // Let Google services play nice with us
    if (host.contains("google")) {
        if (s_fakeUserAgent.isEmpty()) {
            s_fakeUserAgent = QString("Mozilla/5.0 (%1) AppleWebKit/%2 (KHTML, like Gecko) Chrome/10.0 Safari/%2").arg(qz_buildSystem(), QupZilla::WEBKITVERSION);
        }

        return s_fakeUserAgent;
    }

    if (s_userAgent.isEmpty()) {
        s_userAgent = QWebPage::userAgentForUrl(url);
#ifdef Q_WS_MAC
#ifdef __i386__ || __x86_64__
        m_userAgent.replace("PPC Mac OS X", "Intel Mac OS X");
#endif
#endif
    }

    return s_userAgent;
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
            suggestedFileName = exOption->suggestedFileNames.at(0);
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
        case QNetworkReply::ProtocolUnknownError:
            handleUnknownProtocol(exOption->url);
            return false;
            break;
        case QNetworkReply::ContentAccessDenied:
            if (exOption->errorString.startsWith("AdBlock")) {
                if (exOption->frame != erPage->mainFrame()) { //Content in <iframe>
                    QWebElement docElement = erPage->mainFrame()->documentElement();

                    QWebElementCollection elements;
                    elements.append(docElement.findAll("iframe"));

                    foreach(QWebElement element, elements) {
                        QString src = element.attribute("src");
                        if (exOption->url.toString().contains(src)) {
                            element.setStyleProperty("visibility", "hidden");
                        }
                    }

                    return false;
                }
                else {   //The whole page is blocked
                    QString rule = exOption->errorString;
                    rule.remove("AdBlock:");

                    QFile file(":/html/adblockPage.html");
                    file.open(QFile::ReadOnly);
                    QString errString = file.readAll();
                    errString.replace("%TITLE%", tr("AdBlocked Content"));
                    errString.replace("%IMAGE%", "qrc:html/adblock_big.png");
                    errString.replace("%FAVICON%", "qrc:html/adblock_big.png");

                    errString.replace("%RULE%", tr("Blocked by <i>%1</i>").arg(rule));

                    exReturn->baseUrl = exOption->url;
                    exReturn->content = QString(errString + "<span id=\"qupzilla-error-page\"></span>").toUtf8();

                    if (PopupWebPage* popupPage = qobject_cast<PopupWebPage*>(exOption->frame->page())) {
                        WebView* view = qobject_cast<WebView*>(popupPage->view());
                        if (view) {
                            // Closing blocked popup
                            p_QupZilla->adBlockIcon()->popupBlocked(rule, exOption->url);
                            view->closeView();
                        }
                    }
                    return true;
                }
            }
            errorString = tr("Content Access Denied");
            break;
        default:
            if (exOption->errorString != "QupZilla:No Error") {
                qDebug() << "Content error: " << exOption->errorString << exOption->error;
            }
            return false;
        }
    }
    else if (exOption->domain == QWebPage::Http) {
        // 200 status code = OK
        // It shouldn't be reported as an error, but sometimes it is ...
        if (exOption->error == 200) {
            return false;
        }
        errorString = tr("Error code %1").arg(exOption->error);
    }
    else if (exOption->domain == QWebPage::WebKit) {
        return false;    // Downloads
    }

    const QUrl &loadedUrl = exOption->url;
    exReturn->baseUrl = loadedUrl;

    QFile file(":/html/errorPage.html");
    file.open(QFile::ReadOnly);
    QString errString = file.readAll();
    errString.replace("%TITLE%", tr("Failed loading page"));

    errString.replace("%IMAGE%", qz_pixmapToByteArray(qIconProvider->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(45, 45)));
    errString.replace("%FAVICON%", qz_pixmapToByteArray(qIconProvider->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(16, 16)));
    errString.replace("%BOX-BORDER%", "qrc:html/box-border.png");

    errString.replace("%HEADING%", errorString);
    errString.replace("%HEADING2%", tr("QupZilla can't load page from %1.").arg(loadedUrl.host()));
    errString.replace("%LI-1%", tr("Check the address for typing errors such as <b>ww.</b>example.com instead of <b>www.</b>example.com"));
    errString.replace("%LI-2%", tr("If you are unable to load any pages, check your computer's network connection."));
    errString.replace("%LI-3%", tr("If your computer or network is protected by a firewall or proxy, make sure that QupZilla is permitted to access the Web."));
    errString.replace("%TRY-AGAIN%", tr("Try Again"));

    exReturn->content = QString(errString + "<span id=\"qupzilla-error-page\"></span>").toUtf8();
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

    if (eLoop.exec() == 1) {
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

    if (eLoop.exec() == 1) {
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
    Q_UNUSED(originatingFrame)

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
    dialog.setIcon(qIconProvider->standardIcon(QStyle::SP_MessageBoxInformation));
    dialog.exec();

    m_blockAlerts = dialog.isChecked();

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

    if (eLoop.exec() == 1) {
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
        suggFileName = s_lastUploadLocation;
    }
    else {
        suggFileName = oldFile;
    }

    const QString &fileName = QFileDialog::getOpenFileName(originatingFrame->page()->view(), tr("Choose file..."), suggFileName);

    if (!fileName.isEmpty()) {
        s_lastUploadLocation = fileName;
    }

    return fileName;
}

bool WebPage::isPointerSafeToUse(WebPage* page)
{
    // Pointer to WebPage is passed with every QNetworkRequest casted to void*
    // So there is no way to test whether pointer is still valid or not, except
    // this hack.

    return page == 0 ? false : s_livingPages.contains(page);
}

void WebPage::disconnectObjects()
{
    if (m_runningLoop) {
        m_runningLoop->exit(1);
        m_runningLoop = 0;
    }

    s_livingPages.removeOne(this);

    disconnect(this);
    m_networkProxy->disconnectObjects();

    mApp->plugins()->emitWebPageDeleted(this);
}

WebPage::~WebPage()
{
    if (m_runningLoop) {
        m_runningLoop->exit(1);
        m_runningLoop = 0;
    }

    s_livingPages.removeOne(this);
}
