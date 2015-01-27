/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef NETWORKMANAGERPROXY_H
#define NETWORKMANAGERPROXY_H

#if QTWEBENGINE_DISABLED

#include <QNetworkAccessManager>

#include "qzcommon.h"

class WebPage;
class NetworkManager;

class QUPZILLA_EXPORT NetworkManagerProxy : public QNetworkAccessManager
{
public:
    explicit NetworkManagerProxy(QObject* parent = 0);

    void setPage(WebPage* page) { m_page = page; }
    void setPrimaryNetworkAccessManager(NetworkManager* manager);

    QNetworkReply* createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData);

private:
    WebPage* m_page;
    NetworkManager* m_manager;
};

#endif

#endif // NETWORKMANAGERPROXY_H
