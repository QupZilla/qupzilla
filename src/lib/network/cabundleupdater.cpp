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
#include "cabundleupdater.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "browserwindow.h"
#include "datapaths.h"
#include "qztools.h"

#include <QTimer>
#include <QDateTime>
#include <QNetworkReply>
#include <QFile>

CaBundleUpdater::CaBundleUpdater(NetworkManager* manager, QObject* parent)
    : QObject(parent)
    , m_manager(manager)
    , m_progress(Start)
    , m_reply(0)
    , m_latestBundleVersion(0)
{
    m_bundleVersionFileName = DataPaths::path(DataPaths::Config) + QL1S("/certificates/bundle_version");
    m_bundleFileName = DataPaths::path(DataPaths::Config) + QL1S("/certificates/ca-bundle.crt");
    m_lastUpdateFileName = DataPaths::path(DataPaths::Config) + QL1S("/certificates/last_update");

    // Make sure the certificates directory exists
    QDir certDir(DataPaths::path(DataPaths::Config) + QL1S("/certificates"));
    if (!certDir.exists())
        certDir.mkpath(certDir.absolutePath());

    int updateTime = 30 * 1000;

    // Check immediately on first run
    if (!QFile(m_lastUpdateFileName).exists()) {
        updateTime = 0;
    }

    QTimer::singleShot(updateTime, this, SLOT(start()));
}

void CaBundleUpdater::start()
{
    QFile updateFile(m_lastUpdateFileName);
    bool updateNow = false;

    if (updateFile.exists()) {
        if (updateFile.open(QFile::ReadOnly)) {
            QDateTime updateTime = QDateTime::fromString(updateFile.readAll());
            updateNow = updateTime.addDays(5) < QDateTime::currentDateTime();
        }
        else {
            qWarning() << "CaBundleUpdater::start cannot open file for reading" << m_lastUpdateFileName;
        }
    }
    else {
        updateNow = true;
    }

    if (updateNow) {
        m_progress = CheckLastUpdate;

        QUrl url = QUrl::fromEncoded(QString(Qz::WWWADDRESS + QL1S("/certs/bundle_version")).toUtf8());
        m_reply = m_manager->get(QNetworkRequest(url));
        connect(m_reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    }
}

void CaBundleUpdater::replyFinished()
{
    if (m_progress == CheckLastUpdate) {
        QByteArray response = m_reply->readAll().trimmed();
        m_reply->close();
        m_reply->deleteLater();

        if (m_reply->error() != QNetworkReply::NoError || response.isEmpty()) {
            return;
        }

        bool ok;

        m_latestBundleVersion = response.toInt(&ok);
        if (!ok || m_latestBundleVersion <= 0)
            return;

        int currentBundleVersion = QzTools::readAllFileContents(m_bundleVersionFileName).trimmed().toInt(&ok);
        if (!ok)
            currentBundleVersion = 0;

        QFile file(m_lastUpdateFileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            qWarning() << "CaBundleUpdater::replyFinished cannot open file for writing" << m_lastUpdateFileName;
            return;
        }
        file.write(QDateTime::currentDateTime().toString().toUtf8());

        if (m_latestBundleVersion > currentBundleVersion) {
            m_progress = LoadBundle;

            QUrl url = QUrl::fromEncoded(QString(Qz::WWWADDRESS + QL1S("/certs/ca-bundle.crt")).toUtf8());
            m_reply = m_manager->get(QNetworkRequest(url));
            connect(m_reply, SIGNAL(finished()), this, SLOT(replyFinished()));
        }
    }
    else if (m_progress == LoadBundle) {
        QByteArray response = m_reply->readAll();
        m_reply->close();
        m_reply->deleteLater();

        if (m_reply->error() != QNetworkReply::NoError || response.isEmpty()) {
            return;
        }

        QFile file(m_bundleVersionFileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            qWarning() << "CaBundleUpdater::replyFinished cannot open file for writing" << m_bundleVersionFileName;
            return;
        }

        file.write(QByteArray::number(m_latestBundleVersion));
        file.close();

        file.setFileName(m_bundleFileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            qWarning() << "CaBundleUpdater::replyFinished cannot open file for writing" << m_bundleFileName;
            return;
        }

        file.write(response);

        // Reload newly downloaded certificates
        mApp->networkManager()->loadSettings();
    }
}
