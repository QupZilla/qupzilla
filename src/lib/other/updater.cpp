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
#include "updater.h"
#include "qupzilla.h"
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
    QTimer::singleShot(60 * 1000, this, SLOT(start())); //Start checking after 1 minute
}

Updater::Version Updater::parseVersionFromString(const QString &string)
{
    Version ver;
    ver.isValid = false;

    QStringList v = string.split(".");
    if (v.count() != 3) {
        return ver;
    }

    QStringList r = v.at(2).split("-");

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
    if (one.contains("rc") && two.contains('b')) {
        return true;
    }

    if (one.contains("b") && two.contains("rc")) {
        return false;
    }

    if (one.isEmpty()) {
        return true;
    }

    if (two.isEmpty()) {
        return false;
    }

    if (one.contains('b')) {
        int o = one.remove('b').toInt();
        int t = two.remove('b').toInt();

        return o > t;
    }

    if (one.contains("rc")) {
        int o = one.remove("rc").toInt();
        int t = two.remove("rc").toInt();

        return o > t;
    }

    return false;
}

void Updater::start()
{
    startDownloadingUpdateInfo(QUrl(QupZilla::WWWADDRESS + "/update.php?v=" + QupZilla::VERSION));
}

void Updater::startDownloadingUpdateInfo(const QUrl &url)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    manager->get(QNetworkRequest(QUrl(url)));

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downCompleted(QNetworkReply*)));
}

void Updater::downCompleted(QNetworkReply* reply)
{
    QString html = QString(reply->readAll());
    if (html.startsWith("Version:")) {
        html.remove("Version:");
        Version current = parseVersionFromString(QupZilla::VERSION);
        Version updated = parseVersionFromString(html);
        if (current < updated) {
            mApp->desktopNotifications()->showNotifications(QPixmap(":icons/qupzillaupdate.png"), tr("Update available"), tr("New version of QupZilla is ready to download."));
//            QAction* action = new QAction(QIcon(":icons/qupzillaupdate.png"), "Update", this);
//            connect(action, SIGNAL(triggered()), this, SLOT(downloadNewVersion()));
//            p_QupZilla->menuBar()->addAction(action);
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
