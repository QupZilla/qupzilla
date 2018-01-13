/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2015-2018 David Rosca <nowrep@gmail.com>
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

#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <QString>

#include "qzcommon.h"

class QWebEngineView;

class QUPZILLA_EXPORT Scripts
{
public:
    static QString setupWebChannel(quint32 worldId);
    static QString setupFormObserver();
    static QString setupWindowObject();

    static QString setCss(const QString &css);
    static QString sendPostData(const QUrl &url, const QByteArray &data);
    static QString completeFormData(const QByteArray &data);
    static QString getOpenSearchLinks();
    static QString getAllImages();
    static QString getAllMetaAttributes();
    static QString getFormData(const QPointF &pos);
};

#endif // SCRIPTS_H
