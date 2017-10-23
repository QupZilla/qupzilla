/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2017 David Rosca <nowrep@gmail.com>
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
#ifndef LOCATIONCOMPLETERREFRESHJOB_H
#define LOCATIONCOMPLETERREFRESHJOB_H

#include <QFutureWatcher>

#include "qzcommon.h"

class QStandardItem;

class QUPZILLA_EXPORT LocationCompleterRefreshJob : public QObject
{
    Q_OBJECT

public:
    explicit LocationCompleterRefreshJob(const QString &searchString);

    // Timestamp when the job was created
    qint64 timestamp() const;
    QString searchString() const;
    bool isCanceled() const;

    QList<QStandardItem*> completions() const;
    QString domainCompletion() const;

signals:
    void finished();

private slots:
    void slotFinished();
    void jobCancelled();

private:
    enum Type {
        HistoryAndBookmarks = 0,
        History = 1,
        Bookmarks = 2,
        Nothing = 4
    };

    void runJob();
    void completeFromHistory();
    void completeMostVisited();

    QString createDomainCompletion(const QString &completion) const;

    qint64 m_timestamp;
    QString m_searchString;
    QString m_domainCompletion;
    QList<QStandardItem*> m_items;
    QFutureWatcher<void>* m_watcher;
    bool m_jobCancelled;
};

#endif // LOCATIONCOMPLETERREFRESHJOB_H
