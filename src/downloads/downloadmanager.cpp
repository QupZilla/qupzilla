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
#include "globalfunctions.h"
#include "webpage.h"
#include "downloadfilehelper.h"

DownloadManager::DownloadManager(QWidget* parent) :
    QWidget(parent)
    ,ui(new Ui::DownloadManager)
    ,m_isClosing(false)
{
    setWindowFlags(windowFlags() ^ Qt::WindowMaximizeButtonHint);
    ui->setupUi(this);
#ifdef Q_WS_WIN
    if (QtWin::isCompositionEnabled())
        QtWin::extendFrameIntoClientArea(this);
#endif
    ui->clearButton->setIcon(QIcon::fromTheme("edit-clear"));
    qz_centerWidgetOnScreen(this);

    m_networkManager = mApp->networkManager();

    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearList()));

    loadSettings();

#ifdef W7API
    if (QtWin::isRunningWindows7())
        win7.init(this->winId());
#endif
}

void DownloadManager::loadSettings()
{
    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("DownloadManager");
    m_downloadPath = settings.value("defaultDownloadPath", "").toString();
    m_lastDownloadPath = settings.value("lastDownloadPath",QDir::homePath()+"/").toString();
    m_useNativeDialog = settings.value("useNativeDialog",
                                   #ifdef Q_WS_WIN
                                       false
                                   #else
                                       true
                                   #endif
                                       ).toBool();
    settings.endGroup();
}

void DownloadManager::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    emit resized(size());
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

void DownloadManager::handleUnsupportedContent(QNetworkReply* reply, bool askWhatToDo)
{
    DownloadFileHelper* h = new DownloadFileHelper(m_lastDownloadPath, m_downloadPath, m_useNativeDialog);
    h->setDownloadManager(this);
    h->setListWidget(ui->list);
    h->handleUnsupportedContent(reply, askWhatToDo);

    connect(h, SIGNAL(itemCreated(QListWidgetItem*,DownloadItem*)), this, SLOT(itemCreated(QListWidgetItem*,DownloadItem*)));
}

void DownloadManager::itemCreated(QListWidgetItem *item, DownloadItem *downItem)
{
    connect(downItem, SIGNAL(deleteItem(DownloadItem*)), this, SLOT(deleteItem(DownloadItem*)));
    connect(downItem, SIGNAL(downloadFinished(bool)), this, SLOT(downloadFinished(bool)));

    ui->list->setItemWidget(item, downItem);
    item->setSizeHint(downItem->sizeHint());

    show();
    activateWindow();
}

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
}
