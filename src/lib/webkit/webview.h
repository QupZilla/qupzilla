/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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

#include "qzcommon.h"
#include "loadrequest.h"

class WebPage;
class LoadRequest;

class QUPZILLA_EXPORT WebView : public QWebView
{
    Q_OBJECT
public:
    explicit WebView(QWidget* parent = 0);
    ~WebView();

    QIcon icon() const;
    QUrl url() const;

    QString title() const;
    bool isTitleEmpty() const;

    WebPage* page() const;
    void setPage(QWebPage* page);

    void load(const LoadRequest &request);
    bool loadingError() const;
    bool isLoading() const;

    int loadingProgress() const;
    void fakeLoadingProgress(int progress);

    bool hasRss() const;
    QWebElement activeElement() const;

    // Set zoom level (0 - 17)
    int zoomLevel() const;
    void setZoomLevel(int level);

    // Executes window.onbeforeunload, returns true if view can be closed
    bool onBeforeUnload();

    void addNotification(QWidget* notif);
    bool eventFilter(QObject* obj, QEvent* event);

    virtual QWidget* overlayWidget() = 0;

    static bool isUrlValid(const QUrl &url);
    static QUrl guessUrlFromString(const QString &string);
    static QList<int> zoomLevels();

    // Force context menu event to be sent on mouse release
    // This allows to override right mouse button events (eg. for mouse gestures)
    static bool forceContextMenuOnMouseRelease();
    static void setForceContextMenuOnMouseRelease(bool force);

signals:
    void viewportResized(QSize);
    void showNotification(QWidget*);
    void privacyChanged(bool);
    void rssChanged(bool);
    void zoomLevelChanged(int);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomReset();

    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editSelectAll();
    void editDelete();

    void reload();
    void reloadBypassCache();

    void back();
    void forward();

    void printPage(QWebFrame* frame = 0);
    void sendPageByMail();
    void savePageAs();

    void openUrlInNewTab(const QUrl &url, Qz::NewTabPositionFlags position);

    virtual void closeView() = 0;
    virtual void openNewTab(Qz::NewTabPositionFlags position) { Q_UNUSED(position) }
    virtual void loadInNewTab(const LoadRequest &req, Qz::NewTabPositionFlags position) = 0;

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

    void loadRequest(const LoadRequest &req);
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

private slots:
    void pauseMedia();
    void muteMedia();
    void frameStateChanged();
    void emitChangedUrl();
    void checkRss();
    void addSpeedDial();
    void configureSpeedDial();
    void reloadAllSpeedDials();

private:
    QList<int> m_zoomLevels;
    int m_currentZoomLevel;

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

    static bool s_forceContextMenuOnMouseRelease;
};

#endif // WEBVIEW_H
