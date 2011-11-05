/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include "iconfetcher.h"
#include "followredirectreply.h"

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
//
//        Rather getting favicon.ico from base directory than from subfolders
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
