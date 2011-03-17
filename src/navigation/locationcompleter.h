/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#ifndef LOCATIONCOMPLETER_H
#define LOCATIONCOMPLETER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QCompleter>
#include <QDebug>
#include <QStringList>
#include <QSqlQuery>
#include <QTreeView>
#include <QStandardItemModel>
#include <QTimer>
#include <QHeaderView>
#include <QUrl>

class LocationCompleter : public QCompleter
{
    Q_OBJECT
public:
    explicit LocationCompleter(QObject* parent = 0);

    //virtual QString pathFromIndex(const QModelIndex &index) const;
    virtual QStringList splitPath(const QString &path) const;

signals:

public slots:
    void loadInitialHistory();
    void refreshCompleter(QString string);

};

#endif // LOCATIONCOMPLETER_H
