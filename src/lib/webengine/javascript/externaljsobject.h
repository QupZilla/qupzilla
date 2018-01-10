/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2018 David Rosca <nowrep@gmail.com>
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

#include "qzcommon.h"

class WebPage;
class AutoFillJsObject;

class QWebChannel;

class QUPZILLA_EXPORT ExternalJsObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* speedDial READ speedDial CONSTANT)
    Q_PROPERTY(QObject* autoFill READ autoFill CONSTANT)
    Q_PROPERTY(QObject* recovery READ recovery CONSTANT)

public:
    explicit ExternalJsObject(WebPage *page);

    WebPage *page() const;

    static void setupWebChannel(QWebChannel *webChannel, WebPage *page);

    static void registerExtraObject(const QString &id, QObject *object);
    static void unregisterExtraObject(const QString &id);

private:
    QObject *speedDial() const;
    QObject *autoFill() const;
    QObject *recovery() const;

    WebPage *m_page;
    AutoFillJsObject *m_autoFill;
};

#endif // EXTERNALJSOBJECT_H
