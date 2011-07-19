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
    void load(const QUrl &url);
    QUrl url() const;
    QString title() const;
    void reload();
    WebPage* webPage() const;
    WebTab* webTab() const;
    QString getIp() { return m_currentIp; }
    QLabel* animationLoading(int index, bool addMovie);
    QIcon siteIcon();
    void addNotification(QWidget* notif);
    bool hasRss() { return !m_rss.isEmpty(); }
    QList<QPair<QString,QString> > getRss() { return m_rss; } //FIXME: Make RSS as struct
    void setMouseWheelEnabled(bool state) { m_mouseWheelEnabled = state; }

    void setLocationBar(LocationBar* bar) { m_locationBar = bar; }
    LocationBar* locationBar() { return m_locationBar; }

    static QUrl guessUrlFromString(const QString &string);
    static bool isUrlValid(const QUrl &url);
    int tabIndex() const;

public slots:
    void stop(){ if (page()) {emit ipChanged(m_currentIp); page()->triggerAction(QWebPage::Stop); loadFinished(true);} }
    void back(){ if (page()) {emit ipChanged(m_currentIp); page()->triggerAction(QWebPage::Back);} }
    void forward(){ if (page()) {emit ipChanged(m_currentIp); page()->triggerAction(QWebPage::Forward);} }
    void slotReload(){ if (page()) {emit ipChanged(m_currentIp); page()->triggerAction(QWebPage::Reload);} }
    void iconChanged();
    void selectAll();
    void closeTab();

    void zoomIn();
    void zoomOut();

private slots:
    void trackMouse(bool state) { m_mouseTrack = state; }
    void showImage();
    void copyImageToClipboard();
    void downloadImageToDisk();
    void searchOnGoogle();
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
    void showSourceOfSelection();
    void showSiteInfo();
    void getFocus(const QUrl &urla);
    void showInspector();
    void stopAnimation();
    void setIp(const QHostInfo &info);
    void checkRss();
    void slotIconChanged();

private:
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);
    void wheelEvent(QWheelEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void resizeEvent(QResizeEvent* event);
    TabWidget* tabWidget() const;
    bool isCurrent();
    void applyZoom();

    QupZilla* p_QupZilla;
    int m_progress;
    bool m_isLoading;
    QString m_hoveredLink;
    QList<int> m_zoomLevels;
    int m_currentZoom;
    QUrl m_aboutToLoadUrl;
    QUrl m_lastUrl;
    bool m_wantsClose;
    QString m_currentIp;
    QList<QPair<QString,QString> > m_rss;
    QIcon m_siteIcon;

    WebPage* m_page;
    WebTab* m_webTab;
    NetworkManagerProxy* m_networkProxy;
    LocationBar* m_locationBar;

    bool m_mouseTrack;
    bool m_navigationVisible;
    bool m_mouseWheelEnabled;
    //QTimer* m_loadingTimer; //Too confusing

signals:
    void showUrl(QUrl url);
    void siteIconChanged();
    void wantsCloseTab(int index);
    void changed();
    void ipChanged(QString ip);
    void showNotification(QWidget* notif);
    void viewportResized(QSize size);
    void rssChanged(bool state);
};

#endif // WEBVIEW_H
