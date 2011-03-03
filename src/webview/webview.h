#ifndef WEBVIEW_H
#define WEBVIEW_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

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

class WebView : public QWebView
{
    Q_OBJECT
public:
    explicit WebView(QupZilla* mainClass, QWidget *parent = 0);
    ~WebView();
    bool isLoading() { return m_isLoading;}
    int getLoading() { return m_progress; }

    void zoomReset();
    void load(QUrl url);
    QUrl url() const;
    QString title() const;
    void reload();
    WebPage* getPage() const;
    QString getIp() { return m_currentIp; }
    QLabel* animationLoading(int index, bool addMovie);
    void addNotification(QWidget* notif);
    bool hasRss() { return !m_rss.isEmpty(); }
    QList<QPair<QString,QString> > getRss() { return m_rss; }

    static QUrl guessUrlFromString(const QString &string);
    static bool isUrlValid(const QUrl &url);

public slots:
    void stop(){ if (page()) {emit ipChanged(m_currentIp); page()->triggerAction(QWebPage::Stop); loadFinished(true);} }
    void back(){ if (page()) {emit ipChanged(m_currentIp); page()->triggerAction(QWebPage::Back);} }
    void forward(){ if (page()) {emit ipChanged(m_currentIp); page()->triggerAction(QWebPage::Forward);} }
    void slotReload(){ if (page()) {emit ipChanged(m_currentIp); page()->triggerAction(QWebPage::Reload);} }
    void iconChanged();
    void selectAll();

    void zoomIn();
    void zoomOut();

private slots:
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
    void titleChanged(QString title);
    void linkHovered(const QString &link, const QString &title, const QString &content);
    void openUrlInNewWindow();
    void openUrlInNewTab();
    void closeTab();
    void downloadLinkToDisk();
    void sendLinkByMail();
    void bookmarkLink();
    void showSource();
    void showSiteInfo();
    void getFocus(const QUrl &urla);
    void showInspector();
    void stopAnimation();
    void setIp(QHostInfo info);
    void checkRss();

private:
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void wheelEvent(QWheelEvent *event);
    TabWidget* tabWidget() const;
    int tabIndex() const;
    bool isCurrent();
    void applyZoom();

    QupZilla* p_QupZilla;
    int m_progress;
    bool m_isLoading;
    QString m_hoveredLink;
    QList<int> m_zoomLevels;
    int m_currentZoom;
    QUrl m_aboutToLoadUrl;
    bool m_wantsClose;
    QString m_currentIp;
    QList<QPair<QString,QString> > m_rss;

    WebPage* m_page;
    NetworkManagerProxy* m_networkProxy;
    //QTimer* m_loadingTimer; //Too confusing

signals:
    void showUrl(QUrl url);
    void siteIconChanged();
    void setPrivacy(bool state);
    void wantsCloseTab(int index);
    void changed();
    void ipChanged(QString ip);
    void showNotification(QWidget* notif);
};

#endif // WEBVIEW_H
