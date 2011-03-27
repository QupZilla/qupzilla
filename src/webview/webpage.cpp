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
#include "webpage.h"
#include "webview.h"
#include "tabwidget.h"
#include "qupzilla.h"
#include "downloadmanager.h"
#include "webpluginfactory.h"
#include "mainapplication.h"

WebPage::WebPage(WebView* parent, QupZilla* mainClass)
    : QWebPage(parent)
    ,p_QupZilla(mainClass)
    ,m_view(parent)
//    ,m_isOpeningNextWindowAsNewTab(false)
{
    setForwardUnsupportedContent(true);
    setPluginFactory(new WebPluginFactory(this));
    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), SLOT(handleUnsupportedContent(QNetworkReply*)));
    connect(this, SIGNAL(loadStarted()), this, SLOT(clearSSLCert()));
}

void WebPage::handleUnsupportedContent(QNetworkReply* reply)
{
    if (!reply)
        return;
    QUrl url = reply->url();

    switch(reply->error()) {
    case QNetworkReply::NoError:
        if (reply->header(QNetworkRequest::ContentTypeHeader).isValid()) {
            DownloadManager* dManager = mApp->downManager();
            dManager->handleUnsupportedContent(reply);
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

void WebPage::setSSLCertificate(QSslCertificate cert)
{
//    if (cert != m_SslCert)
        m_SslCert = cert;
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
        QMessageBox::StandardButton button = QMessageBox::warning(view(), tr("Confirmation"),
                             tr("To show this page, QupZilla must resend request witch do it again "
                             "(like searching on making an shoping, witch has been already done."), QMessageBox::Yes | QMessageBox::No);
        if (button != QMessageBox::Yes)
            return false;
    }

    TabWidget::OpenUrlIn openIn= frame ? TabWidget::CurrentTab: TabWidget::NewTab;

    bool accept = QWebPage::acceptNavigationRequest(frame, request, type);
    if (accept && openIn == TabWidget::NewTab) {
//        m_isOpeningNextWindowAsNewTab = true;
//        p_QupZilla->tabWidget()->addView(request.url(),tr("New tab"), openIn);
    }
    return accept;
}

QString WebPage::userAgentForUrl(const QUrl &url) const
{
    return QWebPage::userAgentForUrl(url);
}

void WebPage::populateNetworkRequest(QNetworkRequest &request)
{
    QVariant variant = qVariantFromValue((void *) this);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100), variant);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 101), m_lastRequestType);

    variant = qVariantFromValue((void *) m_view);
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

bool WebPage::extension(Extension extension, const ExtensionOption* option, ExtensionReturn* output)
{
    if (extension == ChooseMultipleFilesExtension)
        return QWebPage::extension(extension, option, output);

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
        default:
            //errorString = exOption->error;
            if (errorString.isEmpty())
                errorString = tr("Unknown error");
            break;
        }
    }
    else if (exOption->domain == QWebPage::Http) {
        errorString = tr("Error code %1").arg(exOption->error);
    }
    else if (exOption->domain == QWebPage::WebKit)
        return false; // Downloads

    QString loadedUrl = exOption->url.toString();
    exReturn->baseUrl = loadedUrl;

    QFile file(":/html/errorPage.html");
    file.open(QFile::ReadOnly);
    QString errString = file.readAll();
    errString.replace("%TITLE%", tr("Failed loading page"));

    //QPixmap pixmap = QIcon::fromTheme("dialog-warning").pixmap(45,45);
    QPixmap pixmap = MainApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(45,45);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    if (pixmap.save(&buffer, "PNG"))
        errString.replace("%IMAGE%", buffer.buffer().toBase64());

    //pixmap = QIcon::fromTheme("dialog-warning").pixmap(16,16);
    pixmap = MainApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(16,16);
    bytes.clear();
    QBuffer buffer2(&bytes);
    buffer2.open(QIODevice::WriteOnly);
    if (pixmap.save(&buffer2, "PNG"))
        errString.replace("%FAVICON%", buffer.buffer().toBase64());

    errString.replace("%HEADING%", errorString);
    errString.replace("%HEADING2%", tr("QupZilla can't load page from %1.").arg(QUrl(loadedUrl).host()));
    errString.replace("%LI-1%", tr("Check the address for typing errors such as <b>ww.</b>example.com instead of <b>www.</b>example.com"));
    errString.replace("%LI-2%", tr("If you are unable to load any pages, check your computer's network connection."));
    errString.replace("%LI-3%", tr("If your computer or network is protected by a firewall or proxy, make sure that QupZilla is permitted to access the Web."));
    errString.replace("%TRY-AGAIN%", tr("Try Again"));

    exReturn->content = errString.toUtf8();
    return true;
}

WebPage::~WebPage()
{
    setNetworkAccessManager(0);
    mainFrame()->deleteLater();
}
