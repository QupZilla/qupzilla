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
#include "downloaditem.h"
#include "ui_downloaditem.h"
#include "mainapplication.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include "webpage.h"
#include "downloadmanager.h"
#include "networkmanager.h"
#include "qztools.h"
#include "schemehandlers/ftpschemehandler.h"

#include <QMenu>
#include <QClipboard>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QTimer>
#include <QFileInfo>
#include <QMessageBox>
#include <QDesktopServices>

#ifdef Q_OS_WIN
#include "Shlwapi.h"
#endif

//#define DOWNMANAGER_DEBUG

DownloadItem::DownloadItem(QListWidgetItem* item, QNetworkReply* reply, const QString &path, const QString &fileName, const QPixmap &fileIcon, QTime* timer, bool openAfterFinishedDownload, const QUrl &downloadPage, DownloadManager* manager)
    : QWidget()
    , ui(new Ui::DownloadItem)
    , m_item(item)
    , m_reply(reply)
    , m_ftpDownloader(0)
    , m_path(path)
    , m_fileName(fileName)
    , m_downTimer(timer)
    , m_downUrl(reply->url())
    , m_downloadPage(downloadPage)
    , m_downloading(false)
    , m_openAfterFinish(openAfterFinishedDownload)
    , m_downloadStopped(false)
    , m_received(0)
    , m_total(0)
{
#ifdef DOWNMANAGER_DEBUG
    qDebug() << __FUNCTION__ << item << reply << path << fileName;
#endif
    QString fullPath = path + fileName;
    if (QFile::exists(fullPath)) {
        QFile::remove(fullPath);
    }

    m_outputFile.setFileName(fullPath);

    ui->setupUi(this);
    setMaximumWidth(525);

    ui->button->setPixmap(QIcon::fromTheme(QSL("process-stop")).pixmap(20, 20));
    ui->fileName->setText(m_fileName);
    ui->downloadInfo->setText(tr("Remaining time unavailable"));
    ui->fileIcon->setPixmap(fileIcon);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));
    connect(ui->button, SIGNAL(clicked(QPoint)), this, SLOT(stop()));
    connect(manager, SIGNAL(resized(QSize)), this, SLOT(parentResized(QSize)));

    startDownloading();
}

void DownloadItem::setTotalSize(qint64 total)
{
    if (total > 0) {
        m_total = total;
    }
}

void DownloadItem::startDownloading()
{
    QUrl locationHeader = m_reply->header(QNetworkRequest::LocationHeader).toUrl();

    bool hasFtpUrlInHeader = locationHeader.isValid() && (locationHeader.scheme() == "ftp");
    if (m_reply->url().scheme() == "ftp" || hasFtpUrlInHeader) {
        QUrl url = hasFtpUrlInHeader ? locationHeader : m_reply->url();
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = 0;

        startDownloadingFromFtp(url);
        return;
    }
    else if (locationHeader.isValid()) {
        m_reply->abort();
        m_reply->deleteLater();

        m_reply = mApp->networkManager()->get(QNetworkRequest(locationHeader));
    }

    m_reply->setParent(this);
    connect(m_reply, SIGNAL(finished()), this, SLOT(finished()));
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error()));
    connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));

    m_downloading = true;
    m_timer.start(1000, this);
    readyRead();
    QTimer::singleShot(200, this, SLOT(updateDownload()));

    if (m_reply->error() != QNetworkReply::NoError) {
        stop(false);
        error();
    }
}

void DownloadItem::startDownloadingFromFtp(const QUrl &url)
{
    if (!m_outputFile.isOpen() && !m_outputFile.open(QIODevice::WriteOnly)) {
        stop(false);
        ui->downloadInfo->setText(tr("Error: Cannot write to file!"));
        return;
    }

    m_ftpDownloader = new FtpDownloader(this);
    connect(m_ftpDownloader, SIGNAL(finished()), this, SLOT(finished()));
    connect(m_ftpDownloader, SIGNAL(dataTransferProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    connect(m_ftpDownloader, SIGNAL(errorOccured(QFtp::Error)), this, SLOT(error()));
    connect(m_ftpDownloader, SIGNAL(ftpAuthenticationRequierd(QUrl,QAuthenticator*)), mApp->networkManager(), SLOT(ftpAuthentication(QUrl,QAuthenticator*)));

    m_ftpDownloader->download(url, &m_outputFile);
    m_downloading = true;
    m_timer.start(1000, this);

    QTimer::singleShot(200, this, SLOT(updateDownload()));

    if (m_ftpDownloader->error() != QFtp::NoError) {
        error();
    }
}

void DownloadItem::parentResized(const QSize &size)
{
    if (size.width() < 200) {
        return;
    }
    setMaximumWidth(size.width());
}

void DownloadItem::metaDataChanged()
{
    QUrl locationHeader = m_reply->header(QNetworkRequest::LocationHeader).toUrl();
    if (locationHeader.isValid()) {
        m_reply->close();
        m_reply->deleteLater();

        m_reply = mApp->networkManager()->get(QNetworkRequest(locationHeader));
        startDownloading();
    }
}

void DownloadItem::finished()
{
#ifdef DOWNMANAGER_DEBUG
    qDebug() << __FUNCTION__ << m_reply;
#endif
    m_timer.stop();

    QString host = m_reply ? m_reply->url().host() : m_ftpDownloader->url().host();
    ui->downloadInfo->setText(tr("Done - %1 (%2)").arg(host, QDateTime::currentDateTime().toString(Qt::DefaultLocaleShortDate)));
    ui->progressBar->hide();
    ui->button->hide();
    ui->frame->hide();
    m_outputFile.close();

    if (m_reply) {
        m_reply->deleteLater();
    }
    else {
        m_ftpDownloader->deleteLater();
    }

    m_item->setSizeHint(sizeHint());
#if QT_VERSION == 0x040700 // Workaround
    ui->button->show();
    ui->button->hide();
#endif
    m_downloading = false;

    if (m_openAfterFinish) {
        openFile();
    }

    emit downloadFinished(true);
}

void DownloadItem::downloadProgress(qint64 received, qint64 total)
{
#ifdef DOWNMANAGER_DEBUG
    qDebug() << __FUNCTION__ << received << total;
#endif
    qint64 currentValue = 0;
    qint64 totalValue = 0;
    if (total > 0) {
        currentValue = received * 100 / total;
        totalValue = 100;
    }
    ui->progressBar->setValue(currentValue);
    ui->progressBar->setMaximum(totalValue);
    m_currSpeed = received * 1000.0 / m_downTimer->elapsed();
    m_received = received;
    m_total = total;
}

void DownloadItem::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_timer.timerId()) {
        updateDownloadInfo(m_currSpeed, m_received, m_total);
    }
    else {
        QWidget::timerEvent(event);
    }
}

int DownloadItem::progress()
{
    return ui->progressBar->value();
}

bool DownloadItem::isCancelled()
{
    return ui->downloadInfo->text().startsWith(tr("Cancelled"));
}

QString DownloadItem::remaingTimeToString(QTime time)
{
    if (time < QTime(0, 0, 10)) {
        return tr("few seconds");
    }
    else if (time < QTime(0, 1)) {
        return tr("%n seconds", "", time.second());
    }
    else if (time < QTime(1, 0)) {
        return tr("%n minutes", "", time.minute());
    }
    else {
        return tr("%n hours", "", time.hour());
    }
}

QString DownloadItem::currentSpeedToString(double speed)
{
    if (speed < 0) {
        return tr("Unknown speed");
    }

    speed /= 1024; // kB
    if (speed < 1000) {
        return QString::number(speed, 'f', 0) + QLatin1String(" ") + tr("kB/s");
    }

    speed /= 1024; //MB
    if (speed < 1000) {
        return QString::number(speed, 'f', 2) + QLatin1String(" ") + tr("MB/s");
    }

    speed /= 1024; //GB
    return QString::number(speed, 'f', 2) + QLatin1String(" ") + tr("GB/s");
}

void DownloadItem::updateDownloadInfo(double currSpeed, qint64 received, qint64 total)
{
#ifdef DOWNMANAGER_DEBUG
    qDebug() << __FUNCTION__ << currSpeed << received << total;
#endif
    //            QString          QString       QString     QString
    //          | m_remTime |   |m_currSize|  |m_fileSize|  |m_speed|
    // Remaining 26 minutes -     339MB of      693 MB        (350kB/s)

    int estimatedTime = ((total - received) / 1024) / (currSpeed / 1024);
    QString speed = currentSpeedToString(currSpeed);
    // We have QString speed now

    QTime time;
    time = time.addSecs(estimatedTime);
    QString remTime = remaingTimeToString(time);
    m_remTime = time;

    QString currSize = QzTools::fileSizeToString(received);
    QString fileSize = QzTools::fileSizeToString(total);

    if (fileSize == tr("Unknown size")) {
        ui->downloadInfo->setText(tr("%2 - unknown size (%3)").arg(currSize, speed));
    }
    else {
        ui->downloadInfo->setText(tr("Remaining %1 - %2 of %3 (%4)").arg(remTime, currSize, fileSize, speed));
    }
}

void DownloadItem::stop(bool askForDeleteFile)
{
#ifdef DOWNMANAGER_DEBUG
    qDebug() << __FUNCTION__;
#endif
    if (m_downloadStopped) {
        return;
    }
    m_downloadStopped = true;
    QString host;
    if (m_reply) {
        host = m_reply->url().host();
    }
    else if (m_ftpDownloader) {
        host = m_ftpDownloader->url().host();
    }
    m_openAfterFinish = false;
    m_timer.stop();
    if (m_reply) {
        m_reply->abort();
    }
    else if (m_ftpDownloader) {
        m_ftpDownloader->abort();
        m_ftpDownloader->close();
    }
    QString outputfile = QFileInfo(m_outputFile).absoluteFilePath();
    m_outputFile.close();
    ui->downloadInfo->setText(tr("Cancelled - %1").arg(host));
    ui->progressBar->hide();
    ui->button->hide();
    m_item->setSizeHint(sizeHint());

#if QT_VERSION == 0x040700 // Workaround
    ui->button->show();
    ui->button->hide();
#endif
    m_downloading = false;

    emit downloadFinished(false);

    if (askForDeleteFile) {
        QMessageBox::StandardButton button = QMessageBox::question(m_item->listWidget()->parentWidget(), tr("Delete file"), tr("Do you want to also delete downloaded file?"), QMessageBox::Yes | QMessageBox::No);
        if (button == QMessageBox::Yes) {
            QFile::remove(outputfile);
        }
    }
}

void DownloadItem::mouseDoubleClickEvent(QMouseEvent* e)
{
    openFile();
    e->accept();
}

void DownloadItem::customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    menu.addAction(QIcon::fromTheme("document-open"), tr("Open File"), this, SLOT(openFile()));

    menu.addAction(tr("Open Folder"), this, SLOT(openFolder()));
    menu.addSeparator();
    menu.addAction(tr("Go to Download Page"), this, SLOT(goToDownloadPage()))->setEnabled(!m_downloadPage.isEmpty());
    menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy Download Link"), this, SLOT(copyDownloadLink()));
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme("process-stop"), tr("Cancel downloading"), this, SLOT(stop()))->setEnabled(m_downloading);
    menu.addAction(QIcon::fromTheme("list-remove"), tr("Remove From List"), this, SLOT(clear()))->setEnabled(!m_downloading);

    if (m_downloading || ui->downloadInfo->text().startsWith(tr("Cancelled")) || ui->downloadInfo->text().startsWith(tr("Error"))) {
        menu.actions().at(0)->setEnabled(false);
    }
    menu.exec(mapToGlobal(pos));
}

void DownloadItem::goToDownloadPage()
{
    BrowserWindow* qz = mApp->getWindow();

    if (qz) {
        qz->tabWidget()->addView(m_downloadPage, Qz::NT_SelectedTab);
    }
    else {
        mApp->createWindow(Qz::BW_NewWindow, m_downloadPage);
    }
}

void DownloadItem::copyDownloadLink()
{
    QApplication::clipboard()->setText(m_downUrl.toString());
}

void DownloadItem::clear()
{
    emit deleteItem(this);
}

void DownloadItem::openFile()
{
    if (m_downloading) {
        return;
    }
    QFileInfo info(m_path + m_fileName);
    if (info.exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()));
    }
    else {
        QMessageBox::warning(m_item->listWidget()->parentWidget(), tr("Not found"), tr("Sorry, the file \n %1 \n was not found!").arg(info.absoluteFilePath()));
    }
}

void DownloadItem::openFolder()
{
#ifdef Q_OS_WIN
    QString winFileName = m_path + m_fileName;
    winFileName.replace(QLatin1Char('/'), "\\");
    QString shExArg = "/e,/select,\"" + winFileName + "\"";
    ShellExecute(NULL, NULL, TEXT("explorer.exe"), shExArg.toStdWString().c_str(), NULL, SW_SHOW);
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_path));
#endif
}

void DownloadItem::readyRead()
{
#ifdef DOWNMANAGER_DEBUG
    qDebug() << __FUNCTION__ ;
#endif
    if (!m_outputFile.isOpen() && !m_outputFile.open(QIODevice::WriteOnly)) {
        stop(false);
        ui->downloadInfo->setText(tr("Error: Cannot write to file!"));
        return;
    }
    m_outputFile.write(m_reply->readAll());
}

void DownloadItem::error()
{
#ifdef DOWNMANAGER_DEBUG
    qDebug() << __FUNCTION__ << (m_reply ? m_reply->error() : m_ftpDownloader->error());
#endif
    if (m_reply && m_reply->error() != QNetworkReply::NoError) {
        ui->downloadInfo->setText(tr("Error: ") + m_reply->errorString());
    }
    else if (m_ftpDownloader && m_ftpDownloader->error() != QFtp::NoError) {
        stop(false);
        ui->downloadInfo->setText(tr("Error: ") + m_ftpDownloader->errorString());
    }
}

void DownloadItem::updateDownload()
{
#ifdef DOWNMANAGER_DEBUG
    qDebug() << __FUNCTION__ ;
#endif
    // after caling stop() (from readyRead()) m_reply will be a dangling pointer,
    // thus it should be checked after m_outputFile.isOpen()
    if (ui->progressBar->maximum() == 0 && m_outputFile.isOpen() &&
        ((m_reply && m_reply->isFinished()) ||
         (m_ftpDownloader && m_ftpDownloader->isFinished())
        )
       ) {
        downloadProgress(0, 0);
        finished();
    }
}

DownloadItem::~DownloadItem()
{
    delete ui;
    delete m_item;
    delete m_downTimer;
}
