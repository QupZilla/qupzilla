/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "networkmanager.h"
#include "desktopnotificationsfactory.h"
#include "qztools.h"
#include "webpage.h"
#include "webview.h"
#include "settings.h"
#include "datapaths.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "tabbar.h"
#include "locationbar.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QDir>
#include <QShortcut>
#include <QStandardPaths>
#include <QWebEngineHistory>
#include <QWebEngineDownloadItem>

#ifdef Q_OS_WIN
#include <QtWin>
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

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
        QtWin::extendFrameIntoClientArea(this, -1, -1, -1, -1);
    }
#endif
    ui->clearButton->setIcon(QIcon::fromTheme("edit-clear"));
    QzTools::centerWidgetOnScreen(this);

    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearList()));

    QShortcut* clearShortcut = new QShortcut(QKeySequence("CTRL+L"), this);
    connect(clearShortcut, SIGNAL(activated()), this, SLOT(clearList()));

    loadSettings();

    QzTools::setWmClass("Download Manager", this);
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

void DownloadManager::closeDownloadTab(const QUrl &url) const
{
    // Attempt to close empty tab that was opened only for loading the download url
    auto testWebView = [](TabbedWebView *view, const QUrl &url) {
        if (view->browserWindow()->tabWidget()->tabBar()->normalTabsCount() < 2) {
            return false;
        }
        WebPage *page = view->page();
        if (page->history()->count() != 0) {
            return false;
        }
        if (page->url() != QUrl()) {
            return false;
        }
        QUrl tabUrl = page->requestedUrl();
        if (tabUrl.isEmpty()) {
            tabUrl = QUrl(view->webTab()->locationBar()->text());
        }
        return tabUrl.host() == url.host();
    };

    if (testWebView(mApp->getWindow()->weView(), url)) {
        mApp->getWindow()->weView()->closeView();
        return;
    }

    const auto windows = mApp->windows();
    for (auto *window : windows) {
        const auto tabs = window->tabWidget()->allTabs();
        for (auto *tab : tabs) {
            auto *view = tab->webView();
            if (testWebView(view, url)) {
                view->closeView();
                return;
            }
        }
    }
}

void DownloadManager::startExternalManager(const QUrl &url)
{
    QString arguments = m_externalArguments;
    arguments.replace(QLatin1String("%d"), url.toEncoded());

    QzTools::startExternalProcess(m_externalExecutable, arguments);
    m_lastDownloadOption = ExternalManager;
}

void DownloadManager::timerEvent(QTimerEvent* e)
{
    QVector<QTime> remTimes;
    QVector<int> progresses;
    QVector<double> speeds;

    if (e->timerId() == m_timer.timerId()) {
        if (!ui->list->count()) {
            ui->speedLabel->clear();
            setWindowTitle(tr("Download Manager"));
#ifdef Q_OS_WIN
            if (m_taskbarButton) {
                m_taskbarButton->progress()->hide();
            }
#endif
            return;
        }
        for (int i = 0; i < ui->list->count(); i++) {
            DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
            if (!downItem || downItem->isCancelled() || !downItem->isDownloading()) {
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

#ifndef Q_OS_WIN
        ui->speedLabel->setText(tr("%1% of %2 files (%3) %4 remaining").arg(QString::number(progress), QString::number(progresses.count()),
                                DownloadItem::currentSpeedToString(speed),
                                DownloadItem::remaingTimeToString(remaining)));
#endif
        setWindowTitle(tr("%1% - Download Manager").arg(progress));
#ifdef Q_OS_WIN
        if (m_taskbarButton) {
            m_taskbarButton->progress()->show();
            m_taskbarButton->progress()->setValue(progress);
        }
#endif
    }

    QWidget::timerEvent(e);
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
    emit downloadsCountChanged();
}

void DownloadManager::download(QWebEngineDownloadItem *downloadItem)
{
    closeDownloadTab(downloadItem->url());

    QString downloadPath;
    bool openFile = false;

    QString fileName = QFileInfo(downloadItem->path()).fileName();
    fileName = QUrl::fromPercentEncoding(fileName.toUtf8());
    // Filename may have been percent encoded and actually containing path
    fileName = QFileInfo(fileName).fileName();

    const bool forceAsk = downloadItem->savePageFormat() != QWebEngineDownloadItem::UnknownSaveFormat
            || downloadItem->type() == QWebEngineDownloadItem::UserRequested;

    if (m_useExternalManager) {
        startExternalManager(downloadItem->url());
    } else if (forceAsk || m_downloadPath.isEmpty()) {
        enum Result { Open = 1, Save = 2, ExternalManager = 3, SavePage = 4, Unknown = 0 };
        Result result = Unknown;

        if (downloadItem->savePageFormat() != QWebEngineDownloadItem::UnknownSaveFormat) {
            // Save Page requested
            result = SavePage;
        } else if (downloadItem->type() == QWebEngineDownloadItem::UserRequested) {
            // Save x as... requested
            result = Save;
        } else {
            // Ask what to do
            DownloadOptionsDialog optionsDialog(fileName, downloadItem, mApp->activeWindow());
            optionsDialog.showExternalManagerOption(m_useExternalManager);
            optionsDialog.setLastDownloadOption(m_lastDownloadOption);
            result = Result(optionsDialog.exec());
        }

        switch (result) {
        case Open:
            openFile = true;
            downloadPath = QzTools::ensureUniqueFilename(DataPaths::path(DataPaths::Temp) + QLatin1Char('/') + fileName);
            m_lastDownloadOption = OpenFile;
            break;

        case Save:
            downloadPath = QFileDialog::getSaveFileName(mApp->activeWindow(), tr("Save file as..."), m_lastDownloadPath + QLatin1Char('/') + fileName);

            if (!downloadPath.isEmpty()) {
                m_lastDownloadPath = QFileInfo(downloadPath).absolutePath();
                Settings().setValue(QSL("DownloadManager/lastDownloadPath"), m_lastDownloadPath);
                m_lastDownloadOption = SaveFile;
            }
            break;

        case SavePage: {
            const QString mhtml = tr("MIME HTML Archive (*.mhtml)");
            const QString htmlSingle = tr("HTML Page, single (*.html)");
            const QString htmlComplete = tr("HTML Page, complete (*.html)");
            const QString filter = QStringLiteral("%1;;%2;;%3").arg(mhtml, htmlSingle, htmlComplete);

            QString selectedFilter;
            downloadPath = QFileDialog::getSaveFileName(mApp->activeWindow(), tr("Save page as..."),
                                                        m_lastDownloadPath + QLatin1Char('/') + fileName,
                                                        filter, &selectedFilter);

            if (!downloadPath.isEmpty()) {
                m_lastDownloadPath = QFileInfo(downloadPath).absolutePath();
                Settings().setValue(QSL("DownloadManager/lastDownloadPath"), m_lastDownloadPath);
                m_lastDownloadOption = SaveFile;

                QWebEngineDownloadItem::SavePageFormat format = QWebEngineDownloadItem::UnknownSaveFormat;

                if (selectedFilter == mhtml) {
                    format = QWebEngineDownloadItem::MimeHtmlSaveFormat;
                } else if (selectedFilter == htmlSingle) {
                    format = QWebEngineDownloadItem::SingleHtmlSaveFormat;
                } else if (selectedFilter == htmlComplete) {
                    format = QWebEngineDownloadItem::CompleteHtmlSaveFormat;
                }

                if (format != QWebEngineDownloadItem::UnknownSaveFormat) {
                    downloadItem->setSavePageFormat(format);
                }
            }
            break;
        }

        case ExternalManager:
            startExternalManager(downloadItem->url());
            // fallthrough

        default:
            downloadItem->cancel();
            return;
        }
    } else {
        downloadPath = QzTools::ensureUniqueFilename(m_downloadPath + QL1C('/') + fileName);
    }

    if (downloadPath.isEmpty()) {
        downloadItem->cancel();
        return;
    }

    // Set download path and accept
    downloadItem->setPath(downloadPath);
    downloadItem->accept();

    // Create download item
    QListWidgetItem* listItem = new QListWidgetItem(ui->list);
    DownloadItem* downItem = new DownloadItem(listItem, downloadItem, QFileInfo(downloadPath).absolutePath(), QFileInfo(downloadPath).fileName(), openFile, this);
    connect(downItem, SIGNAL(deleteItem(DownloadItem*)), this, SLOT(deleteItem(DownloadItem*)));
    connect(downItem, SIGNAL(downloadFinished(bool)), this, SLOT(downloadFinished(bool)));
    ui->list->setItemWidget(listItem, downItem);
    listItem->setSizeHint(downItem->sizeHint());
    downItem->show();

    m_activeDownloadsCount++;
    emit downloadsCountChanged();
}

int DownloadManager::downloadsCount() const
{
    return ui->list->count();
}

int DownloadManager::activeDownloadsCount() const
{
    return m_activeDownloadsCount;
}

void DownloadManager::downloadFinished(bool success)
{
    m_activeDownloadsCount = 0;
    bool downloadingAllFilesFinished = true;
    for (int i = 0; i < ui->list->count(); i++) {
        DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
        if (!downItem) {
            continue;
        }
        if (downItem->isDownloading()) {
            m_activeDownloadsCount++;
        }
        if (downItem->isCancelled() || !downItem->isDownloading()) {
            continue;
        }
        downloadingAllFilesFinished = false;
    }

    emit downloadsCountChanged();

    if (downloadingAllFilesFinished) {
        if (success && qApp->activeWindow() != this) {
            mApp->desktopNotifications()->showNotification(QIcon::fromTheme(QSL("download"), QIcon(QSL(":icons/other/download.svg"))).pixmap(48), tr("QupZilla: Download Finished"), tr("All files have been successfully downloaded."));
            if (!m_closeOnFinish) {
                raise();
                activateWindow();
            }
        }
        ui->speedLabel->clear();
        setWindowTitle(tr("Download Manager"));
#ifdef Q_OS_WIN
        if (m_taskbarButton) {
            m_taskbarButton->progress()->hide();
        }
#endif
        if (m_closeOnFinish) {
            close();
        }
    }
}

void DownloadManager::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
#ifdef Q_OS_WIN
    if (!m_taskbarButton) {
        m_taskbarButton = new QWinTaskbarButton(this);
    }
    m_taskbarButton->setWindow(windowHandle());
    m_taskbarButton->progress()->setRange(0, 100);
#endif
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
                                                 tr("Are you sure you want to quit? All uncompleted downloads will be cancelled!"), QMessageBox::Yes | QMessageBox::No);
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

