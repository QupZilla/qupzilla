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
#ifndef LOCATIONCOMPLETERMODEL_H
#define LOCATIONCOMPLETERMODEL_H

#include <QStandardItemModel>

class LocationCompleterModel : public QStandardItemModel
{
public:
    enum Role {
        TitleRole = Qt::UserRole + 1,
        BookmarkRole = Qt::UserRole + 2,
        IdRole = Qt::UserRole + 3
    };
    explicit LocationCompleterModel(QObject* parent = 0);

    void refreshCompletions(const QString &string);
    void showMostVisited();

signals:

public slots:

private:
    enum Type {
        HistoryAndBookmarks = 0,
        History = 1,
        Bookmarks = 2,
        Nothing = 3
    };

    QString m_lastCompletion;

};

#endif // LOCATIONCOMPLETERMODEL_H
