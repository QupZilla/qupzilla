#ifndef FOLLOWREDIRECTREPLY_H
#define FOLLOWREDIRECTREPLY_H

#include <QObject>
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

#endif // FOLLOWREDIRECTREPLY_H
