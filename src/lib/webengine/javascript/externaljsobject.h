/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#ifndef EXTERNALJSOBJECT_H
#define EXTERNALJSOBJECT_H

#include <QObject>

class WebPage;
class AutoFillJsObject;

class ExternalJsObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* speedDial READ speedDial CONSTANT)
    Q_PROPERTY(QObject* autoFill READ autoFill CONSTANT)

public:
    explicit ExternalJsObject(WebPage *page);

    WebPage *page() const;

public slots:
    void AddSearchProvider(const QString &engineUrl);
    int IsSearchProviderInstalled(const QString &engineURL);

private:
    QObject *speedDial() const;
    QObject *autoFill() const;

    WebPage *m_page;
    AutoFillJsObject *m_autoFill;
};

#endif // EXTERNALJSOBJECT_H
