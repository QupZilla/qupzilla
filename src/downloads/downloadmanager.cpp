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
#include "downloadmanager.h"
#include "ui_downloadmanager.h"
#include "qupzilla.h"
#include "downloadoptionsdialog.h"
#include "downloaditem.h"
#include "ecwin7.h"
#include "networkmanager.h"

DownloadManager::DownloadManager(QWidget* parent) :
    QWidget(parent)
    ,ui(new Ui::DownloadManager)
    ,m_isClosing(false)
{
    ui->setupUi(this);
    ui->clearButton->setIcon(QIcon::fromTheme("edit-clear"));
    //CENTER on screen
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect &size = QWidget::geometry();
    QWidget::move( (screen.width()-size.width())/2, (screen.height()-size.height())/2 );

    m_iconProvider = new QFileIconProvider();
    m_networkManager = mApp->networkManager();

    QSettings settings(mApp->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("DownloadManager");
    m_downloadPath = settings.value("defaultDownloadPath", "").toString();
    m_lastDownloadPath = settings.value("lastDownloadPath",QDir::homePath()+"/").toString();
    settings.endGroup();

    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearList()));

#ifdef W7TASKBAR
    win7.init(this->winId());
#endif
}

#ifdef W7TASKBAR
bool DownloadManager::winEvent(MSG* message, long* result)
{
    return win7.winEvent(message, result);
}
#endif

void DownloadManager::timerEvent(QTimerEvent* event)
{
    QList<QTime> remTimes;
    QList<int> progresses;
    QList<double> speeds;

    if (event->timerId() == m_timer.timerId()) {
        if (!ui->list->count()) {
            ui->speedLabel->clear();
            setWindowTitle(tr("Download Manager"));
            return;
        }
        for (int i = 0; i < ui->list->count(); i++) {
            DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
            if (!downItem || (downItem && downItem->isCancelled()))
                continue;
            if (!downItem->isDownloading()) {
                progresses.append(100);
                continue;
            } else
                progresses.append(downItem->progress());
            remTimes.append(downItem->remainingTime());
            speeds.append(downItem->currentSpeed());
        }
        if (remTimes.isEmpty()) {
            ui->speedLabel->clear();
            setWindowTitle(tr("Download Manager"));
            return;
        }

        QTime remaining;
        foreach (QTime time, remTimes) {
            if (time > remaining)
                remaining = time;
        }

        int progress = 0;
        foreach (int prog, progresses)
            progress+=prog;
        progress = progress / progresses.count();

        double speed = 0.00;
        foreach (double spee, speeds)
            speed+=spee;

        ui->speedLabel->setText(tr("%1% of %2 files (%3) %4 remaining").arg(QString::number(progress),QString::number(progresses.count()),
                                                                            DownloadItem::currentSpeedToString(speed),
                                                                            DownloadItem::remaingTimeToString(remaining)));
        setWindowTitle(QString::number(progress) + tr("% - Download Manager"));
#ifdef W7TASKBAR
        win7.setProgressValue(progress, 100);
        win7.setProgressState(win7.Normal);
#endif
    } else
        QWidget::timerEvent(event);
}

void DownloadManager::clearList()
{
    QList<DownloadItem*> items;
    for (int i = 0; i < ui->list->count(); i++) {
        DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
        if (!downItem)
            continue;
        if (downItem->isDownloading())
            continue;
        items.append(downItem);
    }
    qDeleteAll(items);
}

void DownloadManager::download(const QNetworkRequest &request, bool askWhatToDo)
{
    handleUnsupportedContent(m_networkManager->get(request), askWhatToDo);
}

void DownloadManager::handleUnsupportedContent(QNetworkReply* reply, bool askWhatToDo)
{
    QString path;
    QString fileName;
    QString userFileName;
    QString _fileName = getFileName(reply);

    QFileInfo info(reply->url().toString());
    QTemporaryFile tempFile("XXXXXX."+info.suffix());
    tempFile.open();
    QFileInfo tempInfo(tempFile.fileName());
    QPixmap fileIcon = m_iconProvider->icon(tempInfo).pixmap(30,30);
    QString mimeType = m_iconProvider->type(tempInfo);

    bool openFileOptionsChoosed = false;
    if (askWhatToDo) {
        DownloadOptionsDialog* dialog = new DownloadOptionsDialog(_fileName, fileIcon, mimeType, reply->url(), mApp->activeWindow());
        switch (dialog->exec()) {
        case 0:  //Cancelled
            return;
            break;
        case 1: //Open
            openFileOptionsChoosed = true;
            break;
        case 2: //Save
            break;
        }
    }

    if (!openFileOptionsChoosed) {
        if (m_downloadPath.isEmpty())
            userFileName = QFileDialog::getSaveFileName(mApp->getWindow(), tr("Save file as..."),m_lastDownloadPath+_fileName);
        else
            userFileName = m_downloadPath+_fileName;

        if (userFileName.isEmpty()) {
            reply->abort();
            return;
        }
    } else
        userFileName = QDir::tempPath()+"/"+_fileName;

    int pos = userFileName.lastIndexOf("/");
    if (pos!=-1) {
        int size = userFileName.size();
        path = userFileName.left(pos+1);
        fileName = userFileName.right(size-pos-1);
    }

    if (!path.contains(QDir::tempPath()))
        m_lastDownloadPath = path;

    QSettings settings(mApp->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("DownloadManager");
    settings.setValue("lastDownloadPath",m_lastDownloadPath);
    settings.endGroup();

    QListWidgetItem* item = new QListWidgetItem(ui->list);
    DownloadItem* downItem = new DownloadItem(item, reply, path, fileName, fileIcon, openFileOptionsChoosed, this);
    connect(downItem, SIGNAL(deleteItem(DownloadItem*)), this, SLOT(deleteItem(DownloadItem*)));
    ui->list->setItemWidget(item, downItem);
    item->setSizeHint(downItem->sizeHint());
    show();
    activateWindow();
}

void DownloadManager::deleteItem(DownloadItem* item)
{
    if (item && !item->isDownloading())
        delete item;
}

QString DownloadManager::getFileName(QNetworkReply* reply)
{
    QString path;
    if (reply->hasRawHeader("Content-Disposition")) {
        QString value = reply->rawHeader("Content-Disposition");
        int pos = value.indexOf("filename=");
        if (pos!=-1) {
            QString name = value.mid(pos + 9);
            if (name.startsWith('"') && name.endsWith('"'))
                name = name.mid(1, name.size() - 2);
            path = name;
           }
       }
    if (path.isEmpty())
        path = reply->url().path();

    QFileInfo info(path);
    QString baseName = info.completeBaseName();
    QString endName = info.suffix();

    if (baseName.isEmpty()) {
        baseName = tr("NoNameDownload");
    }

    if (!endName.isEmpty())
        endName="."+endName;

    return baseName+endName;
}

bool DownloadManager::canClose()
{
    if (m_isClosing)
        return true;

    bool isDownloading = false;
    for (int i = 0; i < ui->list->count(); i++) {
        DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
        if (!downItem)
            continue;
        if (downItem->isDownloading()) {
            isDownloading = true;
            break;
        }
    }

    return !isDownloading;
}

void DownloadManager::closeEvent(QCloseEvent* e)
{
    if (mApp->windowCount() == 0) { // No main windows -> we are going to quit
        if (!canClose()){
            QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Warning"),
                             tr("Are you sure to quit? All uncompleted downloads will be cancelled!"), QMessageBox::Yes | QMessageBox::No);
            if (button != QMessageBox::Yes) {
                e->ignore();
                return;
            }
            m_isClosing = true;
        }
        mApp->quitApplication();
    }
    e->accept();
}

DownloadManager::~DownloadManager()
{
    delete ui;
    delete m_networkManager;
    delete m_iconProvider;
}
