#include "iconfetcher.h"

#include <QDebug>

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

    if ( (replyStatus != 301 && replyStatus != 302) || m_redirectCount == 5) {
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

IconFetcher::IconFetcher(QObject* parent)
    : QObject(parent)
    , m_manager(0)
{
}

void IconFetcher::fetchIcon(const QUrl &url)
{
    if (!m_manager)
        return;

    FollowRedirectReply* reply = new FollowRedirectReply(url, m_manager);
    connect(reply, SIGNAL(finished()), this, SLOT(pageDownloaded()));
}

void IconFetcher::pageDownloaded()
{
    FollowRedirectReply* reply = qobject_cast<FollowRedirectReply*> (sender());
    if (!reply)
        return;

    QString html = reply->reply()->readAll();
    QUrl replyUrl = reply->reply()->url();
    delete reply;

    QRegExp rx("<link(.*)>", Qt::CaseInsensitive);
    rx.setMinimal(true);

    QString shortcutIconTag;
    int pos = 0;
    while ((pos = rx.indexIn(html, pos)) != -1) {
        QString linkTag = rx.cap(0);
        pos += rx.matchedLength();

        if (linkTag.contains("rel=\"shortcut icon\"", Qt::CaseInsensitive)) {
            shortcutIconTag = linkTag;
            break;
        }
    }

    FollowRedirectReply* newReply;
    if (shortcutIconTag.isEmpty()) {
//        QUrl faviconUrl = replyUrl.resolved(QUrl("favicon.ico"));
        // Rather getting favicon.ico from base directory than from subfolders
        QUrl faviconUrl = QUrl(replyUrl.toString(QUrl::RemovePath | QUrl::RemoveQuery) + "/favicon.ico");
        newReply = new FollowRedirectReply(faviconUrl, m_manager);
    }
    else {
        QRegExp rx("href=\"(.*)\"", Qt::CaseInsensitive);
        rx.setMinimal(true);
        rx.indexIn(shortcutIconTag);
        QUrl url = QUrl(rx.cap(1));

        QUrl iconUrl = QUrl(replyUrl).resolved(url);
        newReply = new FollowRedirectReply(iconUrl, m_manager);
    }

    connect(newReply, SIGNAL(finished()), this, SLOT(iconDownloaded()));
}

void IconFetcher::iconDownloaded()
{
    FollowRedirectReply* reply = qobject_cast<FollowRedirectReply*> (sender());
    if (!reply)
        return;

    QByteArray response = reply->reply()->readAll();
    delete reply;

    if (!response.isEmpty()) {
        QImage image;
        image.loadFromData(response);
        QIcon icon = QIcon(QPixmap::fromImage(image));
        if (!icon.isNull())
            emit iconFetched(icon);
    }

    emit finished();
}
