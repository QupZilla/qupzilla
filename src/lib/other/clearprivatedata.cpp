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
#include "clearprivatedata.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include "cookiejar.h"
#include "history.h"
#include "settings.h"
#include "datapaths.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "clickablelabel.h"
#include "ui_clearprivatedata.h"
#include "iconprovider.h"
#include "qztools.h"

#include <QNetworkCookie>
#include <QMessageBox>
#include <QWebDatabase>
#include <QWebSettings>
#include <QNetworkDiskCache>
#include <QDateTime>
#include <QSqlQuery>
#include <QCloseEvent>
#include <QFileInfo>

ClearPrivateData::ClearPrivateData(BrowserWindow* window, QWidget* parent)
    : QDialog(parent)
    , m_window(window)
    , ui(new Ui::ClearPrivateData)
{
    ui->setupUi(this);
    ui->buttonBox->setFocus();
    connect(ui->history, SIGNAL(clicked(bool)), this, SLOT(historyClicked(bool)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(dialogAccepted()));
    connect(ui->optimizeDb, SIGNAL(clicked(QPoint)), this, SLOT(optimizeDb()));

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
    const QString profile = DataPaths::currentProfilePath();

    QzTools::removeDir(profile + "LocalStorage");
}

void ClearPrivateData::clearWebDatabases()
{
    const QString profile = DataPaths::currentProfilePath();

    QWebDatabase::removeAllDatabases();
    QzTools::removeDir(profile + "Databases");
}

void ClearPrivateData::clearCache()
{
    mApp->networkCache()->clear();
    mApp->webSettings()->clearMemoryCaches();

    QFile::remove(DataPaths::currentProfilePath() + "ApplicationCache.db");
}

void ClearPrivateData::clearIcons()
{
    mApp->webSettings()->clearIconDatabase();
    IconProvider::instance()->clearIconsDatabase();
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
    QMessageBox::StandardButton b = QMessageBox::question(this, tr("Clear Private Data"),
                                    tr("Are you sure to clear selected private data?"),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (b != QMessageBox::Yes) {
        return;
    }

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

void ClearPrivateData::optimizeDb()
{
    mApp->setOverrideCursor(Qt::WaitCursor);

    QString profilePath = DataPaths::currentProfilePath();
    QString sizeBefore = QzTools::fileSizeToString(QFileInfo(profilePath + "browsedata.db").size());

    mApp->history()->optimizeHistory();

    QString sizeAfter = QzTools::fileSizeToString(QFileInfo(profilePath + "browsedata.db").size());

    mApp->restoreOverrideCursor();

    QMessageBox::information(this, tr("Database Optimized"), tr("Database successfully optimized.<br/><br/><b>Database Size Before: </b>%1<br/><b>Database Size After: </b>%2").arg(sizeBefore, sizeAfter));
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
