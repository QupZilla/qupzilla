/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#ifndef PAGEFORMCOMPLETER_H
#define PAGEFORMCOMPLETER_H

#include <QPair>
#include <QString>
#include <QByteArray>

#include "qz_namespace.h"

class QWebPage;
class QWebElement;
class QWebElementCollection;

struct PageFormData {
    bool found;
    QString username;
    QString password;
    QByteArray postData;
};

class PageFormCompleter
{
public:
    explicit PageFormCompleter(QWebPage* page);

    PageFormData extractFormData(const QByteArray &postData) const;
    void completePage(const QByteArray &data) const;

private:
    typedef QPair<QString, QString> QueryItem;
    typedef QList<QPair<QString, QString> > QueryItems;

    bool queryItemsContains(const QueryItems &queryItems, const QString &attributeName,
                            const QString &attributeValue) const;
    QByteArray convertWebKitFormBoundaryIfNecessary(const QByteArray &data) const;
    QueryItem findUsername(const QWebElement &form) const;
    QueryItems createQueryItems(const QByteArray &data) const;
    QWebElementCollection getAllElementsFromPage(QWebPage* page, const QString &selector) const;

    QWebPage* m_page;
};

#endif // PAGEFORMCOMPLETER_H
