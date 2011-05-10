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
#include "qtwin.h"
#include "desktopnotificationsfactory.h"

DownloadManager::DownloadManager(QWidget* parent) :
    QWidget(parent)
    ,ui(new Ui::DownloadManager)
    ,m_isClosing(false)
{
    ui->setupUi(this);
#ifdef Q_WS_WIN
    if (QtWin::isCompositionEnabled())
        QtWin::extendFrameIntoClientArea(this);
#endif
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

#ifdef W7API
    if (QtWin::isRunningWindows7())
        win7.init(this->winId());
#endif
}

#ifdef W7API
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
            if (!downItem || (downItem && downItem->isCancelled()) || !downItem->isDownloading())
                continue;
            progresses.append(downItem->progress());
            remTimes.append(downItem->remainingTime());
            speeds.append(downItem->currentSpeed());
        }
        if (remTimes.isEmpty())
            return;

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
#ifdef W7API
        if (QtWin::isRunningWindows7()) {
            win7.setProgressValue(progress, 100);
            win7.setProgressState(win7.Normal);
        }
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

//////////////////////////////////////////////////////
//// Getting where to download requested file
//// in 3 functions, as we are using non blocking
//// dialogs ( this is important to make secured downloading
//// on windows working properly )
//////////////////////////////////////////////////////
void DownloadManager::handleUnsupportedContent(QNetworkReply* reply, bool askWhatToDo)
{
    m_htimer = new QTime();
    m_htimer->start();
    m_h_fileName = getFileName(reply);
    m_hreply = reply;

    QFileInfo info(reply->url().toString());
    QTemporaryFile tempFile("XXXXXX."+info.suffix());
    tempFile.open();
    QFileInfo tempInfo(tempFile.fileName());
    m_hfileIcon = m_iconProvider->icon(tempInfo).pixmap(30,30);
    QString mimeType = m_iconProvider->type(tempInfo);

    if (askWhatToDo) {
        DownloadOptionsDialog* dialog = new DownloadOptionsDialog(m_h_fileName, m_hfileIcon, mimeType, reply->url(), mApp->activeWindow());
        dialog->show();
        connect(dialog, SIGNAL(dialogFinished(int)), this, SLOT(optionsDialogAccepted(int)));
    } else
        optionsDialogAccepted(2);
}

void DownloadManager::optionsDialogAccepted(int finish)
{
    m_hOpenFileChoosed = false;
    switch (finish) {
    case 0:  //Cancelled
        if (m_htimer)
            delete m_htimer;
        return;
        break;
    case 1: //Open
        m_hOpenFileChoosed = true;
        break;
    case 2: //Save
        break;
    }

    if (!m_hOpenFileChoosed) {
        if (m_downloadPath.isEmpty()) {
#ifdef Q_WS_WIN //Well, poor Windows users will use non-native file dialog for downloads
            QFileDialog* dialog = new QFileDialog(mApp->getWindow());
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->setWindowTitle(tr("Save file as..."));
            dialog->setDirectory(m_lastDownloadPath);
            dialog->selectFile(m_h_fileName);
            dialog->show();
            connect(dialog, SIGNAL(fileSelected(QString)), this, SLOT(fileNameChoosed(QString)));
#else
            fileNameChoosed(QFileDialog::getSaveFileName(mApp->getWindow(), tr("Save file as..."), m_lastDownloadPath + m_h_fileName));
#endif
        }
        else
            fileNameChoosed(m_downloadPath + m_h_fileName);
    } else
        fileNameChoosed(QDir::tempPath() + "/" + m_h_fileName);
}

void DownloadManager::fileNameChoosed(const QString &name)
{
    m_huserFileName = name;
    if (m_huserFileName.isEmpty()) {
        m_hreply->abort();
        if (m_htimer)
            delete m_htimer;
        return;
    }

    int pos = m_huserFileName.lastIndexOf("/");
    if (pos != -1) {
        int size = m_huserFileName.size();
        m_hpath = m_huserFileName.left(pos+1);
        m_hfileName = m_huserFileName.right(size-pos-1);
    }

    if (!m_hpath.contains(QDir::tempPath()))
        m_lastDownloadPath = m_hpath;

    QSettings settings(mApp->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("DownloadManager");
    settings.setValue("lastDownloadPath",m_lastDownloadPath);
    settings.endGroup();

    QListWidgetItem* item = new QListWidgetItem(ui->list);
    DownloadItem* downItem = new DownloadItem(item, m_hreply, m_hpath, m_hfileName, m_hfileIcon, m_htimer, m_hOpenFileChoosed, this);
    connect(downItem, SIGNAL(deleteItem(DownloadItem*)), this, SLOT(deleteItem(DownloadItem*)));
    connect(downItem, SIGNAL(downloadFinished(bool)), this, SLOT(downloadFinished(bool)));
    ui->list->setItemWidget(item, downItem);
    item->setSizeHint(downItem->sizeHint());
    show();
    activateWindow();

    //Negating all used variables
    m_hpath.clear();
    m_hfileName.clear();
    m_huserFileName.clear();
    m_h_fileName.clear();
    m_hreply = 0;
    m_hfileIcon = QPixmap();
    m_hOpenFileChoosed = false;
}
//////////////////////////////////////////////////////
//// End here
//////////////////////////////////////////////////////

void DownloadManager::downloadFinished(bool success)
{
    bool downloadingAllFilesFinished = true;
    for (int i = 0; i < ui->list->count(); i++) {
        DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
        if (!downItem || (downItem && downItem->isCancelled()) || !downItem->isDownloading())
            continue;
        downloadingAllFilesFinished = false;
    }

    if (downloadingAllFilesFinished) {
        if (success && qApp->activeWindow() != this) {
            mApp->desktopNotifications()->notify(QPixmap(":icons/notifications/download.png"), tr("Download Finished"), tr("All files have been successfuly downloaded."));
            raise();
            activateWindow();
        }
        ui->speedLabel->clear();
        setWindowTitle(tr("Download Manager"));
#ifdef W7API
        if (QtWin::isRunningWindows7()) {
            win7.setProgressValue(0, 100);
            win7.setProgressState(win7.NoProgress);
        }
#endif
    }
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

    QString name = baseName+endName;
    if (name.startsWith("\""))
        name = name.mid(1);
    if (name.endsWith("\";"))
        name.remove("\";");

    return name;
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
