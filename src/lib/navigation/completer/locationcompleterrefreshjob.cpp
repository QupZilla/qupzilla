/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2018 David Rosca <nowrep@gmail.com>
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
#include "iconprovider.h"
#include "sqldatabase.h"
#include "qzsettings.h"
#include "bookmarks.h"
#include "qztools.h"

#include <algorithm>

#include <QDateTime>

#include <QtConcurrent/QtConcurrentRun>

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

bool LocationCompleterRefreshJob::isCanceled() const
{
    return m_jobCancelled;
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
    int i1Count = i1->data(LocationCompleterModel::CountRole).toInt();
    int i2Count = i2->data(LocationCompleterModel::CountRole).toInt();
    return i1Count > i2Count;
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
    foreach (QStandardItem* item, m_items) {
        if (m_jobCancelled) {
            return;
        }

        const QUrl url = item->data(LocationCompleterModel::UrlRole).toUrl();
        item->setData(IconProvider::imageForUrl(url), LocationCompleterModel::ImageRole);
    }

    if (m_jobCancelled) {
        return;
    }

    // Get domain completion
    if (!m_searchString.isEmpty() && qzSettings->useInlineCompletion) {
        QSqlQuery domainQuery = LocationCompleterModel::createDomainQuery(m_searchString);
        if (!domainQuery.lastQuery().isEmpty()) {
            domainQuery.exec();
            if (domainQuery.next()) {
                m_domainCompletion = createDomainCompletion(domainQuery.value(0).toUrl().host());
            }
        }
    }

    if (m_jobCancelled) {
        return;
    }

    // Add search/visit item
    if (!m_searchString.isEmpty()) {
        QStandardItem* item = new QStandardItem();
        item->setText(m_searchString);
        item->setData(m_searchString, LocationCompleterModel::UrlRole);
        item->setData(m_searchString, LocationCompleterModel::SearchStringRole);
        item->setData(true, LocationCompleterModel::VisitSearchItemRole);
        if (!m_domainCompletion.isEmpty()) {
            const QUrl url = QUrl(QSL("http://%1").arg(m_domainCompletion));
            item->setData(IconProvider::imageForDomain(url), LocationCompleterModel::ImageRole);
        }
        m_items.prepend(item);
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

            // Keyword bookmark replaces visit/search item
            if (bookmark->keyword() == m_searchString) {
                continue;
            }

            QStandardItem* item = new QStandardItem();
            item->setText(bookmark->url().toEncoded());
            item->setData(-1, LocationCompleterModel::IdRole);
            item->setData(bookmark->title(), LocationCompleterModel::TitleRole);
            item->setData(bookmark->url(), LocationCompleterModel::UrlRole);
            item->setData(bookmark->visitCount(), LocationCompleterModel::CountRole);
            item->setData(true, LocationCompleterModel::BookmarkRole);
            item->setData(QVariant::fromValue<void*>(static_cast<void*>(bookmark)), LocationCompleterModel::BookmarkItemRole);
            item->setData(m_searchString, LocationCompleterModel::SearchStringRole);

            urlList.append(bookmark->url());
            m_items.append(item);
        }
    }

    // Sort by count
    std::sort(m_items.begin(), m_items.end(), countBiggerThan);

    // Search in history
    if (showType == HistoryAndBookmarks || showType == History) {
        const int historyLimit = 20;
        QSqlQuery query = LocationCompleterModel::createHistoryQuery(m_searchString, historyLimit);
        query.exec();

        while (query.next()) {
            const QUrl url = query.value(1).toUrl();

            if (urlList.contains(url)) {
                continue;
            }

            QStandardItem* item = new QStandardItem();
            item->setText(url.toEncoded());
            item->setData(query.value(0), LocationCompleterModel::IdRole);
            item->setData(query.value(2), LocationCompleterModel::TitleRole);
            item->setData(url, LocationCompleterModel::UrlRole);
            item->setData(query.value(3), LocationCompleterModel::CountRole);
            item->setData(true, LocationCompleterModel::HistoryRole);
            item->setData(m_searchString, LocationCompleterModel::SearchStringRole);

            m_items.append(item);
        }
    }
}

void LocationCompleterRefreshJob::completeMostVisited()
{
    QSqlQuery query(SqlDatabase::instance()->database());
    query.exec(QSL("SELECT id, url, title FROM history ORDER BY count DESC LIMIT 15"));

    while (query.next()) {
        QStandardItem* item = new QStandardItem();
        const QUrl url = query.value(1).toUrl();

        item->setText(url.toEncoded());
        item->setData(query.value(0), LocationCompleterModel::IdRole);
        item->setData(query.value(2), LocationCompleterModel::TitleRole);
        item->setData(url, LocationCompleterModel::UrlRole);
        item->setData(true, LocationCompleterModel::HistoryRole);

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
