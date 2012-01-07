#include "cabundleupdater.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "qupzilla.h"
#include "globalfunctions.h"

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

        m_reply = m_manager->get(QNetworkRequest(QupZilla::WWWADDRESS + "/certs/bundle_version"));
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
        int currentBundleVersion = qz_readAllFileContents(m_bundleVersionFileName).trimmed().toInt();

        if (m_latestBundleVersion == 0) {
            return;
        }

        if (m_latestBundleVersion > currentBundleVersion) {
            m_progress = LoadBundle;
            m_reply = m_manager->get(QNetworkRequest(QupZilla::WWWADDRESS + "/certs/ca-bundle.crt"));
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
    }
}
