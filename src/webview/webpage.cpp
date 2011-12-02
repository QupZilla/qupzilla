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
#include "webpage.h"
#include "webview.h"
#include "tabwidget.h"
#include "qupzilla.h"
#include "downloadmanager.h"
#include "webpluginfactory.h"
#include "mainapplication.h"
#include "ui_jsconfirm.h"
#include "ui_jsalert.h"
#include "ui_jsprompt.h"
#include "widget.h"
#include "globalfunctions.h"
#include "speeddial.h"
#include "pluginproxy.h"

QString WebPage::m_lastUploadLocation = QDir::homePath();

WebPage::WebPage(WebView* parent, QupZilla* mainClass)
    : QWebPage(parent)
    , p_QupZilla(mainClass)
    , m_view(parent)
    , m_speedDial(mApp->plugins()->speedDial())
    , m_blockAlerts(false)
    , m_secureStatus(false)
//    , m_isOpeningNextWindowAsNewTab(false)
{
    setForwardUnsupportedContent(true);
    setPluginFactory(new WebPluginFactory(this));
    history()->setMaximumItemCount(20);

    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), SLOT(handleUnsupportedContent(QNetworkReply*)));
//    connect(this, SIGNAL(loadStarted()), this, SLOT(loadingStarted()));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(progress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(finished()));
    connect(m_view, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged(QUrl)));

    connect(mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()));
}

void WebPage::scheduleAdjustPage()
{
    if (m_view->isLoading()) {
        m_adjustingScheduled = true;
    }
    else {
        mainFrame()->setZoomFactor(mainFrame()->zoomFactor() + 1);
        mainFrame()->setZoomFactor(mainFrame()->zoomFactor() - 1);
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

    QTimer::singleShot(100, this, SLOT(cleanBlockedObjects()));
}

//void WebPage::loadingStarted()
//{
//    m_adBlockedEntries.clear();
//    m_blockAlerts = false;
//    m_SslCert.clear();
//}

void WebPage::addJavaScriptObject()
{
    if (mainFrame()->url().toString() != "qupzilla:speeddial") {
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

void WebPage::setSSLCertificate(const QSslCertificate &cert)
{
//    if (cert != m_SslCert) -- crashing on linux :-|
    m_SslCert = cert;
}

QSslCertificate WebPage::sslCertificate()
{
    if (mainFrame()->url().scheme() == "https" &&
            m_SslCert.subjectInfo(QSslCertificate::CommonName).remove("*").contains(QRegExp(mainFrame()->url().host()))) {
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
        bool result = javaScriptConfirm(frame, tr("To show this page, QupZilla must resend request which do it again \n"
                                        "(like searching on making an shoping, which has been already done.)"));
        if (!result) {
            return false;
        }
    }

//    TabWidget::OpenUrlIn openIn= frame ? TabWidget::CurrentTab: TabWidget::NewTab;

    bool accept = QWebPage::acceptNavigationRequest(frame, request, type);
//    if (accept && openIn == TabWidget::NewTab) {
//        m_isOpeningNextWindowAsNewTab = true;
//        p_QupZilla->tabWidget()->addView(request.url(),tr("New tab"), openIn);
//    }
    return accept;
}

QString WebPage::userAgentForUrl(const QUrl &url) const
{
    return QWebPage::userAgentForUrl(url);
}

void WebPage::populateNetworkRequest(QNetworkRequest &request)
{
    QPointer<WebPage> pagePointer = this;
    QPointer<WebView> webViewPointer = m_view;

    QVariant variant = qVariantFromValue((void*) pagePointer);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100), variant);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 101), m_lastRequestType);

    variant = qVariantFromValue((void*) webViewPointer);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 102), variant);
}

QWebPage* WebPage::createWindow(QWebPage::WebWindowType type)
{
//    if (m_isOpeningNextWindowAsNewTab)
//        return 0;
//    m_isOpeningNextWindowAsNewTab = false;
//    qDebug() << type;
//    QWebView* view = new QWebView();
//    view->show();
//    return view->page();
    Q_UNUSED(type);
    int index = p_QupZilla->tabWidget()->addView();
    return p_QupZilla->weView(index)->page();
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
        QUrl mainFrameUrl = mainFrame()->url();
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

bool WebPage::extension(Extension extension, const ExtensionOption* option, ExtensionReturn* output)
{
    if (extension == ChooseMultipleFilesExtension) {
        return QWebPage::extension(extension, option, output);
    }

    const ErrorPageExtensionOption* exOption = static_cast<const QWebPage::ErrorPageExtensionOption*>(option);
    ErrorPageExtensionReturn* exReturn = static_cast<QWebPage::ErrorPageExtensionReturn*>(output);

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
        case QNetworkReply::ContentAccessDenied:
            if (exOption->errorString.startsWith("AdBlockRule")) {
                if (exOption->frame != exOption->frame->page()->mainFrame()) { //Content in <iframe>
                    QWebElement docElement = exOption->frame->page()->mainFrame()->documentElement();

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

                    QPixmap pixmap(":/html/adblock_big.png");
                    QByteArray bytes;
                    QBuffer buffer(&bytes);
                    buffer.open(QIODevice::WriteOnly);
                    if (pixmap.save(&buffer, "PNG")) {
                        errString.replace("%IMAGE%", buffer.buffer().toBase64());
                        errString.replace("%FAVICON%", buffer.buffer().toBase64());
                    }

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
//            if (errorString.isEmpty())
//                errorString = tr("Unknown error");
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
    WebView* _view = qobject_cast<WebView*>(originatingFrame->page()->view());

    ResizableFrame* widget = new ResizableFrame(_view->webTab());
    widget->setObjectName("jsFrame");
    Ui_jsPrompt* ui = new Ui_jsPrompt();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->lineEdit->setText(defaultValue);
    ui->lineEdit->setFocus();
    widget->resize(originatingFrame->page()->viewportSize());
    widget->show();

    connect(_view, SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));
    connect(ui->lineEdit, SIGNAL(returnPressed()), ui->buttonBox->button(QDialogButtonBox::Ok), SLOT(animateClick()));

    QEventLoop eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));
    eLoop.exec();

    QString x = ui->lineEdit->text();
    bool _result = ui->buttonBox->clickedButtonRole() == QDialogButtonBox::AcceptRole;
    *result = x;
    delete widget;

    _view->setFocus();

    return _result;
}

bool WebPage::javaScriptConfirm(QWebFrame* originatingFrame, const QString &msg)
{
    WebView* _view = qobject_cast<WebView*>(originatingFrame->page()->view());

    ResizableFrame* widget = new ResizableFrame(_view->webTab());
    widget->setObjectName("jsFrame");
    Ui_jsConfirm* ui = new Ui_jsConfirm();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();
    widget->resize(originatingFrame->page()->viewportSize());
    widget->show();

    connect(_view, SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));

    QEventLoop eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));
    eLoop.exec();

    bool result = ui->buttonBox->clickedButtonRole() == QDialogButtonBox::AcceptRole;
    delete widget;

    _view->setFocus();

    return result;
}

void WebPage::javaScriptAlert(QWebFrame* originatingFrame, const QString &msg)
{
    if (m_blockAlerts) {
        return;
    }

    WebView* _view = qobject_cast<WebView*>(originatingFrame->page()->view());

    ResizableFrame* widget = new ResizableFrame(_view->webTab());
    widget->setObjectName("jsFrame");
    Ui_jsAlert* ui = new Ui_jsAlert();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();
    widget->resize(originatingFrame->page()->viewportSize());
    widget->show();

    connect(_view, SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));

    QEventLoop eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));
    eLoop.exec();

    m_blockAlerts = ui->preventAlerts->isChecked();

    delete widget;

    _view->setFocus();
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

WebPage::~WebPage()
{
    setNetworkAccessManager(0);
    mainFrame()->deleteLater();
}
