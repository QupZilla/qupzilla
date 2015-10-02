/* ============================================================
* QupZilla - QtWebEngine based browser
* Copyright (C) 2015  David Rosca <nowrep@gmail.com>
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
#ifndef ICONLOADER_H
#define ICONLOADER_H

#include <QObject>

#include "qzcommon.h"

class QIcon;

class FollowRedirectReply;

class QUPZILLA_EXPORT IconLoader : public QObject
{
    Q_OBJECT

public:
    explicit IconLoader(const QUrl &url, QObject* parent = 0);

signals:
    void iconLoaded(const QIcon &icon);
    void error();

private slots:
    void finished();

private:
    FollowRedirectReply* m_reply;
};

#endif // ICONLOADER_H
