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
#ifndef LOCATIONCOMPLETERMODEL_H
#define LOCATIONCOMPLETERMODEL_H

#include <QStandardItemModel>

#include "qzcommon.h"

class QSqlQuery;
class QUrl;

struct TabPosition {
    int windowIndex;
    int tabIndex;
    TabPosition()
        : windowIndex(-1)
        , tabIndex(-1)
    {}
};
Q_DECLARE_METATYPE(TabPosition)

class LocationCompleterModel : public QStandardItemModel
{
public:
    enum Role {
        TitleRole = Qt::UserRole + 1,
        BookmarkRole = Qt::UserRole + 2,
        IdRole = Qt::UserRole + 3,
        SearchStringRole = Qt::UserRole + 4,
        CountRole = Qt::UserRole + 5,
        TabPositionRole = Qt::UserRole + 6
    };
    explicit LocationCompleterModel(QObject* parent = 0);

    void refreshCompletions(const QString &string);
    void showMostVisited();

    QString completeDomain(const QString &text);

private:
    enum Type {
        HistoryAndBookmarks = 0,
        History = 1,
        Bookmarks = 2,
        Nothing = 4
    };

    QSqlQuery createQuery(const QString &searchString,
                          int limit, bool exactMatch = false);

    TabPosition tabPositionForUrl(const QUrl &url) const;
    TabPosition tabPositionForEncodedUrl(const QString &encodedUrl) const;
    void refreshTabPositions();

    QString m_lastCompletion;
};

#endif // LOCATIONCOMPLETERMODEL_H
