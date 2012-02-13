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
#ifndef QUPZILLASCHEMEHANDLER_H
#define QUPZILLASCHEMEHANDLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QTimer>
#include <QTextStream>
#include <QTextDocument>

class QupZillaSchemeHandler
{
public:
    explicit QupZillaSchemeHandler();

    QNetworkReply* createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData);
};

class QupZillaSchemeReply : public QNetworkReply
{
    Q_OBJECT
public:
    explicit QupZillaSchemeReply(const QNetworkRequest &req, QObject* parent = 0);

    virtual qint64 bytesAvailable() const;

protected:
    virtual qint64 readData(char* data, qint64 maxSize);
    virtual void abort() { }

private slots:
    void delayedFinish();
    void loadPage();

private:
    QString aboutPage();
    QString reportbugPage();
    QString startPage();
    QString speeddialPage();
    QString configPage();

    QBuffer m_buffer;
    QString m_pageName;
};

#endif // QUPZILLASCHEMEHANDLER_H
