/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "qupzilla.h"
#include "qztools.h"

#include <QTimer>
#include <QDateTime>
#include <QNetworkReply>
#include <QDebug>
#include <QFile>

CaBundleUpdater::CaBundleUpdater(NetworkManager* manager, QObject* parent)
    : QObject(parent)
    , m_manager(manager)
    , m_progress(Start)
    , m_latestBundleVersion(0)
{
    m_bundleVersionFileName = mApp->PROFILEDIR + "certificates/bundle_version";
    m_bundleFileName = mApp->PROFILEDIR + "certificates/ca-bundle.crt";
    m_lastUpdateFileName = mApp->PROFILEDIR + "certificates/last_update";

    QTimer::singleShot(30 * 1000, this, SLOT(start()));
}

void CaBundleUpdater::start()
{
    QFile updateFile(m_lastUpdateFileName);
    bool updateNow = false;

    if (updateFile.exists()) {
        if (!updateFile.open(QFile::ReadOnly)) {
            qWarning() << "CaBundleUpdater::start cannot open file for reading" << m_lastUpdateFileName;
        }
        else {
            QDateTime updateTime = QDateTime::fromString(updateFile.readAll());
            updateNow = updateTime.addDays(5) < QDateTime::currentDateTime();
        }
    }
    else {
        updateNow = true;
    }

    if (updateNow) {
        m_progress = CheckLastUpdate;

        QUrl url = QUrl::fromEncoded(QString(QupZilla::WWWADDRESS + "/certs/bundle_version").toUtf8());
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

        m_latestBundleVersion = response.toInt();
        int currentBundleVersion = QzTools::readAllFileContents(m_bundleVersionFileName).trimmed().toInt();

        if (m_latestBundleVersion == 0) {
            return;
        }

        if (m_latestBundleVersion > currentBundleVersion) {
            m_progress = LoadBundle;

            QUrl url = QUrl::fromEncoded(QString(QupZilla::WWWADDRESS + "/certs/ca-bundle.crt").toUtf8());
            m_reply = m_manager->get(QNetworkRequest(url));
            connect(m_reply, SIGNAL(finished()), this, SLOT(replyFinished()));
        }

        QFile file(m_lastUpdateFileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            qWarning() << "CaBundleUpdater::replyFinished cannot open file for writing" << m_lastUpdateFileName;
        }

        file.write(QDateTime::currentDateTime().toString().toUtf8());
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
        }

        file.write(QByteArray::number(m_latestBundleVersion));
        file.close();

        file.setFileName(m_bundleFileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            qWarning() << "CaBundleUpdater::replyFinished cannot open file for writing" << m_bundleFileName;
        }

        file.write(response);

        // Reload newly downloaded certificates
        mApp->networkManager()->loadSettings();
    }
}
