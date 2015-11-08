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
#include "updater.h"
#include "browserwindow.h"
#include "qztools.h"
#include "mainapplication.h"
#include "tabwidget.h"
#include "desktopnotificationsfactory.h"
#include "networkmanager.h"

#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>

Updater::Version::Version(const QString &s)
    : isValid(false)
    , majorVersion(-1)
    , minorVersion(-1)
    , revisionNumber(-1)
{
    isValid = false;

    QStringList v = s.split(QLatin1Char('.'));
    if (v.count() != 3) {
        return;
    }

    bool ok;

    majorVersion = v.at(0).toInt(&ok);
    if (!ok) {
        return;
    }

    minorVersion = v.at(1).toInt(&ok);
    if (!ok) {
        return;
    }

    revisionNumber = v.at(2).toInt(&ok);
    if (!ok) {
        return;
    }

    isValid = majorVersion >= 0 && minorVersion >= 0 && revisionNumber >= 0;
}

bool Updater::Version::operator <(const Updater::Version &other) const
{
    if (this->majorVersion != other.majorVersion) {
        return this->majorVersion < other.majorVersion;
    }
    if (this->minorVersion != other.minorVersion) {
        return this->minorVersion < other.minorVersion;
    }
    if (this->revisionNumber != other.revisionNumber) {
        return this->revisionNumber < other.revisionNumber;
    }

    return false;
}

bool Updater::Version::operator >(const Updater::Version &other) const
{
    if (*this == other) {
        return false;
    }
    return !operator<(other);
}

bool Updater::Version::operator ==(const Updater::Version &other) const
{
    return (this->majorVersion == other.majorVersion &&
            this->minorVersion == other.minorVersion &&
            this->revisionNumber == other.revisionNumber);
}

bool Updater::Version::operator >=(const Updater::Version &other) const
{
    if (*this == other) {
        return true;
    }
    return *this > other;
}

bool Updater::Version::operator <=(const Updater::Version &other) const
{
    if (*this == other) {
        return true;
    }
    return *this < other;
}

QString Updater::Version::versionString() const
{
    return QString("%1.%2.%3").arg(majorVersion, minorVersion, revisionNumber);
}

Updater::Updater(BrowserWindow* window, QObject* parent)
    : QObject(parent)
    , m_window(window)
{
    QTimer::singleShot(60 * 1000, this, SLOT(start())); // Start checking after 1 minute
}

void Updater::start()
{
    QUrl url = QUrl(QString("%1/update.php?v=%2&os=%3").arg(Qz::WWWADDRESS,
                    Qz::VERSION,
                    QzTools::operatingSystem()));

    startDownloadingUpdateInfo(url);
}

void Updater::startDownloadingUpdateInfo(const QUrl &url)
{
    QNetworkReply *reply = mApp->networkManager()->get(QNetworkRequest(QUrl(url)));

    connect(reply, SIGNAL(finished()), this, SLOT(downCompleted()));
}

void Updater::downCompleted()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    QString html = reply->readAll();

    if (html.startsWith(QLatin1String("Version:"))) {
        html.remove(QLatin1String("Version:"));
        Version current(Qz::VERSION);
        Version updated(html);

        if (current.isValid && updated.isValid && current < updated) {
            mApp->desktopNotifications()->showNotification(QPixmap(":icons/qupzilla.png"), tr("Update available"), tr("New version of QupZilla is ready to download."));
        }
    }

    reply->deleteLater();
}

void Updater::downloadNewVersion()
{
    m_window->tabWidget()->addView(QUrl::fromEncoded(QByteArray(Qz::WWWADDRESS) + QByteArray("/download")), tr("Update"), Qz::NT_NotSelectedTab);
}
