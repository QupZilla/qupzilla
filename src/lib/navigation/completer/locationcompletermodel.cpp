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
#include "locationcompletermodel.h"
#include "mainapplication.h"
#include "iconprovider.h"
#include "bookmarkitem.h"
#include "bookmarks.h"
#include "qzsettings.h"
#include "browserwindow.h"
#include "tabwidget.h"

#include <QSqlQuery>

LocationCompleterModel::LocationCompleterModel(QObject* parent)
    : QStandardItemModel(parent)
    , m_lastCompletion(QChar(QChar::Nbsp))
{
}

static bool countBiggerThan(const QStandardItem* i1, const QStandardItem* i2)
{
    // Move bookmarks up
    bool i1Bookmark = i1->data(LocationCompleterModel::BookmarkRole).toBool();
    bool i2Bookmark = i2->data(LocationCompleterModel::BookmarkRole).toBool();

    if (i1Bookmark && i2Bookmark) {
        return i1->data(LocationCompleterModel::CountRole).toInt() >
               i2->data(LocationCompleterModel::CountRole).toInt();
    }
    return i1Bookmark;
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

    QList<QUrl> urlList;
    QList<QStandardItem*> itemList;
    Type showType = (Type) qzSettings->showLocationSuggestions;

    if (showType == HistoryAndBookmarks || showType == Bookmarks) {
        const int bookmarksLimit = 10;
        QList<BookmarkItem*> bookmarks = mApp->bookmarks()->searchBookmarks(string, bookmarksLimit);

        foreach (BookmarkItem* bookmark, bookmarks) {
            Q_ASSERT(bookmark->isUrl());

            QStandardItem* item = new QStandardItem();
            item->setIcon(bookmark->icon());
            item->setText(bookmark->url().toEncoded());
            item->setData(-1, IdRole);
            item->setData(bookmark->title(), TitleRole);
            item->setData(bookmark->url(), UrlRole);
            item->setData(bookmark->visitCount(), CountRole);
            item->setData(QVariant(true), BookmarkRole);
            item->setData(QVariant::fromValue<void*>(static_cast<void*>(bookmark)), BookmarkItemRole);
            item->setData(string, SearchStringRole);
            setTabPosition(item);

            urlList.append(bookmark->url());
            itemList.append(item);
        }
    }

    if (showType == HistoryAndBookmarks || showType == History) {
        const int historyLimit = 20;
        QSqlQuery query = createQuery(string, historyLimit);
        query.exec();

        while (query.next()) {
            const QUrl url = query.value(1).toUrl();

            if (urlList.contains(url)) {
                continue;
            }

            QStandardItem* item = new QStandardItem();
            item->setIcon(IconProvider::iconForUrl(url));
            item->setText(url.toEncoded());
            item->setData(query.value(0), IdRole);
            item->setData(query.value(2), TitleRole);
            item->setData(url, UrlRole);
            item->setData(query.value(3), CountRole);
            item->setData(QVariant(false), BookmarkRole);
            item->setData(string, SearchStringRole);
            setTabPosition(item);

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
        const QUrl url = query.value(1).toUrl();

        item->setIcon(IconProvider::iconForUrl(url));
        item->setText(url.toEncoded());
        item->setData(query.value(0), IdRole);
        item->setData(query.value(2), TitleRole);
        item->setData(url, UrlRole);
        item->setData(QVariant(false), BookmarkRole);
        setTabPosition(item);

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

QSqlQuery LocationCompleterModel::createQuery(const QString &searchString, int limit, bool exactMatch) const
{
    QStringList searchList;
    QString query = QLatin1String("SELECT id, url, title, count FROM history WHERE ");

    if (exactMatch) {
        query.append(QLatin1String("title LIKE ? OR url LIKE ? "));
    }
    else {
        searchList = searchString.split(QLatin1Char(' '), QString::SkipEmptyParts);
        const int slSize = searchList.size();
        for (int i = 0; i < slSize; ++i) {
            query.append(QLatin1String("(title LIKE ? OR url LIKE ?) "));
            if (i < slSize - 1) {
                query.append(QLatin1String("AND "));
            }
        }
    }

    query.append(QLatin1String("LIMIT ?"));

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

    sqlQuery.addBindValue(limit);

    return sqlQuery;
}

void LocationCompleterModel::setTabPosition(QStandardItem* item) const
{
    Q_ASSERT(item);

    if (!qzSettings->showSwitchTab) {
        return;
    }

    const QUrl url = item->data(UrlRole).toUrl();
    const QList<BrowserWindow*> windows = mApp->windows();

    foreach (BrowserWindow* window, windows) {
        QList<WebTab*> tabs = window->tabWidget()->allTabs();
        for (int i = 0; i < tabs.count(); ++i) {
            WebTab* tab = tabs.at(i);
            if (tab->url() == url) {
                item->setData(QVariant::fromValue<void*>(static_cast<void*>(window)), TabPositionWindowRole);
                item->setData(i, TabPositionTabRole);
                return;
            }
        }
    }

    // Tab wasn't found
    item->setData(QVariant::fromValue<void*>(static_cast<void*>(0)), TabPositionWindowRole);
    item->setData(-1, TabPositionTabRole);
}

void LocationCompleterModel::refreshTabPositions() const
{
    for (int row = 0; row < rowCount(); ++row) {
        QStandardItem* itm = item(row);
        if (itm) {
            setTabPosition(itm);
        }
    }
}
