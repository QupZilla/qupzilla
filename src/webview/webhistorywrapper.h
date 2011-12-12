/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#ifndef WEBHISTORYWRAPPER_H
#define WEBHISTORYWRAPPER_H

#include <QObject>
#include <QWebHistory>

class WebHistoryWrapper : public QObject
{
    Q_OBJECT
public:
    explicit WebHistoryWrapper(QObject* parent = 0);

    static QList<QWebHistoryItem> forwardItems(int maxItems, QWebHistory* history);
    static QList<QWebHistoryItem> backItems(int maxItems, QWebHistory* history);

    static bool canGoForward(QWebHistory* history);
    static bool canGoBack(QWebHistory* history);

    static void goBack(QWebHistory* history);
    static void goForward(QWebHistory* history);

    static int indexOfItem(const QList<QWebHistoryItem> &list, const QWebHistoryItem &item);

};

#endif // WEBHISTORYWRAPPER_H
