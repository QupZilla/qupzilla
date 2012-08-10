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
    QString searchString = QString("%%1%").arg(string);
    QList<QUrl> urlList;
    QSqlQuery query;

    if (showType == HistoryAndBookmarks || showType == Bookmarks) {
        query.prepare("SELECT id, url, title, icon FROM bookmarks WHERE title LIKE ? OR url LIKE ? LIMIT ?");
        query.addBindValue(searchString);
        query.addBindValue(searchString);
        query.addBindValue(limit);
        query.exec();

        while (query.next()) {
            QStandardItem* item = new QStandardItem();
            const QUrl &url = query.value(1).toUrl();

            item->setIcon(qIconProvider->iconFromImage(QImage::fromData(query.value(3).toByteArray())));
            item->setText(url.toEncoded());
            item->setData(query.value(0), IdRole);
            item->setData(query.value(2), TitleRole);
            item->setData(QVariant(true), BookmarkRole);
            appendRow(item);
            urlList.append(url);
        }

        limit -= query.size();
    }

    if (showType == HistoryAndBookmarks || showType == History) {
        query.prepare("SELECT id, url, title FROM history WHERE title LIKE ? OR url LIKE ? ORDER BY count DESC LIMIT ?");
        query.addBindValue(searchString);
        query.addBindValue(searchString);
        query.addBindValue(limit);
        query.exec();

        while (query.next()) {
            QStandardItem* item = new QStandardItem();
            const QUrl &url = query.value(1).toUrl();

            if (urlList.contains(url)) {
                continue;
            }

            item->setIcon(_iconForUrl(url));
            item->setText(url.toEncoded());
            item->setData(query.value(0), IdRole);
            item->setData(query.value(2), TitleRole);
            item->setData(QVariant(false), BookmarkRole);

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
