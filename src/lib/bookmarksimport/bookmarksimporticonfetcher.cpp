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
#include "bookmarksimporticonfetcher.h"
#include "iconfetcher.h"

#include <QTimer>
#include <QNetworkAccessManager>

BookmarksImportIconFetcher::BookmarksImportIconFetcher(QObject* parent)
    : QObject(parent)
{
}

void BookmarksImportIconFetcher::addEntry(const QUrl &url, QTreeWidgetItem* item)
{
    Pair pair;
    pair.url = url;
    pair.item = item;

    m_pairs.append(pair);
}

void BookmarksImportIconFetcher::startFetching()
{
    QTimer::singleShot(0, this, SLOT(slotStartFetching()));
}

void BookmarksImportIconFetcher::slotIconFetched(const QImage &image)
{
    IconFetcher* fetcher = qobject_cast<IconFetcher*>(sender());
    if (!fetcher) {
        return;
    }

    QTreeWidgetItem* itemPointer = static_cast<QTreeWidgetItem*>(fetcher->data().value<void*>());

    emit iconFetched(image, itemPointer);
}

void BookmarksImportIconFetcher::slotFetcherFinished()
{
    IconFetcher* fetcher = qobject_cast<IconFetcher*>(sender());
    if (!fetcher) {
        return;
    }

    m_fetchers.removeOne(fetcher);

    emit oneFinished();
}

void BookmarksImportIconFetcher::slotStartFetching()
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    foreach(const Pair & pair, m_pairs) {
        QVariant itemPointer = QVariant::fromValue((void*) pair.item);

        IconFetcher* fetcher = new IconFetcher(this);
        fetcher->setNetworkAccessManager(manager);
        fetcher->setData(itemPointer);
        fetcher->fetchIcon(pair.url);

        connect(fetcher, SIGNAL(iconFetched(QImage)), this, SLOT(slotIconFetched(QImage)));
        connect(fetcher, SIGNAL(finished()), this, SLOT(slotFetcherFinished()));

        m_fetchers.append(fetcher);
    }
}
