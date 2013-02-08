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
#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebPage>
#include <QSslCertificate>

#include "qz_namespace.h"
#include "autofill.h"

class QWebSecurityOrigin;
class QFileSystemWatcher;
class QEventLoop;

class QupZilla;
class AdBlockRule;
class TabbedWebView;
class SpeedDial;
class NetworkManagerProxy;

class QT_QUPZILLA_EXPORT WebPage : public QWebPage
{
    Q_OBJECT
public:
    struct AdBlockedEntry {
        const AdBlockRule* rule;
        QUrl url;

        bool operator==(const AdBlockedEntry &other) const {
            return (this->rule == other.rule && this->url == other.url);
        }
    };

    WebPage(QupZilla* mainClass);
    ~WebPage();

    QUrl url() const;

    void setWebView(TabbedWebView* view);
    void populateNetworkRequest(QNetworkRequest &request);

    void setSSLCertificate(const QSslCertificate &cert);
    QSslCertificate sslCertificate();

    bool javaScriptPrompt(QWebFrame* originatingFrame, const QString &msg, const QString &defaultValue, QString* result);
    bool javaScriptConfirm(QWebFrame* originatingFrame, const QString &msg);
    void javaScriptAlert(QWebFrame* originatingFrame, const QString &msg);

    void addAdBlockRule(const AdBlockRule* rule, const QUrl &url);
    QList<AdBlockedEntry> adBlockedEntries() const;

    bool hasMultipleUsernames() const;
    QList<AutoFillData> autoFillData() const;

    void scheduleAdjustPage();
    bool isRunningLoop();

    bool isLoading() const;
    bool loadingError() const;

    void addRejectedCerts(const QList<QSslCertificate> &certs);
    bool containsRejectedCerts(const QList<QSslCertificate> &certs);

    QString userAgentForUrl(const QUrl &url) const;

    static bool isPointerSafeToUse(WebPage* page);
    void disconnectObjects();

signals:
    void privacyChanged(bool status);

protected slots:
    void handleUnsupportedContent(QNetworkReply* url);

    void progress(int prog);
    void finished();

private slots:
    void cleanBlockedObjects();
    void urlChanged(const QUrl &url);
    void addJavaScriptObject();

    void watchedFileChanged(const QString &file);
    void printFrame(QWebFrame* frame);
    void downloadRequested(const QNetworkRequest &request);
    void windowCloseRequested();

    void dbQuotaExceeded(QWebFrame* frame);

#ifdef USE_QTWEBKIT_2_2
    void appCacheQuotaExceeded(QWebSecurityOrigin* origin, quint64 originalQuota);
    void featurePermissionRequested(QWebFrame* frame, const QWebPage::Feature &feature);
#endif

protected:
    bool event(QEvent* event);
    QWebPage* createWindow(QWebPage::WebWindowType type);
    QObject* createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);

private:
    bool supportsExtension(Extension extension) const;
    bool extension(Extension extension, const ExtensionOption* option, ExtensionReturn* output = 0);
    bool acceptNavigationRequest(QWebFrame* frame, const QNetworkRequest &request, NavigationType type);
    QString chooseFile(QWebFrame* originatingFrame, const QString &oldFile);

    void handleUnknownProtocol(const QUrl &url);
    void desktopServicesOpen(const QUrl &url);

    static QString s_lastUploadLocation;
    static QUrl s_lastUnsupportedUrl;
    static QTime s_lastUnsupportedUrlTime;
    static QList<WebPage*> s_livingPages;

    QupZilla* p_QupZilla;
    NetworkManagerProxy* m_networkProxy;
    TabbedWebView* m_view;
    SpeedDial* m_speedDial;
    QFileSystemWatcher* m_fileWatcher;
    QEventLoop* m_runningLoop;

    QSslCertificate m_sslCert;
    QList<QSslCertificate> m_rejectedSslCerts;
    QList<AdBlockedEntry> m_adBlockedEntries;
    QList<AutoFillData> m_autoFillData;

    QWebPage::NavigationType m_lastRequestType;
    QUrl m_lastRequestUrl;

    int m_loadProgress;
    bool m_blockAlerts;
    bool m_secureStatus;
    bool m_adjustingScheduled;
};

#endif // WEBPAGE_H
