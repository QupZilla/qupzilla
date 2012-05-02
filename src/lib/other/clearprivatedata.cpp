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
#include "history.h"
#include "settings.h"
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
#include <QCloseEvent>

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

    Settings settings;
    settings.beginGroup("ClearPrivateData");
    restoreState(settings.value("state", QByteArray()).toByteArray());
    settings.endGroup();
}

void ClearPrivateData::historyClicked(bool state)
{
    ui->historyLength->setEnabled(state);
}

void ClearPrivateData::clearLocalStorage()
{
    const QString &profile = mApp->currentProfilePath();

    qz_removeDir(profile + "LocalStorage");
}

void ClearPrivateData::clearWebDatabases()
{
    const QString &profile = mApp->currentProfilePath();

    QWebDatabase::removeAllDatabases();
    qz_removeDir(profile + "Databases");
}

void ClearPrivateData::clearCache()
{
    mApp->webSettings()->clearMemoryCaches();
    mApp->networkManager()->cache()->clear();

    QFile::remove(mApp->currentProfilePath() + "ApplicationCache.db");
}

void ClearPrivateData::clearIcons()
{
    mApp->webSettings()->clearIconDatabase();
    qIconProvider->clearIconDatabase();
}

void ClearPrivateData::clearFlash()
{
    p_QupZilla->tabWidget()->addView(QUrl("http://www.macromedia.com/support/documentation/en/flashplayer/help/settings_manager07.html"));
}

void ClearPrivateData::closeEvent(QCloseEvent* e)
{
    Settings settings;
    settings.beginGroup("ClearPrivateData");
    settings.setValue("state", saveState());
    settings.endGroup();

    e->accept();
}

void ClearPrivateData::dialogAccepted()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (ui->history->isChecked()) {
        qint64 start = QDateTime::currentMSecsSinceEpoch();
        qint64 end = 0;

        const QDate &today = QDate::currentDate();
        const QDate &week = today.addDays(1 - today.dayOfWeek());
        const QDate &month = QDate(today.year(), today.month(), 1);

        switch (ui->historyLength->currentIndex()) {
        case 0: //Later Today
            end = QDateTime(today).toMSecsSinceEpoch();
            break;
        case 1: //Week
            end = QDateTime(week).toMSecsSinceEpoch();
            break;
        case 2: //Month
            end = QDateTime(month).toMSecsSinceEpoch();
            break;
        case 3: //All
            break;
        }

        if (end == 0) {
            mApp->history()->clearHistory();
        }
        else {
            const QList<int> &indexes = mApp->history()->indexesFromTimeRange(start, end);
            mApp->history()->deleteHistoryEntry(indexes);
        }
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

static const int stateDataVersion = 0x0001;

void ClearPrivateData::restoreState(const QByteArray &state)
{
    QDataStream stream(state);
    if (stream.atEnd()) {
        return;
    }

    int version = -1;
    int historyIndex = -1;
    bool databases = false;
    bool localStorage = false;
    bool cache = false;
    bool cookies = false;
    bool icons = false;

    stream >> version;
    if (version != stateDataVersion) {
        return;
    }

    stream >> historyIndex;
    stream >> databases;
    stream >> localStorage;
    stream >> cache;
    stream >> cookies;
    stream >> icons;

    if (historyIndex != -1) {
        ui->history->setChecked(true);
        ui->historyLength->setEnabled(true);
        ui->historyLength->setCurrentIndex(historyIndex);
    }

    ui->databases->setChecked(databases);
    ui->localStorage->setChecked(localStorage);
    ui->cache->setChecked(cache);
    ui->cookies->setChecked(cookies);
    ui->icons->setChecked(icons);
}

QByteArray ClearPrivateData::saveState()
{
    // history - web database - local storage - cache - cookies - icons
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << stateDataVersion;

    if (!ui->history->isChecked()) {
        stream << -1;
    }
    else {
        stream << ui->historyLength->currentIndex();
    }

    stream << ui->databases->isChecked();
    stream << ui->localStorage->isChecked();
    stream << ui->cache->isChecked();
    stream << ui->cookies->isChecked();
    stream << ui->icons->isChecked();

    return data;
}
