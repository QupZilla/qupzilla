/* ============================================================
* QupZilla - QtWebEngine based browser
* Copyright (C) 2015-2016  David Rosca <nowrep@gmail.com>
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
#include "iconloader.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "qztools.h"

#include <QTimer>
#include <QNetworkReply>

IconLoader::IconLoader(const QUrl &url, QObject *parent)
    : QObject(parent)
    , m_reply(Q_NULLPTR)
{
    m_reply = mApp->networkManager()->get(QNetworkRequest(url));
    connect(m_reply, &QNetworkReply::finished, this, &IconLoader::finished);
}

void IconLoader::finished()
{
    // Ignore error and always emit iconLoaded, even when icon is null
    const QByteArray data = m_reply->readAll();
    emit iconLoaded(QIcon(QPixmap::fromImage(QImage::fromData(data))));

    m_reply->deleteLater();
}
