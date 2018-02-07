/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include <QWebEngineScript>
#include <QWebEngineFullScreenRequest>
#include <QVector>

#include "qzcommon.h"

class QEventLoop;
class QWebEngineDownloadItem;

class WebView;
class WebHitTestResult;
class DelayedFileWatcher;

class QUPZILLA_EXPORT WebPage : public QWebEnginePage
{
    Q_OBJECT

public:
    enum JsWorld {
        UnsafeJsWorld = QWebEngineScript::MainWorld,
        SafeJsWorld = QWebEngineScript::ApplicationWorld
    };

    explicit WebPage(QObject* parent = 0);
    ~WebPage();

    WebView *view() const;

    bool execPrintPage(QPrinter *printer, int timeout = 1000);
    QVariant execJavaScript(const QString &scriptSource, quint32 worldId = UnsafeJsWorld, int timeout = 500);

    QPointF mapToViewport(const QPointF &pos) const;
    WebHitTestResult hitTestContent(const QPoint &pos) const;

    void scroll(int x, int y);
    void setScrollPosition(const QPointF &pos);

    bool javaScriptPrompt(const QUrl &securityOrigin, const QString &msg, const QString &defaultValue, QString* result) Q_DECL_OVERRIDE;
    bool javaScriptConfirm(const QUrl &securityOrigin, const QString &msg) Q_DECL_OVERRIDE;
    void javaScriptAlert(const QUrl &securityOrigin, const QString &msg) Q_DECL_OVERRIDE;
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID) override;

    QStringList autoFillUsernames() const;

    bool isRunningLoop();

    bool isLoading() const;

signals:
    void privacyChanged(bool status);
    void printRequested();
    void navigationRequestAccepted(const QUrl &url, NavigationType type, bool isMainFrame);

protected slots:
    void progress(int prog);
    void finished();

private slots:
    void urlChanged(const QUrl &url);
    void watchedFileChanged(const QString &file);
    void windowCloseRequested();
    void fullScreenRequested(QWebEngineFullScreenRequest fullScreenRequest);
    void featurePermissionRequested(const QUrl &origin, const QWebEnginePage::Feature &feature);
    void renderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode);

private:
    void setupWebChannelForUrl(const QUrl &url);
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) Q_DECL_OVERRIDE;
    bool certificateError(const QWebEngineCertificateError &error) Q_DECL_OVERRIDE;
    QStringList chooseFiles(FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes) Q_DECL_OVERRIDE;
    QWebEnginePage* createWindow(QWebEnginePage::WebWindowType type) Q_DECL_OVERRIDE;

    void handleUnknownProtocol(const QUrl &url);
    void desktopServicesOpen(const QUrl &url);

    static QString s_lastUploadLocation;
    static QUrl s_lastUnsupportedUrl;
    static QTime s_lastUnsupportedUrlTime;

    DelayedFileWatcher* m_fileWatcher;
    QEventLoop* m_runningLoop;

    QStringList m_autoFillUsernames;

    int m_loadProgress;
    bool m_blockAlerts;
    bool m_secureStatus;

    QMetaObject::Connection m_contentsResizedConnection;

    QUrl m_channelUrl;
    int m_channelWorldId = -1;
    QTimer *m_setupChannelTimer = nullptr;

    friend class WebView;
};

#endif // WEBPAGE_H
