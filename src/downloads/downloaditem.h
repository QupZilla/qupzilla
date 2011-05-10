/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QFileInfo>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>

namespace Ui {
    class DownloadItem;
}

class DownloadItem : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadItem(QListWidgetItem* item, QNetworkReply* reply, QString path, QString fileName, QPixmap fileIcon, QTime* timer, bool openAfterFinishedDownload, QWidget* parent = 0);
    bool isDownloading() { return m_downloading; }
    bool isCancelled();
    QTime remainingTime() { return m_remTime; }
    double currentSpeed() { return m_currSpeed; }
    int progress();
    ~DownloadItem();

    static QString remaingTimeToString(QTime time);
    static QString currentSpeedToString(double speed);
    static QString fileSizeToString(qint64 size);

signals:
    void deleteItem(DownloadItem*);
    void downloadFinished(bool success);

private slots:
    void finished();
    void metaDataChanged();
    void downloadProgress(qint64 received, qint64 total);
    void stop(bool askForDeleteFile = true);
    void openFile();
    void openFolder();
    void readyRead();
    void error(QNetworkReply::NetworkError);
    void updateDownload();
    void customContextMenuRequested(QPoint pos);
    void clear();

    void goToDownloadPage();
    void copyDownloadLink();

private:
    void timerEvent(QTimerEvent* event);
    void updateDownloadInfo(double currSpeed, qint64 received, qint64 total);
    void mouseDoubleClickEvent(QMouseEvent* e);
    Ui::DownloadItem* ui;

    QListWidgetItem* m_item;
    QNetworkReply* m_reply;
    QString m_path;
    QString m_fileName;
    QTime* m_downTimer;
    QTime m_remTime;
    QBasicTimer m_timer;
    QFile m_outputFile;
    QUrl m_downUrl;
    QUrl m_downloadPage;

    bool m_downloading;
    bool m_openAfterFinish;
    double m_currSpeed;
    qint64 m_received;
    qint64 m_total;
};

#endif // DOWNLOADITEM_H
