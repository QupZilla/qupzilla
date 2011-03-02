#ifndef WEBPAGE_H
#define WEBPAGE_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

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
    WebPage(WebView *parent, QupZilla* mainClass);
    void populateNetworkRequest(QNetworkRequest &request);
    ~WebPage();

    QString userAgentForUrl(const QUrl &url) const;
    bool supportsExtension(Extension extension) const { return (extension == ErrorPageExtension); }
    bool extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output);

protected slots:
    QWebPage* createWindow(QWebPage::WebWindowType type);
    void handleUnsupportedContent(QNetworkReply* url);

protected:
    bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);

    QupZilla* p_QupZilla;
    QNetworkRequest m_lastRequest;
    QWebPage::NavigationType m_lastRequestType;
    WebView* m_view;
};

#endif // WEBPAGE_H
