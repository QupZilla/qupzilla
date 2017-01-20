/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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

#include <QWidget>
#include <QBasicTimer>

#include "qzcommon.h"

namespace Ui
{
class DownloadManager;
}

class QUrl;
class QNetworkAccessManager;
class QListWidgetItem;
class QWebEngineDownloadItem;

class DownloadItem;
class EcWin7;
class WebPage;

class QUPZILLA_EXPORT DownloadManager : public QWidget
{
    Q_OBJECT
public:
    enum DownloadOption { OpenFile, SaveFile, ExternalManager, NoOption };

    struct DownloadInfo {
        WebPage* page;
        QString suggestedFileName;

        bool askWhatToDo;
        bool forceChoosingPath;

        DownloadInfo(WebPage* p = 0) {
            page = p;
            suggestedFileName = QString();
            askWhatToDo = true;
            forceChoosingPath = false;
        }
    };

    explicit DownloadManager(QWidget* parent = 0);
    ~DownloadManager();

    void loadSettings();

    void download(QWebEngineDownloadItem *downloadItem);

    bool canClose();

    bool useExternalManager() const;
    void startExternalManager(const QUrl &url);

    void setLastDownloadPath(const QString &lastPath) { m_lastDownloadPath = lastPath; }
    void setLastDownloadOption(const DownloadOption &option) { m_lastDownloadOption = option; }

public slots:
    void show();

#ifdef W7TASKBAR
protected:
    virtual bool nativeEvent(const QByteArray &eventType, void* _message, long* result);
#endif

private slots:
    void clearList();
    void deleteItem(DownloadItem* item);
    void downloadFinished(bool success);

signals:
    void resized(QSize);

private:
#ifdef W7TASKBAR
    EcWin7 win7;
#endif
    void timerEvent(QTimerEvent* e);
    void closeEvent(QCloseEvent* e);
    void resizeEvent(QResizeEvent* e);
    void keyPressEvent(QKeyEvent* e);

    Ui::DownloadManager* ui;
    QBasicTimer m_timer;

    QString m_lastDownloadPath;
    QString m_downloadPath;
    bool m_useNativeDialog;
    bool m_isClosing;
    bool m_closeOnFinish;

    bool m_useExternalManager;
    QString m_externalExecutable;
    QString m_externalArguments;

    DownloadOption m_lastDownloadOption;
};

#endif // DOWNLOADMANAGER_H
