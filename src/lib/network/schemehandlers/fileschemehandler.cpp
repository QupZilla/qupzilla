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
#include "fileschemehandler.h"
#include "globalfunctions.h"
#include "iconprovider.h"
#include "downloadoptionsdialog.h"
#include "mainapplication.h"
#include "qupzilla.h"

#include <QFileIconProvider>
#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>
#include <QDir>

FileSchemeHandler::FileSchemeHandler()
{
}

QNetworkReply* FileSchemeHandler::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
{
    Q_UNUSED(outgoingData)

    if (op != QNetworkAccessManager::GetOperation) {
        return 0;
    }

    // Only list directories
    QFileInfo fileInfo(request.url().toLocalFile());
    if (!fileInfo.isDir() || !fileInfo.isReadable() || !fileInfo.exists()) {
        return 0;
    }

    FileSchemeReply* reply = new FileSchemeReply(request);
    return reply;
}

void FileSchemeHandler::handleUrl(const QUrl &url)
{
    QFileIconProvider iconProvider;
    QFile file(url.toLocalFile());
    QFileInfo info(file);

    if (!info.exists() || info.isDir() || !info.isReadable()) {
        return;
    }

    const QString &fileName = info.fileName();
    const QPixmap &pixmap = iconProvider.icon(info).pixmap(30);
    const QString &type = iconProvider.type(info);

    DownloadOptionsDialog dialog(fileName, pixmap, type, url, mApp->getWindow());
    dialog.showExternalManagerOption(false);
    dialog.showFromLine(false);

    int status = dialog.exec();

    if (status == 1) {
        // Open
        QDesktopServices::openUrl(url);
    }
    else if (status == 2) {
        // Save
        const QString &savePath = QFileDialog::getSaveFileName(mApp->getWindow(),
                                  QObject::tr("Save file as..."),
                                  QDir::homePath() + "/" + qz_getFileNameFromUrl(url));

        if (!savePath.isEmpty()) {
            file.copy(savePath);
        }
    }
}

FileSchemeReply::FileSchemeReply(const QNetworkRequest &req, QObject* parent)
    : QNetworkReply(parent)
{
    setOperation(QNetworkAccessManager::GetOperation);
    setRequest(req);
    setUrl(req.url());

    m_buffer.open(QIODevice::ReadWrite);
    setError(QNetworkReply::NoError, tr("No Error"));
    open(QIODevice::ReadOnly);

    QTimer::singleShot(0, this, SLOT(loadPage()));
}

qint64 FileSchemeReply::bytesAvailable() const
{
    return m_buffer.bytesAvailable() + QNetworkReply::bytesAvailable();
}

qint64 FileSchemeReply::readData(char* data, qint64 maxSize)
{
    return m_buffer.read(data, maxSize);
}

void FileSchemeReply::loadPage()
{
    QTextStream stream(&m_buffer);
    stream.setCodec("UTF-8");

    stream << loadDirectory();

    stream.flush();
    m_buffer.reset();

    setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("text/html"));
    setHeader(QNetworkRequest::ContentLengthHeader, m_buffer.bytesAvailable());
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
    setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QByteArray("Ok"));
    emit metaDataChanged();
    emit downloadProgress(m_buffer.size(), m_buffer.size());

    emit readyRead();
    emit finished();
}

QString FileSchemeReply::loadDirectory()
{
    const QDir &dir = QDir(request().url().toLocalFile());
    const QFileInfoList &list = dir.entryInfoList(QDir::AllEntries | QDir::Hidden, QDir::Name | QDir::DirsFirst);

    static QString sPage;

    if (sPage.isEmpty()) {
        sPage = qz_readAllFileContents(":/html/dirlist.html");
        sPage.replace(QLatin1String("%BOX-BORDER%"), QLatin1String("qrc:html/box-border.png"));
        sPage.replace(QLatin1String("%UP-IMG%"), qz_pixmapToByteArray(qIconProvider->standardIcon(QStyle::SP_FileDialogToParent).pixmap(22)));
        sPage.replace(QLatin1String("%UP-DIR-TEXT%"), tr("Up to higher level directory"));
        sPage.replace(QLatin1String("%SHOW-HIDDEN-TEXT%"), tr("Show hidden files"));
        sPage.replace(QLatin1String("%NAME%"), tr("Name"));
        sPage.replace(QLatin1String("%SIZE%"), tr("Size"));
        sPage.replace(QLatin1String("%MODIFIED%"), tr("Last modified"));
        sPage = qz_applyDirectionToPage(sPage);
    }

    QString page = sPage;
    page.replace(QLatin1String("%TITLE%"), tr("Index for %1").arg(request().url().toLocalFile()));

    QString upDirDisplay = QLatin1String("none");
    QString showHiddenDisplay = QLatin1String("none");
    QString tBody;

    if (!dir.isRoot()) {
        QDir upDir = dir;
        upDir.cdUp();

        upDirDisplay = QLatin1String("inline");
        page.replace(QLatin1String("%UP-DIR-LINK%"), QUrl::fromLocalFile(upDir.absolutePath()).toEncoded());
    }

    foreach(const QFileInfo & info, list) {
        if (info.fileName() == QLatin1String(".") || info.fileName() == QLatin1String("..")) {
            continue;
        }

        QString line = QLatin1String("<tr");

        if (info.isHidden()) {
            showHiddenDisplay = "inline";
            line += " class=\"tr-hidden\"";
        }

        line += QLatin1String("><td class=\"td-name\" style=\"background-image:url(data:image/png;base64,");
        line += qz_pixmapToByteArray(QFileIconProvider().icon(info).pixmap(16));
        line += QLatin1String(");\">");
        line += QLatin1String("<a href=\"");
        line += QUrl::fromLocalFile(info.absoluteFilePath()).toEncoded();
        line += QLatin1String("\">");
        line += info.fileName();
        line += QLatin1String("</a></td><td class=\"td-size\">");
        line += info.isDir() ? QString() : qz_fileSizeToString(info.size());
        line += QLatin1String("</td><td>");
        line += info.lastModified().toString("dd.MM.yyyy");
        line += QLatin1String("</td><td>");
        line += info.lastModified().toString("hh:mm:ss");
        line += QLatin1String("</td></tr>\n");

        tBody.append(line);
    }

    if (tBody.isEmpty()) {
        tBody = QString("<tr><td colspan='4'>%1</td></tr>").arg(tr("Folder is empty."));
    }

    page.replace(QLatin1String("%T-BODY%"), tBody);
    page.replace(QLatin1String("%UP-DIR-DISPLAY%"), upDirDisplay);
    page.replace(QLatin1String("%SHOW-HIDDEN-DISPLAY%"), showHiddenDisplay);

    return page;
}
