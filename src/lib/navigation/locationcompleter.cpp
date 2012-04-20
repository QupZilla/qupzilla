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
#include "locationcompleter.h"
#include "locationcompleterdelegate.h"
#include "locationbar.h"
#include "iconprovider.h"
#include "mainapplication.h"

#include <QStandardItemModel>
#include <QSqlQuery>

LocationCompleter::LocationCompleter(QObject* parent)
    : QCompleter(parent)
{
    QStandardItemModel* completeModel = new QStandardItemModel();
    setModel(completeModel);

    m_listView = new CompleterListView();
    m_listView->setItemDelegateForColumn(0, new LocationCompleterDelegate(m_listView));
    setPopup(m_listView);

    setCompletionMode(QCompleter::PopupCompletion);
    setMaxVisibleItems(6);
}

QStringList LocationCompleter::splitPath(const QString &path) const
{
    Q_UNUSED(path);
    return QStringList();
}

void LocationCompleter::showMostVisited()
{
    QStandardItemModel* cModel = qobject_cast<QStandardItemModel*>(model());
    cModel->clear();

    QSqlQuery query;
    query.exec("SELECT url, title FROM history ORDER BY count DESC LIMIT 15");

    while (query.next()) {
        QStandardItem* item = new QStandardItem();
        const QUrl &url = query.value(0).toUrl();

        item->setIcon(_iconForUrl(url));
        item->setText(url.toEncoded());
        item->setData(query.value(1), Qt::UserRole);

        cModel->appendRow(item);
    }

    m_listView->setMinimumHeight(6 * m_listView->rowHeight());

    QCompleter::complete();
}

void LocationCompleter::refreshCompleter(const QString &string)
{
    int limit = string.size() < 3 ? 25 : 15;
    QString searchString = QString("%%1%").arg(string);
    QList<QUrl> urlList;

    QStandardItemModel* cModel = qobject_cast<QStandardItemModel*>(model());
    cModel->clear();

    QSqlQuery query;
    query.prepare("SELECT url, title, icon FROM bookmarks WHERE title LIKE ? OR url LIKE ? LIMIT ?");
    query.addBindValue(searchString);
    query.addBindValue(searchString);
    query.addBindValue(limit);
    query.exec();

    while (query.next()) {
        QStandardItem* item = new QStandardItem();
        const QUrl &url = query.value(0).toUrl();

        item->setText(url.toEncoded());
        item->setData(query.value(1), Qt::UserRole);
        item->setIcon(IconProvider::iconFromImage(QImage::fromData(query.value(2).toByteArray())));

        cModel->appendRow(item);
        urlList.append(url);
    }

    limit -= query.size();

    query.prepare("SELECT url, title FROM history WHERE title LIKE ? OR url LIKE ? ORDER BY count DESC LIMIT ?");
    query.addBindValue(searchString);
    query.addBindValue(searchString);
    query.addBindValue(limit);
    query.exec();

    while (query.next()) {
        QStandardItem* item = new QStandardItem();
        const QUrl &url = query.value(0).toUrl();

        if (urlList.contains(url)) {
            continue;
        }

        item->setIcon(_iconForUrl(url));
        item->setText(url.toEncoded());
        item->setData(query.value(1), Qt::UserRole);

        cModel->appendRow(item);
    }

    if (cModel->rowCount() > 6) {
        m_listView->setMinimumHeight(6 * m_listView->rowHeight());
    }
    else {
        m_listView->setMinimumHeight(0);
    }

    m_listView->setUpdatesEnabled(true);
}
