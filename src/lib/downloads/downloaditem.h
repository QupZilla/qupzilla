/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H

#include <QWidget>
#include <QFile>
#include <QBasicTimer>
#include <QUrl>
#include <QNetworkReply>
#include <QTime>

#include "qzcommon.h"

namespace Ui
{
class DownloadItem;
}

class QListWidgetItem;
class QWebEngineDownloadItem;

class DownloadManager;

class QUPZILLA_EXPORT DownloadItem : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadItem(QListWidgetItem* item, QWebEngineDownloadItem* downloadItem, const QString &path, const QString &fileName, bool openFile, DownloadManager* manager);
    bool isDownloading() { return m_downloading; }
    bool isCancelled();
    QTime remainingTime() { return m_remTime; }
    double currentSpeed() { return m_currSpeed; }
    int progress();
    ~DownloadItem();

    static QString remaingTimeToString(QTime time);
    static QString currentSpeedToString(double speed);

signals:
    void deleteItem(DownloadItem*);
    void downloadFinished(bool success);

private slots:
    void parentResized(const QSize &size);
    void finished();
    void downloadProgress(qint64 received, qint64 total);
    void stop();
    void openFile();
    void openFolder();
    void customContextMenuRequested(const QPoint &pos);
    void clear();

    void copyDownloadLink();

private:
    void startDownloading();

    void updateIcon();
    void updateDownloadInfo(double currSpeed, qint64 received, qint64 total);
    void mouseDoubleClickEvent(QMouseEvent* e);

    Ui::DownloadItem* ui;

    QListWidgetItem* m_item;
    QWebEngineDownloadItem* m_download;
    QString m_path;
    QString m_fileName;
    QTime m_downTimer;
    QTime m_remTime;
    QBasicTimer m_timer;
    QUrl m_downUrl;
    bool m_openFile;
    bool m_validIcon;

    bool m_downloading;
    bool m_downloadStopped;
    double m_currSpeed;
    qint64 m_received;
    qint64 m_total;
};

#endif // DOWNLOADITEM_H
