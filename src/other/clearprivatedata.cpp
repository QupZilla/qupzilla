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
#include "clearprivatedata.h"
#include "qupzilla.h"
#include "cookiejar.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "clickablelabel.h"
#include "ui_clearprivatedata.h"
#include "iconprovider.h"

ClearPrivateData::ClearPrivateData(QupZilla* mainClass, QWidget* parent) :
    QDialog(parent)
    ,p_QupZilla(mainClass)
    ,ui(new Ui::ClearPrivateData)
{
    ui->setupUi(this);
    ui->buttonBox->setFocus();
    connect(ui->clearAdobeCookies, SIGNAL(clicked(QPoint)), this, SLOT(clearFlash()));
    connect(ui->history, SIGNAL(clicked(bool)), this, SLOT(historyClicked(bool)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(dialogAccepted()));
    resize(sizeHint());
}

void ClearPrivateData::historyClicked(bool state)
{
    ui->historyLength->setEnabled(state);
}

void ClearPrivateData::clearFlash()
{
    p_QupZilla->tabWidget()->addView(QUrl("http://www.macromedia.com/support/documentation/en/flashplayer/help/settings_manager07.html"));
}

void ClearPrivateData::dialogAccepted()
{
    if (ui->history->isChecked()) {
        QDateTime dateTime = QDateTime::currentDateTime();
        qint64 nowMS = QDateTime::currentMSecsSinceEpoch();
        qint64 date = 0;

        switch (ui->historyLength->currentIndex()) {
        case 0: //Later Today
            dateTime.setTime(QTime(0,0));
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
        query.exec("DELETE FROM history WHERE date > "+QString::number(date));
        query.exec("VACUUM");
    }
    if (ui->cookies->isChecked()) {
        QList<QNetworkCookie> cookies;
        mApp->cookieJar()->setAllCookies(cookies);
    }
    if (ui->cache->isChecked()) {
        mApp->webSettings()->clearMemoryCaches();
        mApp->networkManager()->cache()->clear();
    }
    if (ui->icons->isChecked()) {
        mApp->webSettings()->clearIconDatabase();
        mApp->iconProvider()->clearIconDatabase();
    }
    close();
}
