/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2017 David Rosca <nowrep@gmail.com>
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
#include "gm_downloader.h"
#include "gm_manager.h"
#include "gm_script.h"

#include "webpage.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "qztools.h"
#include "qzregexp.h"

#include <QFile>
#include <QSettings>
#include <QNetworkReply>

GM_Downloader::GM_Downloader(const QUrl &url, GM_Manager* manager)
    : QObject()
    , m_manager(manager)
{
    m_reply = mApp->networkManager()->get(QNetworkRequest(url));
    connect(m_reply, &QNetworkReply::finished, this, &GM_Downloader::scriptDownloaded);
}

void GM_Downloader::updateScript(const QString &fileName)
{
    m_fileName = fileName;
}

void GM_Downloader::scriptDownloaded()
{
    if (m_reply != qobject_cast<QNetworkReply*>(sender())) {
        emit error();
        deleteLater();
        return;
    }

    if (m_reply->error() != QNetworkReply::NoError) {
        qWarning() << "GreaseMonkey: Cannot download script" << m_reply->errorString();
    }
    else {
        const QByteArray response = QString::fromUtf8(m_reply->readAll()).toUtf8();

        if (response.contains(QByteArray("// ==UserScript=="))) {
            if (m_fileName.isEmpty()) {
                const QString filePath = QString("%1/%2").arg(m_manager->scriptsDirectory(), QzTools::getFileNameFromUrl(m_reply->url()));
                m_fileName = QzTools::ensureUniqueFilename(filePath);
            }
            QFile file(m_fileName);

            if (!file.open(QFile::WriteOnly)) {
                qWarning() << "GreaseMonkey: Cannot open file for writing" << m_fileName;
                emit error();
                deleteLater();
                return;
            }

            file.write(response);
            file.close();

            QSettings settings(m_manager->settinsPath() + QL1S("/greasemonkey/requires/requires.ini"), QSettings::IniFormat);
            settings.beginGroup("Files");

            QzRegExp rx("@require(.*)\\n");
            rx.setMinimal(true);
            rx.indexIn(response);

            for (int i = 1; i <= rx.captureCount(); ++i) {
                const QString url = rx.cap(i).trimmed();
                if (!url.isEmpty() && !settings.contains(url)) {
                    m_requireUrls.append(QUrl(url));
                }
            }
        }
    }

    m_reply->deleteLater();
    m_reply = 0;

    downloadRequires();
}

void GM_Downloader::requireDownloaded()
{
    if (m_reply != qobject_cast<QNetworkReply*>(sender())) {
        emit error();
        deleteLater();
        return;
    }

    if (m_reply->error() != QNetworkReply::NoError) {
        qWarning() << "GreaseMonkey: Cannot download require script" << m_reply->errorString();
    }
    else {
        const QByteArray response = QString::fromUtf8(m_reply->readAll()).toUtf8();

        if (!response.isEmpty()) {
            const QString filePath = m_manager->settinsPath() + QL1S("/greasemonkey/requires/require.js");
            const QString fileName = QzTools::ensureUniqueFilename(filePath, "%1");

            QFile file(fileName);

            if (!file.open(QFile::WriteOnly)) {
                qWarning() << "GreaseMonkey: Cannot open file for writing" << fileName;
                emit error();
                deleteLater();
                return;
            }

            file.write(response);
            file.close();

            QSettings settings(m_manager->settinsPath() + QL1S("/greasemonkey/requires/requires.ini"), QSettings::IniFormat);
            settings.beginGroup("Files");
            settings.setValue(m_reply->request().url().toString(), QFileInfo(fileName).fileName());
        }
    }

    m_reply->deleteLater();
    m_reply = 0;

    downloadRequires();
}

void GM_Downloader::downloadRequires()
{
    if (!m_requireUrls.isEmpty()) {
        m_reply = mApp->networkManager()->get(QNetworkRequest(m_requireUrls.takeFirst()));
        connect(m_reply, &QNetworkReply::finished, this, &GM_Downloader::requireDownloaded);
    }
    else {
        emit finished(m_fileName);
        deleteLater();
    }
}
