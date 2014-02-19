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
#ifndef PACMANAGER_H
#define PACMANAGER_H

#include <QObject>
#include <QList>
#include <QUrl>

#include "qz_namespace.h"

class QNetworkProxy;

class FollowRedirectReply;
class ProxyAutoConfig;

class QUPZILLA_EXPORT PacManager : public QObject
{
    Q_OBJECT
public:
    explicit PacManager(QObject* parent = 0);

    void loadSettings();
    void downloadPacFile();

    QList<QNetworkProxy> queryProxy(const QUrl &url);

private slots:
    void replyFinished();

private:
    void reloadScript();
    QList<QNetworkProxy> parseProxies(const QString &string);

    ProxyAutoConfig* m_pacrunner;
    FollowRedirectReply* m_reply;

    bool m_loaded;
    QUrl m_url;
};

#endif // PACMANAGER_H
