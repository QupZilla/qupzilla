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
#include "downloadmanager.h"
#include "ui_downloadmanager.h"
#include "browserwindow.h"
#include "mainapplication.h"
#include "downloadoptionsdialog.h"
#include "downloaditem.h"
#include "ecwin7.h"
#include "networkmanager.h"
#include "qtwin.h"
#include "desktopnotificationsfactory.h"
#include "qztools.h"
#include "webpage.h"
#include "downloadfilehelper.h"
#include "settings.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QDir>
#include <QShortcut>

DownloadManager::DownloadManager(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DownloadManager)
    , m_isClosing(false)
    , m_lastDownloadOption(NoOption)
{
    setWindowFlags(windowFlags() ^ Qt::WindowMaximizeButtonHint);
    ui->setupUi(this);
#ifdef Q_OS_WIN
    if (QtWin::isCompositionEnabled()) {
        QtWin::extendFrameIntoClientArea(this);
    }
#endif
    ui->clearButton->setIcon(QIcon::fromTheme("edit-clear"));
    QzTools::centerWidgetOnScreen(this);

    m_networkManager = mApp->networkManager();

    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearList()));

    QShortcut* clearShortcut = new QShortcut(QKeySequence("CTRL+L"), this);
    connect(clearShortcut, SIGNAL(activated()), this, SLOT(clearList()));

    loadSettings();

    QzTools::setWmClass("Download Manager", this);

#ifdef W7TASKBAR
    if (QtWin::isRunningWindows7()) {
        win7.init(QtWin::hwndOfWidget(this));
    }
#endif
}

void DownloadManager::loadSettings()
{
    Settings settings;
    settings.beginGroup("DownloadManager");
    m_downloadPath = settings.value("defaultDownloadPath", QString()).toString();
    m_lastDownloadPath = settings.value("lastDownloadPath", QDir::homePath().append(QLatin1Char('/'))).toString();
    m_closeOnFinish = settings.value("CloseManagerOnFinish", false).toBool();
    m_useNativeDialog = settings.value("useNativeDialog", DEFAULT_DOWNLOAD_USE_NATIVE_DIALOG).toBool();

    m_useExternalManager = settings.value("UseExternalManager", false).toBool();
    m_externalExecutable = settings.value("ExternalManagerExecutable", QString()).toString();
    m_externalArguments = settings.value("ExternalManagerArguments", QString()).toString();
    settings.endGroup();

    if (!m_externalArguments.contains(QLatin1String("%d"))) {
        m_externalArguments.append(QLatin1String(" %d"));
    }
}

void DownloadManager::show()
{
    m_timer.start(500, this);

    QWidget::show();
    raise();
    activateWindow();
}

void DownloadManager::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    emit resized(size());
}

void DownloadManager::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape
        || (e->key() == Qt::Key_W && e->modifiers() == Qt::ControlModifier)) {
        close();
    }

    QWidget::keyPressEvent(e);
}

void DownloadManager::startExternalManager(const QUrl &url)
{
    QString arguments = m_externalArguments;
    arguments.replace(QLatin1String("%d"), url.toEncoded());

    QzTools::startExternalProcess(m_externalExecutable, arguments);
    m_lastDownloadOption = ExternalManager;
}

#ifdef W7TASKBAR
#if (QT_VERSION < 0x050000)
bool DownloadManager::winEvent(MSG* message, long* result)
{
#else
bool DownloadManager::nativeEvent(const QByteArray &eventType, void* _message, long* result)
{
    Q_UNUSED(eventType)
    MSG* message = static_cast<MSG*>(_message);
#endif
    return win7.winEvent(message, result);
}
#endif

void DownloadManager::timerEvent(QTimerEvent* e)
{
    QVector<QTime> remTimes;
    QVector<int> progresses;
    QVector<double> speeds;

    if (e->timerId() == m_timer.timerId()) {
        if (!ui->list->count()) {
            ui->speedLabel->clear();
            setWindowTitle(tr("Download Manager"));
            return;
        }
        for (int i = 0; i < ui->list->count(); i++) {
            DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
            if (!downItem || (downItem && downItem->isCancelled()) || !downItem->isDownloading()) {
                continue;
            }
            progresses.append(downItem->progress());
            remTimes.append(downItem->remainingTime());
            speeds.append(downItem->currentSpeed());
        }
        if (remTimes.isEmpty()) {
            return;
        }

        QTime remaining;
        foreach (const QTime &time, remTimes) {
            if (time > remaining) {
                remaining = time;
            }
        }

        int progress = 0;
        foreach (int prog, progresses) {
            progress += prog;
        }
        progress = progress / progresses.count();

        double speed = 0.00;
        foreach (double spee, speeds) {
            speed += spee;
        }

        ui->speedLabel->setText(tr("%1% of %2 files (%3) %4 remaining").arg(QString::number(progress), QString::number(progresses.count()),
                                DownloadItem::currentSpeedToString(speed),
                                DownloadItem::remaingTimeToString(remaining)));
        setWindowTitle(tr("%1% - Download Manager").arg(progress));
#ifdef W7TASKBAR
        if (QtWin::isRunningWindows7()) {
            win7.setProgressValue(progress, 100);
            win7.setProgressState(win7.Normal);
        }
#endif
    }
    else {
        QWidget::timerEvent(e);
    }
}

void DownloadManager::clearList()
{
    QList<DownloadItem*> items;
    for (int i = 0; i < ui->list->count(); i++) {
        DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
        if (!downItem) {
            continue;
        }
        if (downItem->isDownloading()) {
            continue;
        }
        items.append(downItem);
    }
    qDeleteAll(items);
}

void DownloadManager::download(const QNetworkRequest &request, const DownloadInfo &info)
{
    if (!info.page) {
        return;
    }

    // Clearing web page info from request
    QNetworkRequest req = request;
    req.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100), 0);

    handleUnsupportedContent(m_networkManager->get(req), info);
}

void DownloadManager::handleUnsupportedContent(QNetworkReply* reply, const DownloadInfo &info)
{
    if (!info.page || reply->url().scheme() == QLatin1String("qupzilla")) {
        return;
    }

//    if (fromPageDownload && m_useExternalManager) {
//        startExternalManager(reply->url());
//        reply->abort();
//        reply->deleteLater();
//        return;
//    }

    reply->setProperty("downReply", QVariant(true));

    DownloadFileHelper* h = new DownloadFileHelper(m_lastDownloadPath, m_downloadPath, m_useNativeDialog);
    connect(h, SIGNAL(itemCreated(QListWidgetItem*,DownloadItem*)), this, SLOT(itemCreated(QListWidgetItem*,DownloadItem*)));

    h->setLastDownloadOption(m_lastDownloadOption);
    h->setDownloadManager(this);
    h->setListWidget(ui->list);
    h->handleUnsupportedContent(reply, info);
}

void DownloadManager::itemCreated(QListWidgetItem* item, DownloadItem* downItem)
{
    connect(downItem, SIGNAL(deleteItem(DownloadItem*)), this, SLOT(deleteItem(DownloadItem*)));
    connect(downItem, SIGNAL(downloadFinished(bool)), this, SLOT(downloadFinished(bool)));

    ui->list->setItemWidget(item, downItem);
    item->setSizeHint(downItem->sizeHint());
    downItem->show();

    show();
    raise();
    activateWindow();
}

void DownloadManager::downloadFinished(bool success)
{
    bool downloadingAllFilesFinished = true;
    for (int i = 0; i < ui->list->count(); i++) {
        DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
        if (!downItem || (downItem && downItem->isCancelled()) || !downItem->isDownloading()) {
            continue;
        }
        downloadingAllFilesFinished = false;
    }

    if (downloadingAllFilesFinished) {
        if (success && qApp->activeWindow() != this) {
            mApp->desktopNotifications()->showNotification(QIcon::fromTheme(QSL("download"), QIcon(":icons/notifications/download.png")).pixmap(48), tr("Download Finished"), tr("All files have been successfully downloaded."));
            if (!m_closeOnFinish) {
                raise();
                activateWindow();
            }
        }
        ui->speedLabel->clear();
        setWindowTitle(tr("Download Manager"));
#ifdef W7TASKBAR
        if (QtWin::isRunningWindows7()) {
            win7.setProgressValue(0, 100);
            win7.setProgressState(win7.NoProgress);
        }
#endif
        if (m_closeOnFinish) {
            close();
        }
    }
}

void DownloadManager::deleteItem(DownloadItem* item)
{
    if (item && !item->isDownloading()) {
        delete item;
    }
}

bool DownloadManager::canClose()
{
    if (m_isClosing) {
        return true;
    }

    bool isDownloading = false;
    for (int i = 0; i < ui->list->count(); i++) {
        DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
        if (!downItem) {
            continue;
        }
        if (downItem->isDownloading()) {
            isDownloading = true;
            break;
        }
    }

    return !isDownloading;
}

bool DownloadManager::useExternalManager() const
{
    return m_useExternalManager;
}

void DownloadManager::closeEvent(QCloseEvent* e)
{
    if (mApp->windowCount() == 0) { // No main windows -> we are going to quit
        if (!canClose()) {
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
}
