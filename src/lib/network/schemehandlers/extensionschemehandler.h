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
#pragma once

#include <QWebEngineUrlSchemeHandler>

#include "qzcommon.h"

class QUPZILLA_EXPORT ExtensionSchemeHandler : public QObject
{
public:
    explicit ExtensionSchemeHandler(QObject *parent = nullptr) : QObject(parent) { }

    virtual void requestStarted(QWebEngineUrlRequestJob *job) = 0;

protected:
    void setReply(QWebEngineUrlRequestJob *job, const QByteArray &contentType, const QByteArray &content);
};

class QUPZILLA_EXPORT ExtensionSchemeManager : public QWebEngineUrlSchemeHandler
{
public:
    explicit ExtensionSchemeManager(QObject *parent = nullptr);

    void requestStarted(QWebEngineUrlRequestJob *job) override;

    void registerHandler(const QString &name, ExtensionSchemeHandler *handler);
    void unregisterHandler(const QString &name);

private:
    QHash<QString, ExtensionSchemeHandler*> m_handlers;
};
