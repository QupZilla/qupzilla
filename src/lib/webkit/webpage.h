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
#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>
#include <QSslCertificate>
#include <QVector>

#include "qzcommon.h"
#include "passwordmanager.h"

class QEventLoop;
class QWebEngineDownloadItem;

class BrowserWindow;
class AdBlockRule;
class TabbedWebView;
class SpeedDial;
class NetworkManagerProxy;
class DelayedFileWatcher;

class QUPZILLA_EXPORT WebPage : public QWebEnginePage
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

    WebPage(QObject* parent = 0);
    ~WebPage();

    void setWebView(TabbedWebView* view);
    void populateNetworkRequest(QNetworkRequest &request);

    void setSSLCertificate(const QSslCertificate &cert);
    QSslCertificate sslCertificate();

    bool javaScriptPrompt(QUrl securityOrigin, const QString &msg, const QString &defaultValue, QString* result);
    bool javaScriptConfirm(QUrl securityOrigin, const QString &msg);
    void javaScriptAlert(QUrl securityOrigin, const QString &msg);

    void setJavaScriptEnabled(bool enabled);

    void addAdBlockRule(const AdBlockRule* rule, const QUrl &url);
    QVector<AdBlockedEntry> adBlockedEntries() const;

    bool hasMultipleUsernames() const;
    QVector<PasswordEntry> autoFillData() const;

    void scheduleAdjustPage();
    bool isRunningLoop();

    bool isLoading() const;
    bool loadingError() const;

    void addRejectedCerts(const QList<QSslCertificate> &certs);
    bool containsRejectedCerts(const QList<QSslCertificate> &certs);

#if QTWEBENGINE_DISABLED
    QWebElement activeElement() const;
#endif
    QString userAgentForUrl(const QUrl &url) const;

    static bool isPointerSafeToUse(WebPage* page);

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
    void windowCloseRequested();
    void authentication(const QUrl &requestUrl, QAuthenticator* auth);
    void proxyAuthentication(const QUrl &requestUrl, QAuthenticator* auth, const QString &proxyHost);

#if QTWEBENGINE_DISABLED
    void frameCreated(QWebFrame* frame);
    void frameInitialLayoutCompleted();
    void dbQuotaExceeded(QWebEngineFrame* frame);
    void printFrame(QWebEngineFrame* frame);
#endif

    void doWebSearch(const QString &text);
    void featurePermissionRequested(const QUrl &origin, const QWebEnginePage::Feature &feature);

#ifdef USE_QTWEBKIT_2_2
    void appCacheQuotaExceeded(QWebSecurityOrigin* origin, quint64 originalQuota);
#endif

protected:
    bool event(QEvent* event);
    QWebEnginePage* createWindow(QWebEnginePage::WebWindowType type);
    QObject* createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);

private:
#if QTWEBENGINE_DISABLED
    bool supportsExtension(Extension extension) const;
    bool extension(Extension extension, const ExtensionOption* option, ExtensionReturn* output = 0);
    QString chooseFile(QWebFrame* originatingFrame, const QString &oldFile);
#endif

    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) Q_DECL_OVERRIDE;

    void handleUnknownProtocol(const QUrl &url);
    void desktopServicesOpen(const QUrl &url);

    static QString s_lastUploadLocation;
    static QUrl s_lastUnsupportedUrl;
    static QTime s_lastUnsupportedUrlTime;
    static QList<WebPage*> s_livingPages;

    NetworkManagerProxy* m_networkProxy;
    TabbedWebView* m_view;
    DelayedFileWatcher* m_fileWatcher;
    QEventLoop* m_runningLoop;

    QSslCertificate m_sslCert;
    QVector<QSslCertificate> m_rejectedSslCerts;
    QVector<AdBlockedEntry> m_adBlockedEntries;
    QVector<PasswordEntry> m_passwordEntries;

    QUrl m_lastRequestUrl;

    int m_loadProgress;
    bool m_blockAlerts;
    bool m_secureStatus;
    bool m_javaScriptEnabled;
    bool m_adjustingScheduled;
};

#endif // WEBPAGE_H
