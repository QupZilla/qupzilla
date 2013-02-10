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
#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebView>
#include <QWebElement>

#include "qz_namespace.h"

class WebPage;

class QT_QUPZILLA_EXPORT WebView : public QWebView
{
    Q_OBJECT
public:
    explicit WebView(QWidget* parent = 0);

    QIcon icon() const;
    QString title() const;
    QUrl url() const;

    WebPage* page() const;
    void setPage(QWebPage* page);

    void load(const QNetworkRequest &request, QNetworkAccessManager::Operation operation = QNetworkAccessManager::GetOperation, const QByteArray &body = QByteArray());

    bool loadingError() const;
    bool isLoading() const;

    int loadingProgress() const;
    void fakeLoadingProgress(int progress);

    bool hasRss() const;

    void addNotification(QWidget* notif);
    bool eventFilter(QObject* obj, QEvent* event);

    virtual QWidget* overlayForJsAlert() = 0;
    virtual void disconnectObjects();

    static bool isUrlValid(const QUrl &url);
    static QUrl guessUrlFromString(const QString &string);

signals:
    void viewportResized(QSize);
    void showNotification(QWidget*);
    void privacyChanged(bool);
    void rssChanged(bool);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomReset();

    void load(const QUrl &url);
    void reload();

    void back();
    void forward();

    void selectAll();
    void printPage(QWebFrame* frame = 0);
    void sendPageByMail();
    void savePageAs();

    virtual void closeView() = 0;
    virtual void openUrlInNewTab(const QUrl &url, Qz::NewTabPositionFlag position) = 0;
    virtual void openNewTab() { }

protected slots:
    void slotLoadStarted();
    void slotLoadProgress(int progress);
    void slotLoadFinished();
    void slotIconChanged();
    void slotUrlChanged(const QUrl &url);

    // Context menu slots
    void openUrlInNewWindow();
    void sendLinkByMail();
    void copyLinkToClipboard();
    void downloadUrlToDisk();
    void copyImageToClipboard();
    void openActionUrl();
    void showSource(QWebFrame* frame = 0, const QString &selectedHtml = QString());
    void showSiteInfo();
    void searchSelectedText();
    void searchSelectedTextInBackgroundTab();
    void bookmarkLink();
    void showSourceOfSelection();
    void openUrlInSelectedTab();
    void openUrlInBackgroundTab();

    // To support user's option whether to open in selected or background tab
    void userDefinedOpenUrlInNewTab(const QUrl &url = QUrl(), bool invert = false);
    void userDefinedOpenUrlInBgTab(const QUrl &url = QUrl());

    void createSearchEngine();

    // Clicked frame actions
    void loadClickedFrame();
    void loadClickedFrameInNewTab(bool invert = false);
    void loadClickedFrameInBgTab();
    void reloadClickedFrame();
    void printClickedFrame();
    void clickedFrameZoomIn();
    void clickedFrameZoomOut();
    void clickedFrameZoomReset();
    void showClickedFrameSource();

protected:
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);

    void setZoom(int zoom);
    void applyZoom();
    QUrl lastUrl();

    bool isMediaElement(const QWebElement &element);
    void checkForForm(QMenu* menu, const QWebElement &element);

    void createContextMenu(QMenu* menu, const QWebHitTestResult &hitTest, const QPoint &pos);
    void createPageContextMenu(QMenu* menu, const QPoint &pos);
    void createLinkContextMenu(QMenu* menu, const QWebHitTestResult &hitTest);
    void createImageContextMenu(QMenu* menu, const QWebHitTestResult &hitTest);
    void createSelectedTextContextMenu(QMenu* menu, const QWebHitTestResult &hitTest);
    void createMediaContextMenu(QMenu* menu, const QWebHitTestResult &hitTest);
    void createSpellCheckContextMenu(QMenu* menu);

private slots:
    void pauseMedia();
    void muteMedia();
    void frameStateChanged();
    void emitChangedUrl();
    void checkRss();

private:
    QList<int> m_zoomLevels;
    int m_currentZoom;

    QIcon m_siteIcon;
    QUrl m_siteIconUrl;

    bool m_isLoading;
    int m_progress;
    QUrl m_aboutToLoadUrl;
    QUrl m_lastUrl;

    QWebElement m_clickedElement;
    QWebFrame* m_clickedFrame;
    QUrl m_clickedUrl;

    WebPage* m_page;
    QAction* m_actionReload;
    QAction* m_actionStop;
    bool m_actionsInitialized;

    bool m_disableTouchMocking;
    bool m_isReloading;

    bool m_hasRss;
    bool m_rssChecked;
};

#endif // WEBVIEW_H
