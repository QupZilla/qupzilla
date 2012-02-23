#include "followredirectreply.h"

FollowRedirectReply::FollowRedirectReply(const QUrl &url, QNetworkAccessManager* manager)
    : QObject()
    , m_manager(manager)
    , m_redirectCount(0)
{
    m_reply = m_manager->get(QNetworkRequest(url));
    connect(m_reply, SIGNAL(finished()), this, SLOT(replyFinished()));
}

void FollowRedirectReply::replyFinished()
{
    int replyStatus = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if ((replyStatus != 301 && replyStatus != 302) || m_redirectCount == 5) {
        emit finished();
        return;
    }
    m_redirectCount++;

    QUrl redirectUrl = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    m_reply->close();
    m_reply->deleteLater();

    m_reply = m_manager->get(QNetworkRequest(redirectUrl));
    connect(m_reply, SIGNAL(finished()), this, SLOT(replyFinished()));
}

FollowRedirectReply::~FollowRedirectReply()
{
    m_reply->close();
    m_reply->deleteLater();
}
