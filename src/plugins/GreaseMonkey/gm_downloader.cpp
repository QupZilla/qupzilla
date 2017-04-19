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

GM_Downloader::GM_Downloader(const QUrl &url, GM_Manager *manager, Mode mode)
    : QObject()
    , m_manager(manager)
{
    m_reply = mApp->networkManager()->get(QNetworkRequest(url));
    if (mode == DownloadMainScript) {
        connect(m_reply, &QNetworkReply::finished, this, &GM_Downloader::scriptDownloaded);
    } else {
        connect(m_reply, &QNetworkReply::finished, this, &GM_Downloader::requireDownloaded);
    }
}

void GM_Downloader::updateScript(const QString &fileName)
{
    m_fileName = fileName;
}

void GM_Downloader::scriptDownloaded()
{
    Q_ASSERT(m_reply == qobject_cast<QNetworkReply*>(sender()));

    deleteLater();
    m_reply->deleteLater();

    if (m_reply->error() != QNetworkReply::NoError) {
        qWarning() << "GreaseMonkey: Cannot download script" << m_reply->errorString();
        emit error();
        return;
    }

    const QByteArray response = QString::fromUtf8(m_reply->readAll()).toUtf8();

    if (!response.contains(QByteArray("// ==UserScript=="))) {
        qWarning() << "GreaseMonkey: Script does not contain UserScript header" << m_reply->request().url();
        emit error();
        return;
    }

    if (m_fileName.isEmpty()) {
        const QString filePath = QString("%1/%2").arg(m_manager->scriptsDirectory(), QzTools::getFileNameFromUrl(m_reply->url()));
        m_fileName = QzTools::ensureUniqueFilename(filePath);
    }

    QFile file(m_fileName);

    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "GreaseMonkey: Cannot open file for writing" << m_fileName;
        emit error();
        return;
    }

    file.write(response);
    file.close();

    emit finished(m_fileName);
}

void GM_Downloader::requireDownloaded()
{
    Q_ASSERT(m_reply == qobject_cast<QNetworkReply*>(sender()));

    deleteLater();
    m_reply->deleteLater();

    if (m_reply != qobject_cast<QNetworkReply*>(sender())) {
        emit error();
        return;
    }

    if (m_reply->error() != QNetworkReply::NoError) {
        qWarning() << "GreaseMonkey: Cannot download require script" << m_reply->errorString();
        emit error();
        return;
    }

    const QByteArray response = QString::fromUtf8(m_reply->readAll()).toUtf8();

    if (response.isEmpty()) {
        qWarning() << "GreaseMonkey: Empty script downloaded" << m_reply->request().url();
        emit error();
        return;
    }

    QSettings settings(m_manager->settinsPath() + QL1S("/greasemonkey/requires/requires.ini"), QSettings::IniFormat);
    settings.beginGroup("Files");

    if (m_fileName.isEmpty()) {
        m_fileName = settings.value(m_reply->request().url().toString()).toString();
        if (m_fileName.isEmpty()) {
            QString name = QFileInfo(m_reply->request().url().path()).fileName();
            if (name.isEmpty()) {
                name = QSL("require.js");
            } else if (!name.endsWith(QL1S(".js"))) {
                name.append(QSL(".js"));
            }
            const QString filePath = m_manager->settinsPath() + QL1S("/greasemonkey/requires/") + name;
            m_fileName = QzTools::ensureUniqueFilename(filePath, "%1");
        }
        if (!QFileInfo(m_fileName).isAbsolute()) {
            m_fileName.prepend(m_manager->settinsPath() + QL1S("/greasemonkey/requires/"));
        }
    }

    QFile file(m_fileName);

    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "GreaseMonkey: Cannot open file for writing" << m_fileName;
        emit error();
        return;
    }

    file.write(response);
    file.close();

    settings.setValue(m_reply->request().url().toString(), QFileInfo(m_fileName).fileName());

    emit finished(m_fileName);
}
