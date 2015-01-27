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
#ifndef QUPZILLASCHEMEHANDLER_H
#define QUPZILLASCHEMEHANDLER_H

#include <QNetworkReply>
#include <QBuffer>

#if QTWEBENGINE_DISABLED

#include "schemehandler.h"
#include "qzcommon.h"

class QUPZILLA_EXPORT QupZillaSchemeHandler : public SchemeHandler
{
public:
    explicit QupZillaSchemeHandler();

    QNetworkReply* createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData);
};

class QUPZILLA_EXPORT QupZillaSchemeReply : public QNetworkReply
{
    Q_OBJECT
public:
    explicit QupZillaSchemeReply(const QNetworkRequest &req, QObject* parent = 0);

    qint64 bytesAvailable() const;

protected:
    qint64 readData(char* data, qint64 maxSize);
    void abort() { }

private slots:
    void delayedFinish();
    void loadPage();

private:
    QString aboutPage();
    QString reportbugPage();
    QString startPage();
    QString speeddialPage();
    QString restorePage();
    QString configPage();

    QBuffer m_buffer;
    QString m_pageName;
};

#endif

#endif // QUPZILLASCHEMEHANDLER_H
