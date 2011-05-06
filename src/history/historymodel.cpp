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
#include "historymodel.h"
#include "webview.h"
#include "qupzilla.h"

HistoryModel::HistoryModel(QupZilla* mainClass, QObject* parent)
    : QObject(parent)
    ,m_isSaving(true)
    ,p_QupZilla(mainClass)
{
    loadSettings();
}

void HistoryModel::loadSettings()
{
    QSettings settings(mApp->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    m_isSaving = settings.value("allowHistory",true).toBool();
    settings.endGroup();
}

int HistoryModel::addHistoryEntry(const QString &url, QString &title)
{
    if (!m_isSaving)
        return -2;
    if (url.contains("file://") || title.contains(tr("Failed loading page")) || url.isEmpty() || url.contains("about:blank") )
        return -1;
    if (title == "")
        title=tr("No Named Page");

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
    } else {
        query.prepare("UPDATE history SET count = count + 1, date=?, title=? WHERE url=?");
        query.bindValue(0, QDateTime::currentMSecsSinceEpoch());
        query.bindValue(1, title);
        query.bindValue(2, url);
        query.exec();
    }
    return query.lastInsertId().toInt();
}

int HistoryModel::addHistoryEntry(WebView* view)
{
    if (!m_isSaving)
        return -2;

    QString url = view->url().toString();
    QString title = view->title();
    return addHistoryEntry(url, title);
}

bool HistoryModel::deleteHistoryEntry(int index)
{
    QSqlQuery query;
    query.prepare("SELECT id, count, date, url, title FROM history WHERE id=?");
    query.bindValue(0, index);
    query.exec();
    if (!query.next())
        return false;
    HistoryEntry entry;
    entry.id = query.value(0).toInt();
    entry.count = query.value(1).toInt();
    entry.date = QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong());
    entry.url = query.value(3).toUrl();
    entry.title = query.value(4).toString();

    query.prepare("DELETE FROM history WHERE id=?");
    query.bindValue(0, index);
    if (query.exec()) {
        emit historyEntryDeleted(entry);
        return true;
    }
    return false;
}

bool HistoryModel::deleteHistoryEntry(const QString &url, const QString &title)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM history WHERE url=? AND title=?");
    query.bindValue(0, url);
    query.bindValue(1, title);
    query.exec();
    if (query.next()) {
        int id = query.value(0).toInt();
        return deleteHistoryEntry(id);
    }
    return false;
}

QList<HistoryModel::HistoryEntry> HistoryModel::mostVisited(int count)
{
    QList<HistoryEntry> list;
    QSqlQuery query;
    query.exec(QString("SELECT count, date, id, title, url FROM history ORDER BY count DESC LIMIT %1").arg(count));
    while(query.next()) {
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
