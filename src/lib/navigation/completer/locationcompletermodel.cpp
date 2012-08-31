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
#include "locationcompletermodel.h"
#include "iconprovider.h"
#include "qzsettings.h"
#include "mainapplication.h"

#include <QSqlQuery>

LocationCompleterModel::LocationCompleterModel(QObject* parent)
    : QStandardItemModel(parent)
    , m_lastCompletion(QChar(QChar::Nbsp))
{
}

void LocationCompleterModel::refreshCompletions(const QString &string)
{
    if (m_lastCompletion == string) {
        return;
    }

    m_lastCompletion = string;

    if (string.isEmpty()) {
        showMostVisited();
        return;
    }

    clear();

    Type showType = (Type) qzSettings->showLocationSuggestions;

    int limit = string.size() < 3 ? 25 : 15;
    QList<QUrl> urlList;

    if (showType == HistoryAndBookmarks || showType == Bookmarks) {
        QSqlQuery query = createQuery(string, QString("history.count DESC"), urlList, limit, true, false);
        query.exec();

        while (query.next()) {
            QStandardItem* item = new QStandardItem();
            const QUrl &url = query.value(1).toUrl();

            item->setIcon(qIconProvider->iconFromImage(QImage::fromData(query.value(3).toByteArray())));
            item->setText(url.toEncoded());
            item->setData(query.value(0), IdRole);
            item->setData(query.value(2), TitleRole);
            item->setData(QVariant(true), BookmarkRole);
            item->setData(string, SearchStringRole);
            appendRow(item);
            urlList.append(url);
        }

        limit -= query.size();
    }

    if (showType == HistoryAndBookmarks || showType == History) {
        QSqlQuery query = createQuery(string, "count DESC", urlList, limit, false, false);
        query.exec();

        while (query.next()) {
            QStandardItem* item = new QStandardItem();
            const QUrl &url = query.value(1).toUrl();

            item->setIcon(_iconForUrl(url));
            item->setText(url.toEncoded());
            item->setData(query.value(0), IdRole);
            item->setData(query.value(2), TitleRole);
            item->setData(QVariant(false), BookmarkRole);
            item->setData(string, SearchStringRole);

            appendRow(item);
        }
    }
}

void LocationCompleterModel::showMostVisited()
{
    clear();

    QSqlQuery query;
    query.exec("SELECT id, url, title FROM history ORDER BY count DESC LIMIT 15");

    while (query.next()) {
        QStandardItem* item = new QStandardItem();
        const QUrl &url = query.value(1).toUrl();

        item->setIcon(_iconForUrl(url));
        item->setText(url.toEncoded());
        item->setData(query.value(0), IdRole);
        item->setData(query.value(2), TitleRole);
        item->setData(QVariant(false), BookmarkRole);

        appendRow(item);
    }
}

QSqlQuery LocationCompleterModel::createQuery(QString searchString, QString orderBy, const QList<QUrl> &alreadyFound, int limit, bool bookmarks, bool exactMatch)
{
    QString table = bookmarks ? "bookmarks" : "history";
    QString query = QString("SELECT %1.id, %1.url, %1.title").arg(table);
    QStringList searchList;

    if (bookmarks) {
        query.append(", bookmarks.icon FROM bookmarks LEFT JOIN history ON bookmarks.url=history.url ");
    }
    else {
        query.append(" FROM history ");
    }

    query.append("WHERE ");
    if (exactMatch) {
        query.append(QString("%1.title LIKE ? OR %1.url LIKE ? ").arg(table));
    }
    else {
        searchList = searchString.split(' ', QString::SkipEmptyParts);
        const int slSize = searchList.size();
        for (int i = 0; i < slSize; ++i) {
            query.append(QString("(%1.title LIKE ? OR %1.url LIKE ?) ").arg(table));
            if (i < slSize - 1) {
                query.append("AND ");
            }
        }
    }

    for (int i = 0; i < alreadyFound.count(); i++) {
        query.append(QString("AND (NOT %1.url=?) ").arg(table));
    }

    if (!orderBy.isEmpty()) {
        query.append("ORDER BY " + orderBy);
    }

    query.append(" LIMIT ?");

    QSqlQuery sqlQuery;
    sqlQuery.prepare(query);

    if (exactMatch) {
        sqlQuery.addBindValue(QString("%%1%").arg(searchString));
        sqlQuery.addBindValue(QString("%%1%").arg(searchString));
    }
    else {
        foreach(const QString & str, searchList) {
            sqlQuery.addBindValue(QString("%%1%").arg(str));
            sqlQuery.addBindValue(QString("%%1%").arg(str));
        }
    }

    foreach(const QUrl & url, alreadyFound) {
        sqlQuery.addBindValue(url);
    }

    sqlQuery.addBindValue(limit);

    return sqlQuery;
}
