/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#ifndef VIEWSOURCESCHEMEHANDLER_H
#define VIEWSOURCESCHEMEHANDLER_H

#include <QNetworkReply>
#include <QBuffer>

#include "schemehandler.h"
#include "qz_namespace.h"

class QT_QUPZILLA_EXPORT ViewSourceSchemeHandler : public SchemeHandler
{
public:
    explicit ViewSourceSchemeHandler();

    QNetworkReply* createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData);
};

class QT_QUPZILLA_EXPORT ViewSourceSchemeReply : public QNetworkReply
{
    Q_OBJECT
public:
    explicit ViewSourceSchemeReply(const QNetworkRequest &req, QObject* parent = 0);

    qint64 bytesAvailable() const;

protected:
    qint64 readData(char* data, qint64 maxSize);
    void abort() { }

private slots:
    void loadPage();

private:
	QNetworkReply* m_reply;
    QBuffer m_buffer;
};

#endif // VIEWSOURCESCHEMEHANDLER_H
