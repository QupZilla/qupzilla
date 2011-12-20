/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#include <QDebug>
#include <QTabWidget>
#include <QContextMenuEvent>
#include <QWebElement>
#include <QClipboard>
#include <QLabel>
#include <QProcess>
#include <QWebInspector>
#include <QDockWidget>
#include <QWebPage>
#include <QHostInfo>

class QupZilla;
class TabWidget;
class WebPage;
class NetworkManagerProxy;
class WebTab;
class LocationBar;
class WebView : public QWebView
{
    Q_OBJECT
public:
    explicit WebView(QupZilla* mainClass, WebTab* webTab);
    ~WebView();
    bool isLoading() { return m_isLoading;}
    int getLoading() { return m_progress; }

    void zoomReset();
    QUrl url() const;
    QString title() const;
    void reload();
    WebPage* webPage() const;
    WebTab* webTab() const;
    QString getIp() { return m_currentIp; }
    QLabel* animationLoading(int index, bool addMovie);
    QIcon siteIcon();
    void addNotification(QWidget* notif);
    bool hasRss() { return m_hasRss; }
    void setMouseWheelEnabled(bool state) { m_mouseWheelEnabled = state; }

    bool eventFilter(QObject* obj, QEvent* event);

    void setLocationBar(LocationBar* bar) { m_locationBar = bar; }
    LocationBar* locationBar() { return m_locationBar; }

    static QUrl guessUrlFromString(const QString &string);
    static bool isUrlValid(const QUrl &url);
    int tabIndex() const;

signals:
    void showUrl(QUrl url);
    void siteIconChanged();
    void wantsCloseTab(int index);
    void changed();
    void ipChanged(QString ip);
    void showNotification(QWidget* notif);
    void viewportResized(QSize size);
    void rssChanged(bool state);

public slots:
    void load(const QUrl &url);

    void stop();
    void back();
    void forward();
    void slotReload();
    void iconChanged();
    void selectAll();
    void closeTab();

    void zoomIn();
    void zoomOut();

private slots:
    void copyText();

    void trackMouse(bool state) { m_mouseTrack = state; }
    void showImage();
    void copyImageToClipboard();
    void downloadImageToDisk();
    void searchSelectedText();
    void copyLinkToClipboard();
    void loadStarted();
    void downloadRequested(const QNetworkRequest &request);
    void setProgress(int prog);
    void loadFinished(bool state);
    void linkClicked(const QUrl &url);
    void urlChanged(const QUrl &url);
    void titleChanged();
    void linkHovered(const QString &link, const QString &title, const QString &content);
    void openUrlInNewWindow();
    void openUrlInNewTab();
    void downloadLinkToDisk();
    void sendLinkByMail();
    void bookmarkLink();
    void showSource();
#if QT_VERSION >= 0x040800
    void showSourceOfSelection();
#endif
    void showSiteInfo();
    void getFocus(const QUrl &urla);
    void showInspector();
    void stopAnimation();
    void setIp(const QHostInfo &info);
    void checkRss();
    void slotIconChanged();

private:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);
    void wheelEvent(QWheelEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void resizeEvent(QResizeEvent* event);
    void keyPressEvent(QKeyEvent* event);

    TabWidget* tabWidget() const;
    bool isCurrent();
    void applyZoom();

    QupZilla* p_QupZilla;

    QString m_hoveredLink;
    QList<int> m_zoomLevels;
    QUrl m_aboutToLoadUrl;
    QUrl m_lastUrl;
    QString m_currentIp;
    QIcon m_siteIcon;
    int m_progress;
    int m_currentZoom;

    WebPage* m_page;
    WebTab* m_webTab;
    NetworkManagerProxy* m_networkProxy;
    LocationBar* m_locationBar;
    QMenu* m_menu;

    bool m_mouseTrack;
    bool m_navigationVisible;
    bool m_mouseWheelEnabled;
    bool m_wantsClose;
    bool m_isLoading;

    bool m_hasRss;
    bool m_rssChecked;

    QList<QTouchEvent::TouchPoint> m_touchPoints;
    //QTimer* m_loadingTimer;

//    static QList<WebView*> s_deletedPointers;
//    static bool isPointerValid(WebView* pointer);
};

#endif // WEBVIEW_H
