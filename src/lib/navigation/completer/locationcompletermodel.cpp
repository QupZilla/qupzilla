/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#include "sqldatabase.h"

LocationCompleterModel::LocationCompleterModel(QObject* parent)
    : QStandardItemModel(parent)
{
}

void LocationCompleterModel::setCompletions(const QList<QStandardItem*> &items)
{
    clear();
    addCompletions(items);
}

void LocationCompleterModel::addCompletions(const QList<QStandardItem*> &items)
{
    for (QStandardItem *item : items) {
        item->setIcon(QPixmap::fromImage(item->data(ImageRole).value<QImage>()));
        setTabPosition(item);
        if (item->icon().isNull()) {
            item->setIcon(IconProvider::emptyWebIcon());
        }
        appendRow(QList<QStandardItem*>{item});
    }
}

QList<QStandardItem*> LocationCompleterModel::suggestionItems() const
{
    QList<QStandardItem*> items;
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *it = item(i);
        if (it->data(SearchSuggestionRole).toBool()) {
            items.append(it);
        }
    }
    return items;
}

QSqlQuery LocationCompleterModel::createDomainQuery(const QString &text)
{
    if (text.isEmpty() || text == QLatin1String("www.")) {
        return QSqlQuery(SqlDatabase::instance()->database());
    }

    bool withoutWww = text.startsWith(QLatin1Char('w')) && !text.startsWith(QLatin1String("www."));
    QString query = "SELECT url FROM history WHERE ";

    if (withoutWww) {
        query.append(QLatin1String("url NOT LIKE ? AND url NOT LIKE ? AND "));
    }
    else {
        query.append(QLatin1String("url LIKE ? OR url LIKE ? OR "));
    }

    query.append(QLatin1String("(url LIKE ? OR url LIKE ?) ORDER BY date DESC LIMIT 1"));

    QSqlQuery sqlQuery(SqlDatabase::instance()->database());
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

    return sqlQuery;
}

QSqlQuery LocationCompleterModel::createHistoryQuery(const QString &searchString, int limit, bool exactMatch)
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

    query.append(QLatin1String("ORDER BY date DESC LIMIT ?"));

    QSqlQuery sqlQuery(SqlDatabase::instance()->database());
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

    item->setData(-1, TabPositionTabRole);

    if (!qzSettings->showSwitchTab || item->data(VisitSearchItemRole).toBool()) {
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
