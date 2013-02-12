/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
* Copyright (C) 2013  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#include "ftpschemehandler.h"
#include "qztools.h"
#include "iconprovider.h"
#include "mainapplication.h"

#include <QWebSecurityOrigin>
#include <QFileIconProvider>
#include <QTextStream>
#include <QDateTime>
#include <QDir>


QHash<QString, QAuthenticator*> FtpSchemeHandler::m_ftpAuthenticatorsCache = QHash<QString, QAuthenticator*>();

FtpSchemeHandler::FtpSchemeHandler()
{
}

QNetworkReply* FtpSchemeHandler::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
{
    Q_UNUSED(outgoingData)

    if (op != QNetworkAccessManager::GetOperation) {
        return 0;
    }

    QNetworkReply* reply = new FtpSchemeReply(request);
    return reply;
}

QAuthenticator* FtpSchemeHandler::ftpAuthenticator(const QUrl &url)
{
    QString key = url.host();
    if (key.isEmpty()) {
        key = url.toString();
    }
    if (!m_ftpAuthenticatorsCache.contains(key) || !m_ftpAuthenticatorsCache.value(key, 0)) {
        QAuthenticator* auth = new QAuthenticator();
        auth->setUser(url.userName());
        auth->setPassword(url.password());
        m_ftpAuthenticatorsCache.insert(key, auth);
    }

    return m_ftpAuthenticatorsCache.value(key);
}

FtpSchemeReply::FtpSchemeReply(const QNetworkRequest &request, QObject* parent)
    : QNetworkReply(parent)
    , m_ftpLoginId(-1)
    , m_ftpCdId(-1)
    , m_port(21)
    , m_anonymousLoginChecked(false)
    , m_request(request)
    , m_isGoingToDownload(false)
{
    m_ftp = new QFtp(this);
    connect(m_ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(processListInfo(QUrlInfo)));
    connect(m_ftp, SIGNAL(readyRead()), this, SLOT(processData()));
    connect(m_ftp, SIGNAL(commandFinished(int, bool)), this, SLOT(processCommand(int, bool)));
    connect(m_ftp, SIGNAL(dataTransferProgress(qint64, qint64)), this, SIGNAL(downloadProgress(qint64, qint64)));

    m_buffer.open(QIODevice::ReadWrite);

    if (request.url().port() != -1) {
        m_port = request.url().port();
    }

    setUrl(request.url());
    m_ftp->connectToHost(request.url().host(), m_port);
}

void FtpSchemeReply::processCommand(int id, bool err)
{
    if (err) {
        ftpReplyErrorHandler(id);
        return;
    }

    switch (m_ftp->currentCommand()) {
    case QFtp::ConnectToHost:
        if (!m_anonymousLoginChecked) {
            m_anonymousLoginChecked = FTP_AUTHENTICATOR(url())->user().isEmpty()
                                      && FTP_AUTHENTICATOR(url())->password().isEmpty();
        }
        m_ftpLoginId = m_ftp->login(FTP_AUTHENTICATOR(url())->user(),
                                    FTP_AUTHENTICATOR(url())->password());
        break;

    case QFtp::Login:
        if (url().path().isEmpty() || url().path() == QLatin1String("/")) {
            m_ftp->list();
        }
        else {
            m_ftpCdId = m_ftp->cd(url().path());
        }
        break;

    case QFtp::Cd:
        m_ftp->list();
        break;

    case QFtp::List:
        if (m_isGoingToDownload) {
            foreach(const QUrlInfo & item, m_items) {
                if (item.isFile() && item.name() == m_probablyFileForDownload) {
                    emit downloadRequest(m_request);
                    abort();
                    break;
                }
            }
            m_probablyFileForDownload.clear();
            m_isGoingToDownload = false;
            abort();
        }
        else {
            loadPage();
        }
        break;

    case QFtp::Get:
        setContent();
        break;

    default:
        ;
    }
}

void FtpSchemeReply::processListInfo(const QUrlInfo &urlInfo)
{
    m_items.append(urlInfo);
}

void FtpSchemeReply::processData()
{
    open(ReadOnly | Unbuffered);
    QTextStream stream(&m_buffer);

    stream << m_ftp->readAll();

    stream.flush();
    m_buffer.reset();

    setHeader(QNetworkRequest::ContentLengthHeader, m_buffer.bytesAvailable());
    emit metaDataChanged();
}

void FtpSchemeReply::setContent()
{
    open(ReadOnly | Unbuffered);
    setHeader(QNetworkRequest::ContentLengthHeader, QVariant(m_buffer.size()));
    emit readyRead();
    emit finished();
    m_ftp->close();
}

void FtpSchemeReply::abort()
{
    setError(QNetworkReply::OperationCanceledError, "QupZilla:No Error");
    emit error(QNetworkReply::OperationCanceledError);
    emit finished();
    m_ftp->close();
}

qint64 FtpSchemeReply::bytesAvailable() const
{
    return m_buffer.bytesAvailable() + QNetworkReply::bytesAvailable();
}

bool FtpSchemeReply::isSequential() const
{
    return true;
}

qint64 FtpSchemeReply::readData(char* data, qint64 maxSize)
{
    return m_buffer.read(data, maxSize);
}

void FtpSchemeReply::loadPage()
{
    QWebSecurityOrigin::addLocalScheme("ftp");
    open(ReadOnly | Unbuffered);
    QTextStream stream(&m_buffer);
    stream.setCodec("UTF-8");

    stream << loadDirectory();

    stream.flush();
    m_buffer.reset();

    setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("text/html"));
    setHeader(QNetworkRequest::ContentLengthHeader, m_buffer.bytesAvailable());
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
    setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QByteArray("Ok"));
    emit metaDataChanged();

    emit readyRead();
    emit finished();
    m_ftp->close();
    QWebSecurityOrigin::removeLocalScheme("ftp");
}

QString FtpSchemeReply::loadDirectory()
{
    QUrl u = url();
    if (!u.path().endsWith(QLatin1Char('/'))) {
        u.setPath(u.path() + QLatin1String("/"));
    }

    QString base_path = u.path();
    const QDir &dir = QDir(base_path);

    QUrl parent = u.resolved(QUrl(".."));

    static QString sPage;

    if (sPage.isEmpty()) {
        sPage = QzTools::readAllFileContents(":/html/dirlist.html");
        sPage.replace(QLatin1String("%BOX-BORDER%"), QLatin1String("qrc:html/box-border.png"));
        sPage.replace(QLatin1String("%UP-IMG%"), QzTools::pixmapToByteArray(qIconProvider->standardIcon(QStyle::SP_FileDialogToParent).pixmap(22)));
        sPage.replace(QLatin1String("%UP-DIR-TEXT%"), tr("Up to higher level directory"));
        sPage.replace(QLatin1String("%SHOW-HIDDEN-TEXT%"), tr("Show hidden files"));
        sPage.replace(QLatin1String("%NAME%"), tr("Name"));
        sPage.replace(QLatin1String("%SIZE%"), tr("Size"));
        sPage.replace(QLatin1String("%MODIFIED%"), tr("Last modified"));
        sPage = QzTools::applyDirectionToPage(sPage);
    }

    QString page = sPage;
    page.replace(QLatin1String("%TITLE%"), tr("Index for %1").arg(url().toString()));

    QString upDirDisplay = QLatin1String("none");
    QString tBody;

    if (!dir.isRoot()) {
        upDirDisplay = QLatin1String("inline");
        page.replace(QLatin1String("%UP-DIR-LINK%"), parent.toEncoded());
    }

    int lastIndex = m_items.size();
    for (int i = 0; i < lastIndex; ++i) {
        if (m_items.at(i).isFile()) {
            m_items.move(i, m_items.size() - 1);
            --lastIndex;
            --i;
        }
    }

    foreach(const QUrlInfo & item, m_items) {

        if (item.name() == QLatin1String(".") || item.name() == QLatin1String("..")) {
            continue;
        }

        QString line = QLatin1String("<tr");
        QUrl itemUrl = u.resolved(QUrl(QUrl::toPercentEncoding(item.name())));
        QString itemPath = itemUrl.path();

        if (itemPath.endsWith(QLatin1Char('/'))) {
            itemPath.remove(itemPath.size() - 1, 1);
        }

        line += QLatin1String("><td class=\"td-name\" style=\"background-image:url(data:image/png;base64,");
        line += QzTools::pixmapToByteArray(item.isFile()
                                           ? QzTools::iconFromFileName(itemPath).pixmap(16)
                                           : QFileIconProvider().icon(QFileIconProvider::Folder).pixmap(16));
        line += QLatin1String(");\">");
        line += QLatin1String("<a href=\"");
        line += itemUrl.toEncoded();
        line += QLatin1String("\">");
        line += item.name();
        line += QLatin1String("</a></td><td class=\"td-size\">");
        line += item.isDir() ? QString() : QzTools::fileSizeToString(item.size());
        line += QLatin1String("</td><td>");
        line += item.lastModified().toString("dd.MM.yyyy");
        line += QLatin1String("</td><td>");
        line += item.lastModified().toString("hh:mm:ss");
        line += QLatin1String("</td></tr>\n");

        tBody.append(line);
    }

    if (tBody.isEmpty()) {
        tBody = QString("<tr><td colspan='4'>%1</td></tr>").arg(tr("Folder is empty."));
    }

    page.replace(QLatin1String("%T-BODY%"), tBody);
    page.replace(QLatin1String("%UP-DIR-DISPLAY%"), upDirDisplay);
    page.replace(QLatin1String("%SHOW-HIDDEN-DISPLAY%"), QLatin1String("none"));

    return page;
}

void FtpSchemeReply::ftpReplyErrorHandler(int id)
{
    if (m_ftpLoginId == id) {
        if (!m_anonymousLoginChecked) {
            m_anonymousLoginChecked = true;
            FTP_AUTHENTICATOR(url())->setUser(QString());
            FTP_AUTHENTICATOR(url())->setPassword(QString());
            m_ftpLoginId = m_ftp->login();
            return;
        }

        emit ftpAuthenticationRequierd(url(), FTP_AUTHENTICATOR(url()));
        m_ftpLoginId = m_ftp->login(FTP_AUTHENTICATOR(url())->user(), FTP_AUTHENTICATOR(url())->password());
        return;
    }
    else if (m_ftpCdId == id) {
        if (m_isGoingToDownload) {
            m_isGoingToDownload = false;
            abort();
            return;
        }
        QStringList sections = url().path().split(QLatin1Char('/'), QString::SkipEmptyParts);
        if (!sections.isEmpty()) {
            m_probablyFileForDownload = sections.takeLast();
        }
        if (!m_probablyFileForDownload.isEmpty()) {
            m_isGoingToDownload = true;
            QString parentOfPath = QString("/%1/").arg(sections.join(QLatin1String("/")));
            m_ftpCdId = m_ftp->cd(parentOfPath);
        }
        else {
            abort();
        }
        return;
    }
    else {
        setError(ContentNotFoundError, tr("Unknown command"));
        emit error(ContentNotFoundError);
        emit finished();
    }
}

FtpDownloader::FtpDownloader(QObject* parent)
    : QFtp(parent)
    , m_ftpLoginId(-1)
    , m_anonymousLoginChecked(false)
    , m_isFinished(false)
    , m_url(QUrl())
    , m_dev(0)
    , m_lastError(QFtp::NoError)
{
    connect(this, SIGNAL(commandFinished(int, bool)), this, SLOT(processCommand(int, bool)));
    connect(this, SIGNAL(done(bool)), this, SLOT(onDone(bool)));
}

void FtpDownloader::download(const QUrl &url, QIODevice* dev)
{
    m_url = url;
    m_dev = dev;
    QString server = m_url.host();
    if (server.isEmpty()) {
        server = m_url.toString();
    }
    int port = 21;
    if (m_url.port() != -1) {
        port = m_url.port();
    }

    connectToHost(server, port);
}

void FtpDownloader::setError(QFtp::Error err, const QString &errStr)
{
    m_lastError = err;
    m_lastErrorString = errStr;
}

void FtpDownloader::abort()
{
    setError(QFtp::UnknownError, tr("Cancelled!"));
    QFtp::abort();
}

QFtp::Error FtpDownloader::error()
{
    if (m_lastError != QFtp::NoError && QFtp::error() == QFtp::NoError) {
        return m_lastError;
    }
    else {
        return QFtp::error();
    }
}

QString FtpDownloader::errorString() const
{
    if (!m_lastErrorString.isEmpty()
            && m_lastError != QFtp::NoError
            && QFtp::error() == QFtp::NoError) {
        return m_lastErrorString;
    }
    else {
        return QFtp::errorString();
    }
}

void FtpDownloader::processCommand(int id, bool err)
{
    if (!m_url.isValid() || m_url.isEmpty() || !m_dev) {
        abort();
        return;
    }

    if (err) {
        if (m_ftpLoginId == id) {
            if (!m_anonymousLoginChecked) {
                m_anonymousLoginChecked = true;
                FTP_AUTHENTICATOR(m_url)->setUser(QString());
                FTP_AUTHENTICATOR(m_url)->setPassword(QString());
                m_ftpLoginId = login();
                return;
            }
            emit ftpAuthenticationRequierd(m_url, FTP_AUTHENTICATOR(m_url));
            m_ftpLoginId = login(FTP_AUTHENTICATOR(m_url)->user(), FTP_AUTHENTICATOR(m_url)->password());
            return;
        }
        abort();
        return;
    }

    switch (currentCommand()) {
    case QFtp::ConnectToHost:
        if (!m_anonymousLoginChecked) {
            m_anonymousLoginChecked = FTP_AUTHENTICATOR(m_url)->user().isEmpty()
                                      && FTP_AUTHENTICATOR(m_url)->password().isEmpty();
        }
        m_ftpLoginId = login(FTP_AUTHENTICATOR(m_url)->user(), FTP_AUTHENTICATOR(m_url)->password());
        break;

    case QFtp::Login:
        get(m_url.path(), m_dev);
        break;
    default:
        ;
    }
}

void FtpDownloader::onDone(bool err)
{
    disconnect(this, SIGNAL(done(bool)), this, SLOT(onDone(bool)));
    close();
    m_ftpLoginId = -1;
    if (err || m_lastError != QFtp::NoError) {
        emit errorOccured(error());
    }
    else {
        m_isFinished = true;
        emit finished();
    }
}
