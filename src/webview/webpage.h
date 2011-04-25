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
#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QDebug>
#include <QUrl>
#include <QWebView>
#include <QWebFrame>
#include <QWebHistory>
#include <QtNetwork/QtNetwork>
#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDesktopServices>
#include <QStyle>

class QupZilla;
class WebView;
class WebPage : public QWebPage
{
    Q_OBJECT
public:
    struct AdBlockedEntry {
        QString rule;
        QUrl url;
    };

    WebPage(WebView* parent, QupZilla* mainClass);
    void populateNetworkRequest(QNetworkRequest &request);
    ~WebPage();

    void setSSLCertificate(const QSslCertificate &cert);
    QSslCertificate sslCertificate();
    QString userAgentForUrl(const QUrl &url) const;
    virtual bool supportsExtension(Extension extension) const { return (extension == ErrorPageExtension); }
    virtual bool extension(Extension extension, const ExtensionOption* option, ExtensionReturn* output = 0);

    void addAdBlockRule(const QString &filter, const QUrl &url);
    QList<AdBlockedEntry> adBlockedEntries() { return m_adBlockedEntries; }

protected slots:
    QWebPage* createWindow(QWebPage::WebWindowType type);
    void handleUnsupportedContent(QNetworkReply* url);
    void loadingStarted() { m_adBlockedEntries.clear(); /*m_SslCert.clear();*/ }

protected:
    bool acceptNavigationRequest(QWebFrame* frame, const QNetworkRequest &request, NavigationType type);

    QupZilla* p_QupZilla;
    QNetworkRequest m_lastRequest;
    QWebPage::NavigationType m_lastRequestType;
    WebView* m_view;
    QSslCertificate m_SslCert;
    QList<QSslCertificate> m_SslCerts;
    QList<AdBlockedEntry> m_adBlockedEntries;
//    bool m_isOpeningNextWindowAsNewTab;
};

#endif // WEBPAGE_H
