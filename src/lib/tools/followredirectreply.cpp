/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "followredirectreply.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

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
