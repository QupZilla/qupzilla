/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#ifndef LOCATIONCOMPLETERMODEL_H
#define LOCATIONCOMPLETERMODEL_H

#include <QStandardItemModel>

#include "qzcommon.h"

class QSqlQuery;
class QUrl;

class LocationCompleterModel : public QStandardItemModel
{
public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        UrlRole,
        CountRole,
        HistoryRole,
        BookmarkRole,
        BookmarkItemRole,
        SearchStringRole,
        TabPositionWindowRole,
        TabPositionTabRole,
        ImageRole,
        VisitSearchItemRole,
        SearchSuggestionRole
    };

    explicit LocationCompleterModel(QObject* parent = 0);

    void setCompletions(const QList<QStandardItem*> &items);
    void addCompletions(const QList<QStandardItem*> &items);

    QList<QStandardItem*> suggestionItems() const;

    static QSqlQuery createHistoryQuery(const QString &searchString, int limit, bool exactMatch = false);
    static QSqlQuery createDomainQuery(const QString &text);

private:
    enum Type {
        HistoryAndBookmarks = 0,
        History = 1,
        Bookmarks = 2,
        Nothing = 4
    };

    void setTabPosition(QStandardItem* item) const;
    void refreshTabPositions() const;
};

#endif // LOCATIONCOMPLETERMODEL_H
