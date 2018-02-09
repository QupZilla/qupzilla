/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "browserwindow.h"
#include "tabwidget.h"
#include "cookiejar.h"
#include "history.h"
#include "settings.h"
#include "datapaths.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "ui_clearprivatedata.h"
#include "iconprovider.h"
#include "qztools.h"
#include "cookiemanager.h"
#include "desktopnotificationsfactory.h"

#include <QNetworkCookie>
#include <QMessageBox>
#include <QWebEngineSettings>
#include <QNetworkDiskCache>
#include <QDateTime>
#include <QCloseEvent>
#include <QFileInfo>
#include <QWebEngineProfile>

ClearPrivateData::ClearPrivateData(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ClearPrivateData)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->buttonBox->setFocus();
    connect(ui->history, SIGNAL(clicked(bool)), this, SLOT(historyClicked(bool)));
    connect(ui->clear, SIGNAL(clicked(bool)), this, SLOT(dialogAccepted()));
    connect(ui->optimizeDb, SIGNAL(clicked(bool)), this, SLOT(optimizeDb()));
    connect(ui->editCookies, SIGNAL(clicked()), this, SLOT(showCookieManager()));

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
    const QString profile = DataPaths::currentProfilePath();

    QzTools::removeDir(profile + "/Local Storage");
}

void ClearPrivateData::clearWebDatabases()
{
    const QString profile = DataPaths::currentProfilePath();

    QzTools::removeDir(profile + "/IndexedDB");
    QzTools::removeDir(profile + "/databases");
}

void ClearPrivateData::clearCache()
{
    const QString profile = DataPaths::currentProfilePath();

    QzTools::removeDir(profile + "/GPUCache");

    mApp->webProfile()->clearHttpCache();
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

        const QDate today = QDate::currentDate();
        const QDate week = today.addDays(1 - today.dayOfWeek());
        const QDate month = QDate(today.year(), today.month(), 1);

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
        mApp->cookieJar()->deleteAllCookies();
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

    QApplication::restoreOverrideCursor();

    ui->clear->setEnabled(false);
    ui->clear->setText(tr("Done"));

    QTimer::singleShot(1000, this, SLOT(close()));
}

void ClearPrivateData::optimizeDb()
{
    mApp->setOverrideCursor(Qt::WaitCursor);

    const QString profilePath = DataPaths::currentProfilePath();
    QString sizeBefore = QzTools::fileSizeToString(QFileInfo(profilePath + "/browsedata.db").size());

    IconProvider::instance()->clearOldIconsInDatabase();

    QString sizeAfter = QzTools::fileSizeToString(QFileInfo(profilePath + "/browsedata.db").size());

    mApp->restoreOverrideCursor();

    QMessageBox::information(this, tr("Database Optimized"), tr("Database successfully optimized.<br/><br/><b>Database Size Before: </b>%1<br/><b>Database Size After: </b>%2").arg(sizeBefore, sizeAfter));
}

void ClearPrivateData::showCookieManager()
{
    CookieManager* dialog = new CookieManager(this);
    dialog->show();
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

    return data;
}
