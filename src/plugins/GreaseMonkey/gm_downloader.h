/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2017 David Rosca <nowrep@gmail.com>
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

class QNetworkReply;

class GM_Manager;

class GM_Downloader : public QObject
{
    Q_OBJECT
public:
    enum Mode {
        DownloadMainScript,
        DownloadRequireScript
    };

    explicit GM_Downloader(const QUrl &url, GM_Manager *manager, Mode mode = DownloadMainScript);

    void updateScript(const QString& fileName);

signals:
    void finished(const QString& fileName);
    void error();

private slots:
    void scriptDownloaded();
    void requireDownloaded();

private:
    void downloadRequires();

    GM_Manager* m_manager;
    QNetworkReply *m_reply;
    QString m_fileName;
};

#endif // GM_DOWNLOADER_H
