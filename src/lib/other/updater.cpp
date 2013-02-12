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

    QStringList r = v.at(2).split(QLatin1Char('.'));

    ver.majorVersion = v.at(0).toInt();
    ver.minorVersion = v.at(1).toInt();
    ver.revisionNumber = r.at(0).toInt();
    if (r.count() == 2) {
        ver.specialSymbol = r.at(1);
    }

    ver.isValid = true;
    return ver;
}

bool Updater::isBiggerThan_SpecialSymbol(QString one, QString two)
{
    if (one.contains(QLatin1String("rc")) && two.contains(QLatin1Char('b'))) {
        return true;
    }

    if (one.contains(QLatin1Char('b')) && two.contains(QLatin1String("rc"))) {
        return false;
    }

    if (one.isEmpty()) {
        return true;
    }

    if (two.isEmpty()) {
        return false;
    }

    if (one.contains(QLatin1Char('b'))) {
        int o = one.remove(QLatin1Char('b')).toInt();
        int t = two.remove(QLatin1Char('b')).toInt();

        return o > t;
    }

    if (one.contains(QLatin1String("rc"))) {
        int o = one.remove(QLatin1String("rc")).toInt();
        int t = two.remove(QLatin1String("rc")).toInt();

        return o > t;
    }

    return false;
}

void Updater::start()
{
    QUrl url = QUrl(QString("%1/update.php?v=%2&os=%3").arg(QupZilla::WWWADDRESS,
                    QupZilla::VERSION,
                    QzTools::buildSystem()));

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

        if (current < updated) {
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
