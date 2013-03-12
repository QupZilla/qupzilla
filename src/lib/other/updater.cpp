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
#include "updater.h"
#include "qupzilla.h"
#include "qztools.h"
#include "mainapplication.h"
#include "tabwidget.h"
#include "desktopnotificationsfactory.h"

#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

Updater::Updater(QupZilla* mainClass, QObject* parent)
    : QObject(parent)
    , p_QupZilla(mainClass)
{
    QTimer::singleShot(60 * 1000, this, SLOT(start())); // Start checking after 1 minute
}

Updater::Version Updater::parseVersionFromString(const QString &string)
{
    Version ver;
    ver.isValid = false;

    QStringList v = string.split(QLatin1Char('.'));
    if (v.count() != 3) {
        return ver;
    }

    bool ok;

    ver.majorVersion = v.at(0).toInt(&ok);
    if (!ok) {
        return ver;
    }

    ver.minorVersion = v.at(1).toInt(&ok);
    if (!ok) {
        return ver;
    }

    ver.revisionNumber = v.at(2).toInt(&ok);
    if (!ok) {
        return ver;
    }

    ver.isValid = true;
    return ver;
}

void Updater::start()
{
    QUrl url = QUrl(QString("%1/update.php?v=%2&os=%3").arg(QupZilla::WWWADDRESS,
                    QupZilla::VERSION,
                    QzTools::operatingSystem()));

    startDownloadingUpdateInfo(url);
}

void Updater::startDownloadingUpdateInfo(const QUrl &url)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    manager->get(QNetworkRequest(QUrl(url)));

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downCompleted(QNetworkReply*)));
}

void Updater::downCompleted(QNetworkReply* reply)
{
    QString html = reply->readAll();

    if (html.startsWith(QLatin1String("Version:"))) {
        html.remove(QLatin1String("Version:"));
        Version current = parseVersionFromString(QupZilla::VERSION);
        Version updated = parseVersionFromString(html);

        if (current.isValid && updated.isValid && current < updated) {
            mApp->desktopNotifications()->showNotification(QPixmap(":icons/qupzillaupdate.png"), tr("Update available"), tr("New version of QupZilla is ready to download."));
        }
    }

    reply->manager()->deleteLater();
}

void Updater::downloadNewVersion()
{
    p_QupZilla->tabWidget()->addView(QUrl(QupZilla::WWWADDRESS + "/download"), tr("Update"), Qz::NT_NotSelectedTab);
}

Updater::~Updater()
{
}
