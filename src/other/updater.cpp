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

Updater::Updater(QupZilla* mainClass, QObject *parent) :
    QObject(parent)
    ,p_QupZilla(mainClass)
{
    createTrayIcon();
#ifndef DEVELOPING
    QTimer::singleShot(60*1000, this, SLOT(start()) ); //Start checking after 1 minute
#endif
}

void Updater::start()
{
    startDownloadingUpdateInfo(QUrl(QupZilla::WWWADDRESS+"/update.php?v="+QupZilla::VERSION));
}

void Updater::createTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIconMenu = new QMenu();
    m_trayIconMenu->addAction(tr("Go to download page"), p_QupZilla, SLOT(loadActionUrl()))->setData(QUrl(QupZilla::WWWADDRESS+"/download.php"));
    m_trayIconMenu->addAction(tr("Go to QupZilla website"), p_QupZilla, SLOT(loadActionUrl()))->setData(QUrl(QupZilla::WWWADDRESS));
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(tr("Hide notification"), m_trayIcon, SLOT(hide()));


    m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->setIcon(QIcon(":/icons/qupzillaupdate.png"));
    m_trayIcon->setToolTip(tr("QupZilla is checking for updates"));
}
void Updater::startDownloadingUpdateInfo(const QUrl &url)
{
//    trayIcon->show(); // Disabled, it was getting focus, so mainwindow lost focus
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkReply* reply;
    reply=manager->get(QNetworkRequest(QUrl(url)));

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downCompleted(QNetworkReply *)));
}

void Updater::downCompleted(QNetworkReply* reply)
{
    m_trayIcon->show();
    QString html = QString(reply->readAll());
    if (html.startsWith("Version:")){
        html.remove("Version:");
        if (html != QupZilla::VERSION) {
            connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(clicked(QSystemTrayIcon::ActivationReason)));
            connect(m_trayIcon, SIGNAL(messageClicked()), this, SLOT(goUpdate()));
            m_trayIcon->setToolTip(tr("QupZilla found a new version!"));
            m_trayIcon->showMessage(tr("New version is available"), tr("New version of QupZilla %1 is available!").arg(html));
        }
        else
            m_trayIcon->hide();
    }

    reply->manager()->deleteLater();
}

void Updater::goUpdate()
{
    p_QupZilla->tabWidget()->addView(QUrl(QupZilla::WWWADDRESS+"/download.php"), tr("QupZilla Update"));
}

void Updater::clicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
        p_QupZilla->tabWidget()->addView(QUrl(QupZilla::WWWADDRESS+"/download.php"), tr("QupZilla Update"));
}

Updater::~Updater()
{
    m_trayIcon->hide();
    delete m_trayIconMenu;
    delete m_trayIcon;
}
