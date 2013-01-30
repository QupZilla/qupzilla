/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2013  David Rosca <nowrep@gmail.com>
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
#ifndef GM_DOWNLOADER_H
#define GM_DOWNLOADER_H

#include <QObject>
#include <QList>
#include <QUrl>

class QNetworkRequest;

class GM_Manager;

class FollowRedirectReply;

class GM_Downloader : public QObject
{
    Q_OBJECT
public:
    explicit GM_Downloader(const QNetworkRequest &request, GM_Manager* manager);

private slots:
    void scriptDownloaded();
    void requireDownloaded();

private:
    void downloadRequires();

    GM_Manager* m_manager;
    FollowRedirectReply* m_reply;
    QWidget* m_widget;

    QString m_fileName;
    QList<QUrl> m_requireUrls;

};

#endif // GM_DOWNLOADER_H
