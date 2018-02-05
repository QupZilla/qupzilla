/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "extensionschemehandler.h"

#include <QBuffer>
#include <QUrlQuery>
#include <QWebEngineUrlRequestJob>

// ExtensionSchemeHandler
void ExtensionSchemeHandler::setReply(QWebEngineUrlRequestJob *job, const QByteArray &contentType, const QByteArray &content)
{
    QBuffer *buffer = new QBuffer();
    buffer->open(QIODevice::ReadWrite);
    buffer->write(content);
    buffer->seek(0);
    job->reply(contentType, buffer);
}

// ExtensionSchemeManager
ExtensionSchemeManager::ExtensionSchemeManager(QObject *parent)
    : QWebEngineUrlSchemeHandler(parent)
{
}

void ExtensionSchemeManager::requestStarted(QWebEngineUrlRequestJob *job)
{
    ExtensionSchemeHandler *handler = m_handlers.value(job->requestUrl().host());
    if (handler) {
        handler->requestStarted(job);
    } else {
        job->fail(QWebEngineUrlRequestJob::UrlInvalid);
    }
}

void ExtensionSchemeManager::registerHandler(const QString &name, ExtensionSchemeHandler *handler)
{
    m_handlers[name] = handler;
}

void ExtensionSchemeManager::unregisterHandler(const QString &name)
{
    m_handlers.remove(name);
}
