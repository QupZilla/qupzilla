/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "qupzilla.h"
#include "downloaditem.h"
#include "downloadmanager.h"
#include "globalfunctions.h"
#include "settings.h"

#include <QFileIconProvider>
#include <QListWidgetItem>
#include <QTemporaryFile>
#include <QWebHistory>
#include <QDebug>
#include <QFileDialog>
#include <QDesktopServices>

DownloadFileHelper::DownloadFileHelper(const QString &lastDownloadPath, const QString &downloadPath, bool useNativeDialog, WebPage* page)
    : QObject()
    , m_lastDownloadOption(DownloadManager::SaveFile)
    , m_lastDownloadPath(lastDownloadPath)
    , m_downloadPath(downloadPath)
    , m_useNativeDialog(useNativeDialog)
    , m_timer(0)
    , m_reply(0)
    , m_openFileChoosed(false)
    , m_listWidget(0)
    , m_iconProvider(new QFileIconProvider)
    , m_manager(0)
    , m_webPage(page)
{
}

//////////////////////////////////////////////////////
//// Getting where to download requested file
//// in 3 functions, as we are using non blocking
//// dialogs ( this is important to make secure downloading
//// on Windows working properly )
//////////////////////////////////////////////////////

void DownloadFileHelper::handleUnsupportedContent(QNetworkReply* reply, bool askWhatToDo, const QString &suggestedFileName)
{
    m_timer = new QTime();
    m_timer->start();
    m_h_fileName = suggestedFileName.isEmpty() ? getFileName(reply) : suggestedFileName;
    m_reply = reply;

    QFileInfo info(m_h_fileName);
    QTemporaryFile tempFile("XXXXXX." + info.suffix());
    tempFile.open();
    tempFile.write(m_reply->peek(1024 * 1024));
    QFileInfo tempInfo(tempFile.fileName());
    m_fileIcon = m_iconProvider->icon(tempInfo).pixmap(30, 30);
    QString mimeType = m_iconProvider->type(tempInfo);

    m_fileSize = m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    if (m_fileSize > 0) {
        mimeType.append(QString(" (%1)").arg(DownloadItem::fileSizeToString(m_fileSize)));
    }

    // Close Empty Tab
    if (m_webPage) {
        WebView* view = qobject_cast<WebView*>(m_webPage->view());
        if (!m_webPage->url().isEmpty()) {
            m_downloadPage = m_webPage->url();
        }
        else if (m_webPage->history()->canGoBack()) {
            m_downloadPage = m_webPage->history()->backItem().url();
        }
        else if (view && m_webPage->history()->count() == 0) {
            view->closeView();
        }
    }

    if (askWhatToDo) {
        DownloadOptionsDialog* dialog = new DownloadOptionsDialog(m_h_fileName, m_fileIcon, mimeType, reply->url(), mApp->activeWindow());
        dialog->setLastDownloadOption(m_lastDownloadOption);
        dialog->show();
        connect(dialog, SIGNAL(dialogFinished(int)), this, SLOT(optionsDialogAccepted(int)));
    }
    else {
        optionsDialogAccepted(2);
    }
}

void DownloadFileHelper::optionsDialogAccepted(int finish)
{
    m_openFileChoosed = false;
    switch (finish) {
    case 0:  //Cancelled
        if (m_timer) {
            delete m_timer;
        }

        m_reply->abort();
        m_reply->deleteLater();

        return;
        break;
    case 1: //Open
        m_openFileChoosed = true;
        m_lastDownloadOption = DownloadManager::OpenFile;
        break;
    case 2: //Save
        m_lastDownloadOption = DownloadManager::SaveFile;
        break;

    default:
        qWarning() << "DownloadFileHelper::optionsDialogAccepted invalid return value!";
        if (m_timer) {
            delete m_timer;
        }

        m_reply->abort();
        m_reply->deleteLater();
        return;

        break;
    }

    m_manager->setLastDownloadOption(m_lastDownloadOption);

    if (!m_openFileChoosed) {
        if (m_downloadPath.isEmpty()) {
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
                urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
                     << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation))
                     << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation))
                     << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation))
                     << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::MusicLocation))
                     << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::MoviesLocation));
                dialog->setSidebarUrls(urls);

                dialog->show();
                connect(dialog, SIGNAL(fileSelected(QString)), this, SLOT(fileNameChoosed(QString)));
            }
        }
        else {
            fileNameChoosed(m_downloadPath + m_h_fileName, true);
        }
    }
    else {
        fileNameChoosed(QDir::tempPath() + "/" + m_h_fileName, true);
    }
}

void DownloadFileHelper::fileNameChoosed(const QString &name, bool fileNameAutoGenerated)
{
    m_userFileName = name;

    if (m_userFileName.isEmpty()) {
        m_reply->abort();
        m_reply->deleteLater();

        if (m_timer) {
            delete m_timer;
        }
        return;
    }


    int pos = m_userFileName.lastIndexOf("/");
    if (pos != -1) {
        int size = m_userFileName.size();
        m_path = m_userFileName.left(pos + 1);
        m_fileName = m_userFileName.right(size - pos - 1);
    }

    if (fileNameAutoGenerated) {
        m_fileName = qz_ensureUniqueFilename(m_fileName);
    }

    if (!m_path.contains(QDir::tempPath())) {
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
    QString path;
    if (reply->hasRawHeader("Content-Disposition")) {
        QString value = QString::fromLatin1(reply->rawHeader("Content-Disposition"));

        // We try to use UTF-8 encoded filename first if present
        if (value.contains(QRegExp("filename\\s*\\*\\s*=\\s*UTF-8", Qt::CaseInsensitive))) {
            QRegExp reg("filename\\s*\\*\\s*=\\s*UTF-8''([^;]*)", Qt::CaseInsensitive);
            reg.indexIn(value);
            path = QUrl::fromPercentEncoding(reg.cap(1).toUtf8()).trimmed();
        }
        else if (value.contains(QRegExp("filename\\s*=", Qt::CaseInsensitive))) {
            QRegExp reg("filename\\s*=([^;]*)", Qt::CaseInsensitive);
            reg.indexIn(value);
            path = reg.cap(1).trimmed();

            if (path.startsWith('"') && path.endsWith('"')) {
                path = path.mid(1, path.length() - 2);
            }
        }
    }

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
        endName.prepend(".");
    }

    QString name = baseName + endName;

    if (name.contains('"')) {
        name.remove("\";");
    }

    return qz_filterCharsFromFilename(name);
}

DownloadFileHelper::~DownloadFileHelper()
{
    delete m_iconProvider;
}
