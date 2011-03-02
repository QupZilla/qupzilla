#ifndef NETWORKMANAGERPROXY_H
#define NETWORKMANAGERPROXY_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

class WebView;
class WebPage;
class QupZilla;

class NetworkManagerProxy : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit NetworkManagerProxy(QupZilla* mainClass, QObject *parent = 0);
    void setView(WebView* view) { m_view = view; }
    void setPage(WebPage* page) { m_page = page; }

    void setPrimaryNetworkAccessManager(NetworkManagerProxy *manager);
    QNetworkReply* createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData);

protected:
    virtual void populateNetworkRequest(QNetworkRequest &request);

private:
    QupZilla* p_QupZilla;
    WebView* m_view;
    WebPage* m_page;
    NetworkManagerProxy *m_manager;
};

#endif // NETWORKMANAGERPROXY_H
