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
#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QDialog>
#include <QLabel>
#include <QFileIconProvider>
#include <QDesktopWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QTime>
#include <QDateTime>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QNetworkReply>

#include "ecwin7.h"

namespace Ui {
    class DownloadManager;
}

class DownloadItem;
class EcWin7;
class NetworkManager;
class DownloadManager : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadManager(QWidget* parent = 0);
    ~DownloadManager();

    void download(const QNetworkRequest &request);
    void handleUnsupportedContent(QNetworkReply* reply);
    bool canClose();

    void show() { m_timer.start(1000*2, this); QWidget::show(); }

#ifdef W7TASKBAR
protected:
    virtual bool winEvent(MSG* message, long* result);
#endif

private slots:
    void clearList();
    void deleteItem(DownloadItem* item);

private:
#ifdef W7TASKBAR
    EcWin7 win7;
#endif
    void timerEvent(QTimerEvent* event);
    QString getFileName(QNetworkReply* reply);
    void closeEvent(QCloseEvent* e);

    Ui::DownloadManager* ui;
    NetworkManager* m_networkManager;
    QFileIconProvider* m_iconProvider;

    QString m_lastDownloadPath;
    QString m_downloadPath;
    QBasicTimer m_timer;

    bool m_isClosing;
};

#endif // DOWNLOADMANAGER_H
