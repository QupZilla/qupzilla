#ifndef ICONFETCHER_H
#define ICONFETCHER_H

#include <QObject>
#include <QIcon>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class FollowRedirectReply : public QObject
{
    Q_OBJECT
public:
    explicit FollowRedirectReply(const QUrl &url, QNetworkAccessManager* manager);
    ~FollowRedirectReply();

    QNetworkReply* reply() { return m_reply; }

signals:
    void finished();

private slots:
    void replyFinished();

private:
    QNetworkAccessManager* m_manager;
    QNetworkReply* m_reply;
    int m_redirectCount;

};

class IconFetcher : public QObject
{
    Q_OBJECT
public:
    explicit IconFetcher(QObject* parent = 0);
    void setNetworkAccessManager(QNetworkAccessManager* manager) { m_manager = manager; }
    void fetchIcon(const QUrl &url);

signals:
    void iconFetched(QIcon);
    void finished();

public slots:

private slots:
    void pageDownloaded();
    void iconDownloaded();

private:
    QNetworkAccessManager* m_manager;

};

#endif // ICONFETCHER_H
