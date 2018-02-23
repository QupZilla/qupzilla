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
#include "history.h"
#include "historymodel.h"
#include "browserwindow.h"
#include "iconprovider.h"
#include "settings.h"
#include "mainapplication.h"
#include "sqldatabase.h"
#include "webview.h"

#include <QWebEngineProfile>

History::History(QObject* parent)
    : QObject(parent)
    , m_isSaving(true)
    , m_model(0)
{
    loadSettings();
}

HistoryModel* History::model()
{
    if (!m_model) {
        m_model = new HistoryModel(this);
    }

    return m_model;
}

void History::loadSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    m_isSaving = settings.value("allowHistory", true).toBool();
    settings.endGroup();
}

// AddHistoryEntry
void History::addHistoryEntry(WebView* view)
{
    if (!m_isSaving) {
        return;
    }

    const QUrl url = view->url();
    const QString title = view->title();

    addHistoryEntry(url, title);
}

void History::addHistoryEntry(const QUrl &url, QString title)
{
    if (!m_isSaving) {
        return;
    }

    const QStringList schemes = {
        QSL("http"), QSL("https"), QSL("ftp"), QSL("file")
    };

    if (!schemes.contains(url.scheme())) {
        return;
    }

    if (title.isEmpty()) {
        title = tr("Empty Page");
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT id, count, date, title FROM history WHERE url=?");
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
        entry.urlString = url.toEncoded();
        entry.title = title;
        emit historyEntryAdded(entry);
    }
    else {
        int id = query.value(0).toInt();
        int count = query.value(1).toInt();
        QDateTime date = QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong());
        QString oldTitle = query.value(3).toString();

        query.prepare("UPDATE history SET count = count + 1, date=?, title=? WHERE url=?");
        query.bindValue(0, QDateTime::currentMSecsSinceEpoch());
        query.bindValue(1, title);
        query.bindValue(2, url);
        query.exec();

        HistoryEntry before;
        before.id = id;
        before.count = count;
        before.date = date;
        before.url = url;
        before.urlString = url.toEncoded();
        before.title = oldTitle;

        HistoryEntry after = before;
        after.count = count + 1;
        after.date = QDateTime::currentDateTime();
        after.title = title;

        emit historyEntryEdited(before, after);
    }
}

// DeleteHistoryEntry
void History::deleteHistoryEntry(int index)
{
    QList<int> list;
    list.append(index);

    deleteHistoryEntry(list);
}

void History::deleteHistoryEntry(const QList<int> &list)
{
    QSqlDatabase db = SqlDatabase::instance()->database();
    db.transaction();

    QList<QUrl> urls;

    foreach (int index, list) {
        QSqlQuery query(SqlDatabase::instance()->database());
        query.prepare("SELECT count, date, url, title FROM history WHERE id=?");
        query.addBindValue(index);
        query.exec();

        if (!query.isActive() || !query.next()) {
            continue;
        }

        HistoryEntry entry;
        entry.id = index;
        entry.count = query.value(0).toInt();
        entry.date = QDateTime::fromMSecsSinceEpoch(query.value(1).toLongLong());
        entry.url = query.value(2).toUrl();
        entry.urlString = entry.url.toEncoded();
        entry.title = query.value(3).toString();

        query.prepare("DELETE FROM history WHERE id=?");
        query.addBindValue(index);
        query.exec();

        query.prepare("DELETE FROM icons WHERE url=?");
        query.addBindValue(entry.url.toEncoded(QUrl::RemoveFragment));
        query.exec();

        urls.append(entry.url);
        emit historyEntryDeleted(entry);
    }

    mApp->webProfile()->clearVisitedLinks(urls);

    db.commit();
}

void History::deleteHistoryEntry(const QString &url, const QString &title)
{
    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT id FROM history WHERE url=? AND title=?");
    query.bindValue(0, url);
    query.bindValue(1, title);
    query.exec();
    if (query.next()) {
        int id = query.value(0).toInt();
        deleteHistoryEntry(id);
    }
}

QList<int> History::indexesFromTimeRange(qint64 start, qint64 end)
{
    QList<int> list;

    if (start < 0 || end < 0) {
        return list;
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT id FROM history WHERE date BETWEEN ? AND ?");
    query.addBindValue(end);
    query.addBindValue(start);
    query.exec();

    while (query.next()) {
        list.append(query.value(0).toInt());
    }

    return list;
}

bool History::urlIsStored(const QString &url)
{
    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT id FROM history WHERE url=?");
    query.bindValue(0, url);
    query.exec();
    return query.next();
}

QVector<HistoryEntry> History::mostVisited(int count)
{
    QVector<HistoryEntry> list;
    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare(QString("SELECT count, date, id, title, url FROM history ORDER BY count DESC LIMIT %1").arg(count));
    query.exec();
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

void History::clearHistory()
{
    QSqlQuery query(SqlDatabase::instance()->database());
    query.exec(QSL("DELETE FROM history"));
    query.exec(QSL("VACUUM"));

    mApp->webProfile()->clearAllVisitedLinks();

    emit resetHistory();
}

void History::setSaving(bool state)
{
    m_isSaving = state;
}

bool History::isSaving()
{
    return m_isSaving;
}

QString History::titleCaseLocalizedMonth(int month)
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
