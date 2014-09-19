/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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

#include "qzcommon.h"

class QWebPage;
class QWebFrame;
class QWebElement;
class QWebElementCollection;

struct PageFormData {
    QString username;
    QString password;
    QByteArray postData;

    bool isValid() const {
        return !password.isEmpty();
    }
};

class QUPZILLA_EXPORT PageFormCompleter
{
public:
    explicit PageFormCompleter();

    PageFormData extractFormData(QWebPage* page, const QByteArray &postData);
    PageFormData extractFormData(QWebFrame* frame, const QByteArray &postData);

    bool completeFormData(QWebPage* page, const QByteArray &data);
    bool completeFormData(QWebFrame* frame, const QByteArray &data);

private:
    typedef QPair<QString, QString> QueryItem;
    typedef QList<QPair<QString, QString> > QueryItems;

    PageFormData extractFormData(const QByteArray &postData) const;
    bool completeFormData(const QByteArray &data) const;

    bool queryItemsContains(const QueryItems &queryItems, const QString &attributeName,
                            const QString &attributeValue) const;
    QByteArray convertWebKitFormBoundaryIfNecessary(const QByteArray &data) const;
    QueryItem findUsername(const QWebElement &form) const;
    QueryItems createQueryItems(QByteArray data) const;
    QWebElementCollection getAllElementsFromPage(const QString &selector) const;

    QWebPage* m_page;
    QWebFrame* m_frame;
};

#endif // PAGEFORMCOMPLETER_H
