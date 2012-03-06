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
#ifndef BOOKMARKSIMPORTICONFETCHER_H
#define BOOKMARKSIMPORTICONFETCHER_H

#include <QObject>
#include <QUrl>
#include <QList>

#include "qz_namespace.h"

class QNetworkAccessManager;
class QTreeWidgetItem;
class QImage;

class IconFetcher;

class QT_QUPZILLA_EXPORT BookmarksImportIconFetcher : public QObject
{
    Q_OBJECT
public:
    struct Pair {
        QUrl url;
        QTreeWidgetItem* item;
    };

    explicit BookmarksImportIconFetcher(QObject* parent = 0);

    void addEntry(const QUrl &url, QTreeWidgetItem* item);
    void startFetching();

signals:
    void iconFetched(const QImage &image, QTreeWidgetItem* item);
    void oneFinished();

private slots:
    void slotStartFetching();

    void slotIconFetched(const QImage &image);
    void slotFetcherFinished();

private:
    QList<Pair> m_pairs;
    QList<IconFetcher*> m_fetchers;

};

#endif // BOOKMARKSIMPORTICONFETCHER_H
