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
#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QDialog>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QNetworkReply>
#include <QListWidgetItem>

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

    void loadSettings();

    void download(const QNetworkRequest &request, bool askWhatToDo = true);
    void handleUnsupportedContent(QNetworkReply* reply, bool askWhatToDo = true);
    bool canClose();
    void setLastDownloadPath(const QString &lastPath) { m_lastDownloadPath = lastPath; }

public slots:
    void show();

#ifdef W7API
protected:
    virtual bool winEvent(MSG* message, long* result);
#endif

private slots:
    void clearList();
    void deleteItem(DownloadItem* item);

    void itemCreated(QListWidgetItem* item, DownloadItem* downItem);
    void downloadFinished(bool success);

signals:
    void resized(QSize);

private:
#ifdef W7API
    EcWin7 win7;
#endif
    void timerEvent(QTimerEvent* event);
    void closeEvent(QCloseEvent* e);
    void resizeEvent(QResizeEvent *e);

    Ui::DownloadManager* ui;
    NetworkManager* m_networkManager;
    QBasicTimer m_timer;

    QString m_lastDownloadPath;
    QString m_downloadPath;
    bool m_useNativeDialog;
    bool m_isClosing;
    bool m_closeOnFinish;
};

#endif // DOWNLOADMANAGER_H
