/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#include "locationcompleterrefreshjob.h"
#include "locationcompletermodel.h"
#include "mainapplication.h"
#include "bookmarkitem.h"
#include "sqldatabase.h"
#include "qzsettings.h"
#include "bookmarks.h"

#include <QDateTime>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrentRun>
#else
#include <QtConcurrentRun>
#endif

QHash<QString, QByteArray> LocationCompleterRefreshJob::m_iconCache;

LocationCompleterRefreshJob::LocationCompleterRefreshJob(const QString &searchString)
    : QObject()
    , m_timestamp(QDateTime::currentMSecsSinceEpoch())
    , m_searchString(searchString)
    , m_jobCancelled(false)
{
    m_watcher = new QFutureWatcher<void>(this);
    connect(m_watcher, SIGNAL(finished()), this, SLOT(slotFinished()));

    QFuture<void> future = QtConcurrent::run(this, &LocationCompleterRefreshJob::runJob);
    m_watcher->setFuture(future);
}

qint64 LocationCompleterRefreshJob::timestamp() const
{
    return m_timestamp;
}

QString LocationCompleterRefreshJob::searchString() const
{
    return m_searchString;
}

QList<QStandardItem*> LocationCompleterRefreshJob::completions() const
{
    return m_items;
}

QString LocationCompleterRefreshJob::domainCompletion() const
{
    return m_domainCompletion;
}

void LocationCompleterRefreshJob::jobCancelled()
{
    m_jobCancelled = true;
}

void LocationCompleterRefreshJob::slotFinished()
{
    emit finished();
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

void LocationCompleterRefreshJob::runJob()
{
    if (m_jobCancelled || mApp->isClosing() || !mApp) {
        return;
    }

    if (m_searchString.isEmpty()) {
        completeMostVisited();
    }
    else {
        completeFromHistory();
    }

    // Load all icons into QImage
    QSqlQuery query;
    query.prepare(QSL("SELECT icon FROM icons WHERE url LIKE ? ESCAPE \'!\' LIMIT 1"));

    foreach (QStandardItem* item, m_items) {
        const QUrl url = item->data(LocationCompleterModel::UrlRole).toUrl();
        QString urlString = QString::fromUtf8(url.toEncoded(QUrl::RemoveFragment));

        // escaped sqlite wildcards
        urlString.replace("!", "!!");
        urlString.replace("%", "!%");
        urlString.replace("_", "!_");

        query.bindValue(0, QString(QL1S("%1%")).arg(QString::fromUtf8(url.toEncoded(QUrl::RemoveFragment))));

        if (m_iconCache.contains(urlString)) {
            item->setData(QImage::fromData(m_iconCache.value(urlString)), LocationCompleterModel::ImageRole);
        }
        else {
            if (m_jobCancelled) {
                return;
            }
            query.bindValue(0, QString(QL1S("%1%")).arg(urlString));
            QSqlQuery res = SqlDatabase::instance()->exec(query);
            if (res.next()) {
                const QByteArray &iconData = res.value(0).toByteArray();
                m_iconCache.insert(urlString, iconData);
                item->setData(QImage::fromData(iconData), LocationCompleterModel::ImageRole);
            }
            else {
                m_iconCache.insert(urlString, QByteArray());
            }
        }
    }

    // Sort by count
    qSort(m_items.begin(), m_items.end(), countBiggerThan);

    // Get domain completion
    if (!m_searchString.isEmpty() && qzSettings->useInlineCompletion) {
        QSqlQuery domainQuery = LocationCompleterModel::createDomainQuery(m_searchString);
        if (domainQuery.lastQuery().isEmpty()) {
            return;
        }

        QSqlQuery res = SqlDatabase::instance()->exec(domainQuery);
        res.exec();
        if (res.next()) {
            m_domainCompletion = createDomainCompletion(res.value(0).toUrl().host());
        }
    }
}

void LocationCompleterRefreshJob::completeFromHistory()
{
    QList<QUrl> urlList;
    Type showType = (Type) qzSettings->showLocationSuggestions;

    // Search in bookmarks
    if (showType == HistoryAndBookmarks || showType == Bookmarks) {
        const int bookmarksLimit = 10;
        QList<BookmarkItem*> bookmarks = mApp->bookmarks()->searchBookmarks(m_searchString, bookmarksLimit);

        foreach (BookmarkItem* bookmark, bookmarks) {
            Q_ASSERT(bookmark->isUrl());

            QStandardItem* item = new QStandardItem();
            item->setText(bookmark->url().toEncoded());
            item->setData(-1, LocationCompleterModel::IdRole);
            item->setData(bookmark->title(), LocationCompleterModel::TitleRole);
            item->setData(bookmark->url(), LocationCompleterModel::UrlRole);
            item->setData(bookmark->visitCount(), LocationCompleterModel::CountRole);
            item->setData(QVariant(true), LocationCompleterModel::BookmarkRole);
            item->setData(QVariant::fromValue<void*>(static_cast<void*>(bookmark)), LocationCompleterModel::BookmarkItemRole);
            item->setData(m_searchString, LocationCompleterModel::SearchStringRole);

            urlList.append(bookmark->url());
            m_items.append(item);
        }
    }

    // Search in history
    if (showType == HistoryAndBookmarks || showType == History) {
        const int historyLimit = 20;
        QSqlQuery query = LocationCompleterModel::createHistoryQuery(m_searchString, historyLimit);
        QSqlQuery res = SqlDatabase::instance()->exec(query);

        while (res.next()) {
            const QUrl url = res.value(1).toUrl();

            if (urlList.contains(url)) {
                continue;
            }

            QStandardItem* item = new QStandardItem();
            item->setText(url.toEncoded());
            item->setData(res.value(0), LocationCompleterModel::IdRole);
            item->setData(res.value(2), LocationCompleterModel::TitleRole);
            item->setData(url, LocationCompleterModel::UrlRole);
            item->setData(res.value(3), LocationCompleterModel::CountRole);
            item->setData(QVariant(false), LocationCompleterModel::BookmarkRole);
            item->setData(m_searchString, LocationCompleterModel::SearchStringRole);

            m_items.append(item);
        }
    }
}

void LocationCompleterRefreshJob::completeMostVisited()
{
    QSqlQuery query(QSL("SELECT id, url, title FROM history ORDER BY count DESC LIMIT 15"));
    QSqlQuery res = SqlDatabase::instance()->exec(query);

    while (res.next()) {
        QStandardItem* item = new QStandardItem();
        const QUrl url = res.value(1).toUrl();

        item->setText(url.toEncoded());
        item->setData(res.value(0), LocationCompleterModel::IdRole);
        item->setData(res.value(2), LocationCompleterModel::TitleRole);
        item->setData(url, LocationCompleterModel::UrlRole);
        item->setData(QVariant(false), LocationCompleterModel::BookmarkRole);

        m_items.append(item);
    }
}

QString LocationCompleterRefreshJob::createDomainCompletion(const QString &completion) const
{
    // Make sure search string and completion matches

    if (m_searchString.startsWith(QL1S("www.")) && !completion.startsWith(QL1S("www."))) {
        return QL1S("www.") + completion;
    }

    if (!m_searchString.startsWith(QL1S("www.")) && completion.startsWith(QL1S("www."))) {
        return completion.mid(4);
    }

    return completion;
}
