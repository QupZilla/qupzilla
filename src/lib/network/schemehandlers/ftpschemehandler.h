/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
* Copyright (C) 2013-2014  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
* Some codes and ideas are from webftpclient example by Qt Lab
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
#ifndef FTPSCHEMEHANDLER_H
#define FTPSCHEMEHANDLER_H

#include <QAuthenticator>
#include <QNetworkReply>
#include <QBuffer>

#if QT_VERSION >= 0x050000
#include "qftp.h"
#include "qurlinfo.h"
#else
#include <QFtp>
#include <QUrlInfo>
#endif

#include "schemehandler.h"
#include "qzcommon.h"

#define FTP_AUTHENTICATOR FtpSchemeHandler::ftpAuthenticator

class QUPZILLA_EXPORT FtpSchemeHandler : public SchemeHandler
{
public:
    explicit FtpSchemeHandler();

    QNetworkReply* createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData);

    static QAuthenticator* ftpAuthenticator(const QUrl &url);

private:
    static QHash<QString, QAuthenticator*> m_ftpAuthenticatorsCache;
};

class QUPZILLA_EXPORT FtpSchemeReply : public QNetworkReply
{
    Q_OBJECT

public:
    FtpSchemeReply(const QUrl &url, QObject* parent = 0);
    void abort();
    qint64 bytesAvailable() const;
    bool isSequential() const;

protected:
    qint64 readData(char* data, qint64 maxSize);

private slots:
    void processCommand(int id, bool err);
    void processListInfo(QUrlInfo urlInfo);
    void processData();
    QString loadDirectory();
    void loadPage();

private:
    void setContent();
    void ftpReplyErrorHandler(int id);
    QString clickableSections(const QString &path);

    QFtp* m_ftp;
    QList<QUrlInfo> m_items;
    int m_ftpLoginId;
    int m_ftpCdId;
    int m_port;
    bool m_anonymousLoginChecked;
    QString m_probablyFileForDownload;
    bool m_isGoingToDownload;

    QByteArray m_buffer;
    qint64 m_offset;
    bool m_isContentTypeDetected;
    QString m_contentSampleData;

signals:
    void ftpAuthenticationRequierd(const QUrl &, QAuthenticator*);
    void downloadRequest(const QNetworkRequest &);
};

class QUPZILLA_EXPORT FtpDownloader : public QFtp
{
    Q_OBJECT

public:
    FtpDownloader(QObject* parent = 0);

    void download(const QUrl &url, QIODevice* dev);
    bool isFinished();
    QUrl url() const;
    QIODevice* device() const;
    void setError(QFtp::Error err, const QString &errStr);
    void abort();
    QFtp::Error error();
    QString errorString() const;

private slots:
    void processCommand(int id, bool err);
    void onDone(bool err);

private:
    int m_ftpLoginId;
    bool m_anonymousLoginChecked;
    bool m_isFinished;
    QUrl m_url;
    QIODevice* m_dev;
    QFtp::Error m_lastError;
    QString m_lastErrorString;

signals:
    void ftpAuthenticationRequierd(const QUrl &, QAuthenticator*);
    void finished();
    void errorOccured(QFtp::Error);
};
#endif // FTPSCHEMEHANDLER_H
