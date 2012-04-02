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
#include "clearprivatedata.h"
#include "qupzilla.h"
#include "tabwidget.h"
#include "cookiejar.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "clickablelabel.h"
#include "ui_clearprivatedata.h"
#include "iconprovider.h"
#include "globalfunctions.h"

#include <QWebDatabase>
#include <QWebSettings>
#include <QNetworkDiskCache>
#include <QDateTime>
#include <QSqlQuery>

ClearPrivateData::ClearPrivateData(QupZilla* mainClass, QWidget* parent)
    : QDialog(parent)
    , p_QupZilla(mainClass)
    , ui(new Ui::ClearPrivateData)
{
    ui->setupUi(this);
    ui->buttonBox->setFocus();
    connect(ui->clearAdobeCookies, SIGNAL(clicked(QPoint)), this, SLOT(clearFlash()));
    connect(ui->history, SIGNAL(clicked(bool)), this, SLOT(historyClicked(bool)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(dialogAccepted()));

    //Resizing +2 of sizeHint to get visible underlined link
    resize(sizeHint().width(), sizeHint().height() + 2);
}

void ClearPrivateData::historyClicked(bool state)
{
    ui->historyLength->setEnabled(state);
}

void ClearPrivateData::clearLocalStorage()
{
    const QString &profile = mApp->getActiveProfilPath();

    qz_removeDir(profile + "LocalStorage");
}

void ClearPrivateData::clearWebDatabases()
{
    const QString &profile = mApp->getActiveProfilPath();

    QWebDatabase::removeAllDatabases();
    qz_removeDir(profile + "Databases");
}

void ClearPrivateData::clearCache()
{
    mApp->webSettings()->clearMemoryCaches();
    mApp->networkManager()->cache()->clear();

    QFile::remove(mApp->getActiveProfilPath() + "ApplicationCache.db");
}

void ClearPrivateData::clearIcons()
{
    mApp->webSettings()->clearIconDatabase();
    mApp->iconProvider()->clearIconDatabase();
}

void ClearPrivateData::clearFlash()
{
    p_QupZilla->tabWidget()->addView(QUrl("http://www.macromedia.com/support/documentation/en/flashplayer/help/settings_manager07.html"));
}

void ClearPrivateData::dialogAccepted()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (ui->history->isChecked()) {
        QDateTime dateTime = QDateTime::currentDateTime();
        qint64 nowMS = QDateTime::currentMSecsSinceEpoch();
        qint64 date = 0;

        switch (ui->historyLength->currentIndex()) {
        case 0: //Later Today
            dateTime.setTime(QTime(0, 0));
            date = dateTime.toMSecsSinceEpoch();
            break;
        case 1: //Week
            date = nowMS - 60u * 60u * 24u * 7u * 1000u;
            break;
        case 2: //Month
            date = nowMS - 60u * 60u * 24u * 30u * 1000u;
            break;
        case 3: //All
            date = 0;
            break;
        }

        QSqlQuery query;
        query.exec("DELETE FROM history WHERE date > " + QString::number(date));
        query.exec("VACUUM");
    }

    if (ui->cookies->isChecked()) {
        mApp->cookieJar()->setAllCookies(QList<QNetworkCookie>());
    }

    if (ui->cache->isChecked()) {
        clearCache();
    }

    if (ui->databases->isChecked()) {
        clearWebDatabases();
    }

    if (ui->localStorage->isChecked()) {
        clearLocalStorage();
    }

    if (ui->icons->isChecked()) {
        clearIcons();
    }

    QApplication::restoreOverrideCursor();

    close();
}
