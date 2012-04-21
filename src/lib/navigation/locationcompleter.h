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
#ifndef LOCATIONCOMPLETER_H
#define LOCATIONCOMPLETER_H

#include <QCompleter>

#include "qz_namespace.h"

class QStandardItemModel;

class CompleterListView;

class QT_QUPZILLA_EXPORT LocationCompleter : public QCompleter
{
    Q_OBJECT
public:
    explicit LocationCompleter(QObject* parent = 0);

    virtual QStringList splitPath(const QString &path) const;

signals:

public slots:
    void refreshCompleter(const QString &string);
    void showMostVisited();

private:
    CompleterListView* m_listView;
    QStandardItemModel* m_model;
};

#endif // LOCATIONCOMPLETER_H
