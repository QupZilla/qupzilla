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
#ifndef ICONFETCHER_H
#define ICONFETCHER_H

#include "qz_namespace.h"

#include <QObject>
#include <QIcon>
#include <QVariant>
#include <QUrl>

class QNetworkAccessManager;
class QUrl;

class FollowRedirectReply;

class QT_QUPZILLA_EXPORT IconFetcher : public QObject
{
    Q_OBJECT
public:
    explicit IconFetcher(QObject* parent = 0);
    void setNetworkAccessManager(QNetworkAccessManager* manager) { m_manager = manager; }
    void fetchIcon(const QUrl &url);

    void setData(const QVariant &data) { m_data = data; }
    QVariant data() { return m_data; }

    QUrl url() { return m_url; }

signals:
    void iconFetched(QImage);
    void finished();

public slots:

private slots:
    void pageDownloaded();
    void iconDownloaded();

private:
    QNetworkAccessManager* m_manager;

    QVariant m_data;
    QUrl m_url;

};

#endif // ICONFETCHER_H
