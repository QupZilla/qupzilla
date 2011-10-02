/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "tabwidget.h"
#include "desktopnotificationsfactory.h"

Updater::Updater(QupZilla* mainClass, QObject* parent) :
    QObject(parent)
    ,p_QupZilla(mainClass)
{
#ifndef DEVELOPING
    QTimer::singleShot(60*1000, this, SLOT(start()) ); //Start checking after 1 minute
#endif
}

void Updater::start()
{
    startDownloadingUpdateInfo(QUrl(QupZilla::WWWADDRESS+"/update.php?v="+QupZilla::VERSION));
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
    if (html.startsWith("Version:")){
        html.remove("Version:");
        if (html != QupZilla::VERSION) {
            mApp->desktopNotifications()->notify(QPixmap(":icons/qupzillaupdate.png"), tr("Update available"), tr("New version of QupZilla is ready to download."));
//            QAction* action = new QAction(QIcon(":icons/qupzillaupdate.png"), "Update", this);
//            connect(action, SIGNAL(triggered()), this, SLOT(downloadNewVersion()));
//            p_QupZilla->menuBar()->addAction(action);
        }
    }
    reply->manager()->deleteLater();
}

void Updater::downloadNewVersion()
{
    p_QupZilla->tabWidget()->addView(QUrl(QupZilla::WWWADDRESS + "/download.php"), tr("Update"), TabWidget::NewSelectedTab);
}

Updater::~Updater()
{
}
