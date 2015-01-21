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
#include "loadrequest.h"

#include <QWebView>

LoadRequest::LoadRequest()
    : m_operation(GetOperation)
{
}

LoadRequest::LoadRequest(const LoadRequest &other)
    : m_request(other.m_request)
    , m_operation(other.m_operation)
    , m_data(other.m_data)
{
}

LoadRequest::LoadRequest(const QUrl &url)
    : m_operation(GetOperation)
{
    setUrl(url);
}

LoadRequest::LoadRequest(const QNetworkRequest &req, LoadRequest::Operation op, const QByteArray &data)
    : m_request(req)
    , m_operation(op)
    , m_data(data)
{
}

LoadRequest &LoadRequest::operator=(const LoadRequest &other)
{
    m_request = other.m_request;
    m_operation = other.m_operation;
    m_data = other.m_data;
    return *this;
}

bool LoadRequest::isEmpty() const
{
    return m_request.url().isEmpty();
}

QUrl LoadRequest::url() const
{
    return m_request.url();
}

void LoadRequest::setUrl(const QUrl &url)
{
    m_request.setUrl(url);
}

QString LoadRequest::urlString() const
{
    return QUrl::fromPercentEncoding(m_request.url().toEncoded());
}

QNetworkRequest LoadRequest::networkRequest() const
{
    return m_request;
}

void LoadRequest::setNetworkRequest(const QNetworkRequest &req)
{
    m_request = req;
}

LoadRequest::Operation LoadRequest::operation() const
{
    return m_operation;
}

void LoadRequest::setOperation(LoadRequest::Operation op)
{
    m_operation = op;
}

QByteArray LoadRequest::data() const
{
    return m_data;
}

void LoadRequest::setData(const QByteArray &data)
{
    m_data = data;
}
