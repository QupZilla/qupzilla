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
#include "loadrequest.h"

LoadRequest::LoadRequest()
    : m_operation(GetOperation)
{
}

LoadRequest::LoadRequest(const LoadRequest &other)
    : m_url(other.m_url)
    , m_operation(other.m_operation)
    , m_data(other.m_data)
{
}

LoadRequest::LoadRequest(const QUrl &url, LoadRequest::Operation op, const QByteArray &data)
    : m_url(url)
    , m_operation(op)
    , m_data(data)
{
}

LoadRequest &LoadRequest::operator=(const LoadRequest &other)
{
    m_url = other.m_url;
    m_operation = other.m_operation;
    m_data = other.m_data;
    return *this;
}

bool LoadRequest::isValid() const
{
    return m_url.isValid();
}

QUrl LoadRequest::url() const
{
    return m_url;
}

void LoadRequest::setUrl(const QUrl &url)
{
    m_url = url;
}

QString LoadRequest::urlString() const
{
    return QUrl::fromPercentEncoding(m_url.toEncoded());
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

QWebEngineHttpRequest LoadRequest::webRequest() const
{
    QWebEngineHttpRequest req(m_url, m_operation == GetOperation ? QWebEngineHttpRequest::Get : QWebEngineHttpRequest::Post);
    req.setPostData(m_data);
    return req;
}
