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
#include "downloadfilehelper.h"
#include "webpage.h"
#include "tabbedwebview.h"
#include "downloadoptionsdialog.h"
#include "mainapplication.h"
#include "browserwindow.h"
#include "downloaditem.h"
#include "downloadmanager.h"
#include "qztools.h"
#include "datapaths.h"
#include "settings.h"
#include "qzregexp.h"

#include <QFileIconProvider>
#include <QListWidgetItem>
#include <QTemporaryFile>
#include <QWebHistory>
#include <QFileDialog>

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

DownloadFileHelper::DownloadFileHelper(const QString &lastDownloadPath, const QString &downloadPath, bool useNativeDialog)
    : QObject()
    , m_lastDownloadOption(DownloadManager::SaveFile)
    , m_lastDownloadPath(lastDownloadPath)
    , m_downloadPath(downloadPath)
    , m_useNativeDialog(useNativeDialog)
    , m_timer(0)
    , m_reply(0)
    , m_fileSize(0)
    , m_openFileChoosed(false)
    , m_listWidget(0)
    , m_iconProvider(new QFileIconProvider)
    , m_manager(0)
{
}

//////////////////////////////////////////////////////
//// Getting where to download requested file
//// in 3 functions, as we are using non blocking
//// dialogs ( this is important to make secure downloading
//// on Windows working properly )
//////////////////////////////////////////////////////

void DownloadFileHelper::handleUnsupportedContent(QNetworkReply* reply, const DownloadManager::DownloadInfo &info)
{
    m_timer = new QTime();
    m_timer->start();
    m_h_fileName = info.suggestedFileName.isEmpty() ? getFileName(reply) : info.suggestedFileName;
    m_reply = reply;

    QFileInfo fileInfo(m_h_fileName);
    QTemporaryFile tempFile(DataPaths::path(DataPaths::Temp) + "/XXXXXX." + fileInfo.suffix());
    tempFile.open();
    tempFile.write(m_reply->peek(1024 * 1024));
    QFileInfo tempInfo(tempFile.fileName());
    m_fileIcon = m_iconProvider->icon(tempInfo).pixmap(30, 30);
    QString mimeType = m_iconProvider->type(tempInfo);

    m_fileSize = m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    if (m_fileSize > 0) {
        mimeType.append(QString(" (%1)").arg(QzTools::fileSizeToString(m_fileSize)));
    }

    // Close Empty Tab
    if (info.page) {
        WebView* view = qobject_cast<WebView*>(info.page->view());
        if (!info.page->url().isEmpty()) {
            m_downloadPage = info.page->url();
        }
        else if (info.page->history()->canGoBack()) {
            m_downloadPage = info.page->history()->backItem().url();
        }
        else if (view && info.page->history()->count() == 0) {
            view->closeView();
        }
    }

    if (info.askWhatToDo && m_downloadPath.isEmpty()) {
        DownloadOptionsDialog* dialog = new DownloadOptionsDialog(m_h_fileName, m_fileIcon, mimeType, reply->url(), mApp->activeWindow());
        dialog->showExternalManagerOption(m_manager->useExternalManager());
        dialog->setLastDownloadOption(m_lastDownloadOption);
        dialog->show();

        connect(dialog, SIGNAL(dialogFinished(int)), this, SLOT(optionsDialogAccepted(int)));
    }
    else if (info.forceChoosingPath) {
        optionsDialogAccepted(4);
    }
    else if (m_manager->useExternalManager()) {
        optionsDialogAccepted(3);
    }
    else {
        optionsDialogAccepted(2);
    }
}

QString DownloadFileHelper::parseContentDisposition(const QByteArray &header)
{
    QString path;

    if (header.isEmpty()) {
        return path;
    }

    QString value;

    if (QzTools::isUtf8(header.constData())) {
        value = QString::fromUtf8(header);
    }
    else {
        value = QString::fromLatin1(header);
    }

    // We try to use UTF-8 encoded filename first if present
    if (value.contains(QzRegExp("[ ;]{1,}filename*\\*\\s*=\\s*UTF-8''", Qt::CaseInsensitive))) {
        QzRegExp reg("filename\\s*\\*\\s*=\\s*UTF-8''([^;]*)", Qt::CaseInsensitive);
        reg.indexIn(value);
        path = QUrl::fromPercentEncoding(reg.cap(1).toUtf8()).trimmed();
    }
    else if (value.contains(QzRegExp("[ ;]{1,}filename\\s*=", Qt::CaseInsensitive))) {
        QzRegExp reg("[ ;]{1,}filename\\s*=(.*)", Qt::CaseInsensitive);
        reg.indexIn(value);
        path = reg.cap(1).trimmed();

        // Parse filename in quotes (to support semicolon inside filename)
        if (path.startsWith(QLatin1Char('"')) && path.count(QLatin1Char('"')) > 1) {
            int pos = path.indexOf(QLatin1Char('"'), 1);
            while (pos != -1) {
                if (path[pos - 1] != QLatin1Char('\\')) {
                    // We also need to strip starting quote
                    path = path.left(pos).mid(1);
                    break;
                }
                pos = path.indexOf(QLatin1Char('"'), pos + 1);
            }
        }
        else {
            QzRegExp reg("([^;]*)", Qt::CaseInsensitive);
            reg.indexIn(path);
            path = reg.cap(1).trimmed();
        }

        if (path.startsWith(QLatin1Char('"')) && path.endsWith(QLatin1Char('"'))) {
            path = path.mid(1, path.length() - 2);
        }
    }

    return path;
}

void DownloadFileHelper::optionsDialogAccepted(int finish)
{
    bool forceChoosingPath = false;
    m_openFileChoosed = false;

    switch (finish) {
    case 0:  // Cancelled
        delete m_timer;

        m_reply->abort();
        m_reply->deleteLater();

        return;

    case 1: // Open
        m_openFileChoosed = true;
        m_lastDownloadOption = DownloadManager::OpenFile;
        break;

    case 2: // Save
        m_lastDownloadOption = DownloadManager::SaveFile;
        break;

    case 3: // External manager
        m_manager->startExternalManager(m_reply->url());
        m_reply->abort();
        m_reply->deleteLater();
        return;

    case 4: // Force opening save file dialog
        m_lastDownloadOption = DownloadManager::SaveFile;
        forceChoosingPath = true;
        break;

    default:
        qWarning() << "DownloadFileHelper::optionsDialogAccepted invalid return value!";
        delete m_timer;

        m_reply->abort();
        m_reply->deleteLater();
        return;
    }

    m_manager->setLastDownloadOption(m_lastDownloadOption);

    if (!m_openFileChoosed) {
        if (m_downloadPath.isEmpty() || forceChoosingPath) {
            if (m_useNativeDialog) {
                fileNameChoosed(QFileDialog::getSaveFileName(mApp->getWindow(), tr("Save file as..."), m_lastDownloadPath + m_h_fileName));
            }
            else {
                QFileDialog* dialog = new QFileDialog(mApp->getWindow());
                dialog->setAttribute(Qt::WA_DeleteOnClose);
                dialog->setWindowTitle(tr("Save file as..."));
                dialog->setAcceptMode(QFileDialog::AcceptSave);
                dialog->setDirectory(m_lastDownloadPath);
                dialog->selectFile(m_h_fileName);

                QList<QUrl> urls;
                urls <<
#if QT_VERSION >= 0x050000
                     QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
                     << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                     << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
                     << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
                     << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::MusicLocation))
                     << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
#else
                     QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
                     << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation))
                     << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation))
                     << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation))
                     << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::MusicLocation))
                     << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::MoviesLocation));
#endif
                dialog->setSidebarUrls(urls);

                dialog->open();
                connect(dialog, SIGNAL(fileSelected(QString)), this, SLOT(fileNameChoosed(QString)));
            }
        }
        else {
            fileNameChoosed(m_downloadPath + QLatin1Char('/') + m_h_fileName, true);
        }
    }
    else {
        fileNameChoosed(DataPaths::path(DataPaths::Temp) + QLatin1Char('/') + m_h_fileName, true);
    }
}

void DownloadFileHelper::fileNameChoosed(const QString &name, bool fileNameAutoGenerated)
{
    m_userFileName = name.trimmed();

    if (m_userFileName.isEmpty()) {
        m_reply->abort();
        m_reply->deleteLater();

        delete m_timer;
        return;
    }


    int pos = m_userFileName.lastIndexOf(QLatin1Char('/'));
    if (pos != -1) {
        int size = m_userFileName.size();
        m_path = m_userFileName.left(pos + 1);
        m_fileName = m_userFileName.right(size - pos - 1);
    }

    if (fileNameAutoGenerated) {
        m_fileName = QzTools::ensureUniqueFilename(m_fileName);
    }

    if (!m_path.contains(DataPaths::path(DataPaths::Temp))) {
        m_lastDownloadPath = m_path;
    }

    Settings settings;
    settings.beginGroup("DownloadManager");
    settings.setValue("lastDownloadPath", m_lastDownloadPath);
    settings.endGroup();
    m_manager->setLastDownloadPath(m_lastDownloadPath);

    QListWidgetItem* item = new QListWidgetItem(m_listWidget);
    DownloadItem* downItem = new DownloadItem(item, m_reply, m_path, m_fileName, m_fileIcon, m_timer, m_openFileChoosed, m_downloadPage, m_manager);
    downItem->setTotalSize(m_fileSize);

    emit itemCreated(item, downItem);
}

//////////////////////////////////////////////////////
//// End here
//////////////////////////////////////////////////////

QString DownloadFileHelper::getFileName(QNetworkReply* reply)
{
    QString path = parseContentDisposition(reply->rawHeader("Content-Disposition"));

    if (path.isEmpty()) {
        path = reply->url().path();
    }

    QFileInfo info(path);
    QString baseName = info.completeBaseName();
    QString endName = info.suffix();

    if (baseName.isEmpty()) {
        baseName = tr("NoNameDownload");
    }

    if (!endName.isEmpty()) {
        endName.prepend(QLatin1Char('.'));
    }

    QString name = baseName + endName;

    if (name.contains(QLatin1Char('"'))) {
        name.remove(QLatin1String("\";"));
    }

    return QzTools::filterCharsFromFilename(name);
}

DownloadFileHelper::~DownloadFileHelper()
{
    delete m_iconProvider;
}
