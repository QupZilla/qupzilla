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
#include "historymodel.h"
#include "webview.h"
#include "qupzilla.h"
#include "iconprovider.h"
#include "databasewriter.h"

HistoryModel::HistoryModel(QupZilla* mainClass)
    : QObject()
    , m_isSaving(true)
    , p_QupZilla(mainClass)
{
    loadSettings();

    qRegisterMetaType<HistoryModel::HistoryEntry>("HistoryModel::HistoryEntry");

    QThread* t = new QThread(this);
    t->start();
    moveToThread(t);

    connect(this, SIGNAL(signalAddHistoryEntry(QUrl, QString)), this, SLOT(slotAddHistoryEntry(QUrl, QString)));
    connect(this, SIGNAL(signalDeleteHistoryEntry(int)), this, SLOT(slotDeleteHistoryEntry(int)));
}

void HistoryModel::loadSettings()
{
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    m_isSaving = settings.value("allowHistory", true).toBool();
    settings.endGroup();
}

// AddHistoryEntry
void HistoryModel::addHistoryEntry(WebView* view)
{
    if (!m_isSaving) {
        return;
    }

    QUrl url = view->url();
    QString title = view->title();

    addHistoryEntry(url, title);
}

void HistoryModel::addHistoryEntry(const QUrl &url, QString title)
{
    emit signalAddHistoryEntry(url, title);
}

void HistoryModel::slotAddHistoryEntry(const QUrl &url, QString title)
{
    if (!m_isSaving) {
        return;
    }
    if (url.scheme() == "file:" || url.scheme() == "qupzilla" || url.scheme() == "about" ||
            title.contains(tr("Failed loading page")) || url.isEmpty()) {
        return;
    }
    if (title == "") {
        title = tr("No Named Page");
    }

    QSqlQuery query;
    query.prepare("SELECT id FROM history WHERE url=?");
    query.bindValue(0, url);
    query.exec();
    if (!query.next()) {
        query.prepare("INSERT INTO history (count, date, url, title) VALUES (1,?,?,?)");
        query.bindValue(0, QDateTime::currentMSecsSinceEpoch());
        query.bindValue(1, url);
        query.bindValue(2, title);
        query.exec();

        int id = query.lastInsertId().toInt();
        HistoryEntry entry;
        entry.id = id;
        entry.count = 1;
        entry.date = QDateTime::currentDateTime();
        entry.url = url;
        entry.title = title;
        emit historyEntryAdded(entry);
    }
    else {
        int id = query.value(0).toInt();
        query.prepare("UPDATE history SET count = count + 1, date=?, title=? WHERE url=?");
        query.bindValue(0, QDateTime::currentMSecsSinceEpoch());
        query.bindValue(1, title);
        query.bindValue(2, url);
        query.exec();

        HistoryEntry before;
        before.id = id;

        HistoryEntry after;
        after.id = id;
        after.date = QDateTime::currentDateTime();
        after.url = url;
        after.title = title;
        emit historyEntryEdited(before, after);
    }
}

// DeleteHistoryEntry
void HistoryModel::deleteHistoryEntry(int index)
{
    emit signalDeleteHistoryEntry(index);
}

void HistoryModel::deleteHistoryEntry(const QString &url, const QString &title)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM history WHERE url=? AND title=?");
    query.bindValue(0, url);
    query.bindValue(1, title);
    query.exec();
    if (query.next()) {
        int id = query.value(0).toInt();
        deleteHistoryEntry(id);
    }
}

void HistoryModel::slotDeleteHistoryEntry(int index)
{
    QSqlQuery query;
    query.prepare("SELECT id, count, date, url, title FROM history WHERE id=?");
    query.bindValue(0, index);
    query.exec();
    if (!query.next()) {
        return;
    }

    HistoryEntry entry;
    entry.id = query.value(0).toInt();
    entry.count = query.value(1).toInt();
    entry.date = QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong());
    entry.url = query.value(3).toUrl();
    entry.title = query.value(4).toString();

    query.prepare("DELETE FROM history WHERE id=?");
    query.bindValue(0, index);
    query.exec();
    query.prepare("DELETE FROM icons WHERE url=?");
    query.bindValue(0, entry.url.toEncoded(QUrl::RemoveFragment));
    query.exec();

    emit historyEntryDeleted(entry);
}

bool HistoryModel::urlIsStored(const QString &url)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM history WHERE url=?");
    query.bindValue(0, url);
    query.exec();
    return query.next();
}

QList<HistoryModel::HistoryEntry> HistoryModel::mostVisited(int count)
{
    QList<HistoryEntry> list;
    QSqlQuery query;
    query.exec(QString("SELECT count, date, id, title, url FROM history ORDER BY count DESC LIMIT %1").arg(count));
    while (query.next()) {
        HistoryEntry entry;
        entry.count = query.value(0).toInt();
        entry.date = query.value(1).toDateTime();
        entry.id = query.value(2).toInt();
        entry.title = query.value(3).toString();
        entry.url = query.value(4).toUrl();
        list.append(entry);
    }
    return list;
}

bool HistoryModel::optimizeHistory()
{
    QSqlQuery query;
    return query.exec("VACUUM");
}

bool HistoryModel::clearHistory()
{
    QSqlQuery query;
    if (query.exec("DELETE FROM history")) {
        emit historyClear();
        return true;
    }
    return false;
}

void HistoryModel::setSaving(bool state)
{
    m_isSaving = state;
}

bool HistoryModel::isSaving()
{
    return m_isSaving;
}

QString HistoryModel::titleCaseLocalizedMonth(int month)
{
    switch (month) {
    case 1:
        return tr("January");
    case 2:
        return tr("February");
    case 3:
        return tr("March");
    case 4:
        return tr("April");
    case 5:
        return tr("May");
    case 6:
        return tr("June");
    case 7:
        return tr("July");
    case 8:
        return tr("August");
    case 9:
        return tr("September");
    case 10:
        return tr("October");
    case 11:
        return tr("November");
    case 12:
        return tr("December");
    default:
        qWarning("Month number out of range!");
        return QString();
    }
}
