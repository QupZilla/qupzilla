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
#ifndef LOADREQUEST_H
#define LOADREQUEST_H

#include <QNetworkRequest>

#include "qzcommon.h"

class QWebView;

class QUPZILLA_EXPORT LoadRequest
{
public:
    enum Operation {
        GetOperation = 0,
        PostOperation = 1
    };

    LoadRequest();
    LoadRequest(const LoadRequest &other);
    LoadRequest(const QUrl &url);
    LoadRequest(const QNetworkRequest &req, Operation op = GetOperation, const QByteArray &data = QByteArray());

    LoadRequest &operator=(const LoadRequest &other);

    bool isEmpty() const;

    QUrl url() const;
    void setUrl(const QUrl &url);

    QString urlString() const;

    QNetworkRequest networkRequest() const;
    void setNetworkRequest(const QNetworkRequest &req);

    Operation operation() const;
    void setOperation(Operation op);

    QByteArray data() const;
    void setData(const QByteArray &data);

private:
    QNetworkRequest m_request;
    Operation m_operation;
    QByteArray m_data;
};

#endif // LOADREQUEST_H
