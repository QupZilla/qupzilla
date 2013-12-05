/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "qupzilla.h"
#include "tabwidget.h"

#include <QSqlQuery>

LocationCompleterModel::LocationCompleterModel(QObject* parent)
    : QStandardItemModel(parent)
    , m_lastCompletion(QChar(QChar::Nbsp))
{
}

static bool countBiggerThan(const QStandardItem* i1, const QStandardItem* i2)
{
    return i1->data(LocationCompleterModel::CountRole).toInt() >
           i2->data(LocationCompleterModel::CountRole).toInt();
}

void LocationCompleterModel::refreshCompletions(const QString &string)
{
    if (m_lastCompletion == string) {
        refreshTabPositions();
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
    QList<QStandardItem*> itemList;

    // TODO: Those 2 SQL queries can be merged with UNION

    if (showType == HistoryAndBookmarks || showType == Bookmarks) {
        QSqlQuery query = createQuery(string, "history.count DESC", urlList, limit, true);
        query.exec();

        while (query.next()) {
            QStandardItem* item = new QStandardItem();
            const QUrl &url = query.value(1).toUrl();

            item->setIcon(qIconProvider->iconFromImage(QImage::fromData(query.value(4).toByteArray())));
            item->setText(url.toEncoded());
            item->setData(query.value(0), IdRole);
            item->setData(query.value(2), TitleRole);
            item->setData(query.value(3), CountRole);
            item->setData(QVariant(true), BookmarkRole);
            item->setData(string, SearchStringRole);
            if (qzSettings->showSwitchTab) {
                item->setData(QVariant::fromValue<TabPosition>(tabPositionForUrl(url)), TabPositionRole);
            }

            urlList.append(url);
            itemList.append(item);
        }

        limit -= query.size();
    }

    if (showType == HistoryAndBookmarks || showType == History) {
        QSqlQuery query = createQuery(string, "count DESC", urlList, limit);
        query.exec();

        while (query.next()) {
            QStandardItem* item = new QStandardItem();
            const QUrl &url = query.value(1).toUrl();

            item->setIcon(_iconForUrl(url));
            item->setText(url.toEncoded());
            item->setData(query.value(0), IdRole);
            item->setData(query.value(2), TitleRole);
            item->setData(query.value(3), CountRole);
            item->setData(QVariant(false), BookmarkRole);
            item->setData(string, SearchStringRole);
            if (qzSettings->showSwitchTab) {
                item->setData(QVariant::fromValue<TabPosition>(tabPositionForUrl(url)), TabPositionRole);
            }

            itemList.append(item);
        }
    }

    // Sort by count
    qSort(itemList.begin(), itemList.end(), countBiggerThan);

    appendColumn(itemList);
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
        if (qzSettings->showSwitchTab) {
            item->setData(QVariant::fromValue<TabPosition>(tabPositionForUrl(url)), TabPositionRole);
        }

        appendRow(item);
    }
}

QString LocationCompleterModel::completeDomain(const QString &text)
{
    if (text.isEmpty() || text == QLatin1String("www.")) {
        return QString();
    }

    bool withoutWww = text.startsWith(QLatin1Char('w')) && !text.startsWith(QLatin1String("www."));
    QString query = "SELECT url FROM history WHERE ";

    if (withoutWww) {
        query.append(QLatin1String("url NOT LIKE ? AND url NOT LIKE ? AND "));
    }
    else {
        query.append(QLatin1String("url LIKE ? OR url LIKE ? OR "));
    }

    query.append(QLatin1String("(url LIKE ? OR url LIKE ?) ORDER BY count DESC LIMIT 1"));

    QSqlQuery sqlQuery;
    sqlQuery.prepare(query);

    if (withoutWww) {
        sqlQuery.addBindValue(QString("http://www.%"));
        sqlQuery.addBindValue(QString("https://www.%"));
        sqlQuery.addBindValue(QString("http://%1%").arg(text));
        sqlQuery.addBindValue(QString("https://%1%").arg(text));
    }
    else {
        sqlQuery.addBindValue(QString("http://%1%").arg(text));
        sqlQuery.addBindValue(QString("https://%1%").arg(text));
        sqlQuery.addBindValue(QString("http://www.%1%").arg(text));
        sqlQuery.addBindValue(QString("https://www.%1%").arg(text));
    }

    sqlQuery.exec();

    if (!sqlQuery.next()) {
        return QString();
    }

    return sqlQuery.value(0).toUrl().host();
}

QSqlQuery LocationCompleterModel::createQuery(const QString &searchString, const QString &orderBy,
        const QList<QUrl> &alreadyFound, int limit, bool bookmarks, bool exactMatch)
{
    QString table = bookmarks ? "bookmarks" : "history";
    QString query = QString("SELECT %1.id, %1.url, %1.title, history.count").arg(table);
    QStringList searchList;

    if (bookmarks) {
        query.append(QLatin1String(", bookmarks.icon FROM bookmarks LEFT JOIN history ON bookmarks.url=history.url "));
    }
    else {
        query.append(QLatin1String(" FROM history "));
    }

    query.append(QLatin1String("WHERE "));
    if (exactMatch) {
        query.append(QString("%1.title LIKE ? OR %1.url LIKE ? ").arg(table));
    }
    else {
        searchList = searchString.split(QLatin1Char(' '), QString::SkipEmptyParts);
        const int slSize = searchList.size();
        for (int i = 0; i < slSize; ++i) {
            query.append(QString("(%1.title LIKE ? OR %1.url LIKE ?) ").arg(table));
            if (i < slSize - 1) {
                query.append(QLatin1String("AND "));
            }
        }
    }

    for (int i = 0; i < alreadyFound.count(); i++) {
        query.append(QString("AND (NOT %1.url=?) ").arg(table));
    }

    query.append(QString("GROUP BY %1.url ").arg(table));

    if (!orderBy.isEmpty()) {
        query.append("ORDER BY " + orderBy);
    }

    query.append(QLatin1String(" LIMIT ?"));

    QSqlQuery sqlQuery;
    sqlQuery.prepare(query);

    if (exactMatch) {
        sqlQuery.addBindValue(QString("%%1%").arg(searchString));
        sqlQuery.addBindValue(QString("%%1%").arg(searchString));
    }
    else {
        foreach (const QString &str, searchList) {
            sqlQuery.addBindValue(QString("%%1%").arg(str));
            sqlQuery.addBindValue(QString("%%1%").arg(str));
        }
    }

    foreach (const QUrl &url, alreadyFound) {
        sqlQuery.addBindValue(url);
    }

    sqlQuery.addBindValue(limit);

    return sqlQuery;
}

TabPosition LocationCompleterModel::tabPositionForUrl(const QUrl &url) const
{
    return tabPositionForEncodedUrl(url.toEncoded());
}

TabPosition LocationCompleterModel::tabPositionForEncodedUrl(const QString &encodedUrl) const
{
    QList<QupZilla*> windows = mApp->mainWindows();
    int currentWindowIdx = windows.indexOf(mApp->getWindow());
    windows.prepend(mApp->getWindow());
    for (int win = 0; win < windows.count(); ++win) {
        QupZilla* mainWin = windows.at(win);
        QList<WebTab*> tabs = mainWin->tabWidget()->allTabs();
        for (int tab = 0; tab < tabs.count(); ++tab) {
            if (tabs[tab]->url().toEncoded() == encodedUrl) {
                TabPosition pos;
                pos.windowIndex = win == 0 ? currentWindowIdx : win - 1;
                pos.tabIndex = tab;
                return pos;
            }
        }
    }
    return TabPosition();
}

void LocationCompleterModel::refreshTabPositions()
{
    if (!qzSettings->showSwitchTab) {
        return;
    }

    for (int row = 0; row < rowCount(); ++row) {
        QStandardItem* aItem = item(row);
        if (!aItem) {
            continue;
        }
        aItem->setData(QVariant::fromValue<TabPosition>(tabPositionForEncodedUrl(aItem->text())), TabPositionRole);
    }
}
