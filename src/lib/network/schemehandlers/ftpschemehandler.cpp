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
#include "ftpschemehandler.h"
#include "qztools.h"
#include "iconprovider.h"
#include "mainapplication.h"

#include <QWebSecurityOrigin>
#include <QFileIconProvider>
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

    QNetworkReply* reply = new FtpSchemeReply(request.url());
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

FtpSchemeReply::FtpSchemeReply(const QUrl &url, QObject* parent)
    : QNetworkReply(parent)
    , m_ftpLoginId(-1)
    , m_ftpCdId(-1)
    , m_port(21)
    , m_anonymousLoginChecked(false)
    , m_isGoingToDownload(false)
    , m_isContentTypeDetected(false)
{
    m_ftp = new QFtp(this);
    connect(m_ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(processListInfo(QUrlInfo)));
    connect(m_ftp, SIGNAL(readyRead()), this, SLOT(processData()));
    connect(m_ftp, SIGNAL(commandFinished(int,bool)), this, SLOT(processCommand(int,bool)));
    connect(m_ftp, SIGNAL(dataTransferProgress(qint64,qint64)), this, SIGNAL(downloadProgress(qint64,qint64)));

    if (url.port() != -1) {
        m_port = url.port();
    }

    m_offset = 0;
    setUrl(url);
    m_ftp->connectToHost(url.host(), m_port);

    open(ReadOnly);
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
            m_ftpCdId = m_ftp->cd(QString::fromLatin1(QByteArray::fromPercentEncoding(url().path().toUtf8())));
        }
        break;

    case QFtp::Cd:
        m_ftp->list();
        break;

    case QFtp::List:
        if (m_isGoingToDownload) {
            foreach (const QUrlInfo &item, m_items) {
                // don't check if it's a file or not,
                // seems it's a QFtp's bug: for link to a file isDir() returns true
                if (item.name() == m_probablyFileForDownload) {
                    QByteArray decodedUrl = QByteArray::fromPercentEncoding(url().toString().toUtf8());
                    if (QzTools::isUtf8(decodedUrl.constData())) {
                        setUrl(QUrl(QString::fromUtf8(decodedUrl)));
                    }
                    else {
                        setUrl(QUrl(QString::fromLatin1(decodedUrl)));
                    }

                    m_offset = 0;
                    m_ftp->get(url().path());
                    break;
                }
            }
            m_probablyFileForDownload.clear();
            m_isGoingToDownload = false;
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

void FtpSchemeReply::processListInfo(QUrlInfo urlInfo)
{
    QByteArray nameLatin1 = urlInfo.name().toLatin1();
    if (QzTools::isUtf8(nameLatin1.constData())) {
        urlInfo.setName(QString::fromUtf8(nameLatin1));
    }

    m_items.append(urlInfo);
}

void FtpSchemeReply::processData()
{
    QByteArray data = m_ftp->readAll();

    m_buffer += data;

    if (!m_isContentTypeDetected && !data.isEmpty()) {
        data = m_buffer.size() < 1000 ? m_buffer : data;
        data.truncate(1000);
        data = data.simplified();
        m_contentSampleData += QString::fromUtf8(data).simplified();

        if (m_contentSampleData.size() > 500) {
            bool isContentText = true;

            for (int i = 0; i < m_contentSampleData.size(); ++i) {
                if (!m_contentSampleData.at(i).isPrint()) {
                    isContentText = false;
                    break;
                }
            }

            //TODO: also request download for large text file

            m_contentSampleData.clear();
            m_isContentTypeDetected = true;

            if (!isContentText) {
                // request has unsupported content
                m_buffer.clear();
                emit downloadRequest(QNetworkRequest(url()));
                abort();
                return;
            }
        }
    }

    setHeader(QNetworkRequest::ContentLengthHeader, QVariant(m_buffer.size()));
    emit metaDataChanged();
}

void FtpSchemeReply::setContent()
{
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
    return m_buffer.size() - m_offset + QNetworkReply::bytesAvailable();
}

bool FtpSchemeReply::isSequential() const
{
    return true;
}

qint64 FtpSchemeReply::readData(char* data, qint64 maxSize)
{
    if (m_offset < m_buffer.size()) {
        qint64 size = qMin(maxSize, m_buffer.size() - m_offset);
        memcpy(data, m_buffer.constData() + m_offset, size);
        m_offset += size;

        return size;
    }
    else {
        return -1;
    }
}

void FtpSchemeReply::loadPage()
{
    QWebSecurityOrigin::addLocalScheme("ftp");
    open(ReadOnly | Unbuffered);

    m_offset = 0;
    m_buffer = loadDirectory().toUtf8();

    setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("text/html"));
    setHeader(QNetworkRequest::ContentLengthHeader, m_buffer.size());
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
    const QDir dir = QDir(base_path);

    QUrl parent = u.resolved(QUrl(".."));

    static QString sPage;

    if (sPage.isEmpty()) {
        sPage = QzTools::readAllFileContents(":/html/dirlist.html");
        sPage.replace(QLatin1String("%BOX-BORDER%"), QLatin1String("qrc:html/box-border.png"));
        sPage.replace(QLatin1String("%UP-IMG%"), QzTools::pixmapToByteArray(IconProvider::standardIcon(QStyle::SP_FileDialogToParent).pixmap(22)));
        sPage.replace(QLatin1String("%UP-DIR-TEXT%"), tr("Up to higher level directory"));
        sPage.replace(QLatin1String("%SHOW-HIDDEN-TEXT%"), tr("Show hidden files"));
        sPage.replace(QLatin1String("%NAME%"), tr("Name"));
        sPage.replace(QLatin1String("%SIZE%"), tr("Size"));
        sPage.replace(QLatin1String("%MODIFIED%"), tr("Last modified"));
        sPage = QzTools::applyDirectionToPage(sPage);
    }

    QString page = sPage;

    QByteArray titleByteArray = QByteArray::fromPercentEncoding(url().toString().toUtf8());
    QString title;
    if (QzTools::isUtf8(titleByteArray.constData())) {
        title = QString::fromUtf8(titleByteArray);
    }
    else {
        title = QString::fromLatin1(titleByteArray);
    }
    page.replace(QLatin1String("%TITLE%"), tr("Index for %1").arg(QzTools::escape(title)));
    page.replace(QLatin1String("%CLICKABLE-TITLE%"), tr("Index for %1").arg(clickableSections(title)));

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

    foreach (const QUrlInfo &item, m_items) {

        if (item.name() == QLatin1String(".") || item.name() == QLatin1String("..")) {
            continue;
        }

        QString line = QLatin1String("<tr");
        QUrl itemUrl = u.resolved(QUrl(QUrl::toPercentEncoding(item.name())));
        QString itemPath = itemUrl.path();

        if (itemPath.endsWith(QLatin1Char('/'))) {
            itemPath.remove(itemPath.size() - 1, 1);
        }

        QIcon itemIcon;
        if (item.isSymLink()) {
            itemIcon = IconProvider::standardIcon(QStyle::SP_DirLinkIcon);
        }
        else if (item.isFile()) {
            itemIcon = QzTools::iconFromFileName(itemPath);
        }
        else {
            itemIcon = IconProvider::standardIcon(QStyle::SP_DirIcon);
        }

        line += QLatin1String("><td class=\"td-name\" style=\"background-image:url(data:image/png;base64,");
        line += QzTools::pixmapToByteArray(itemIcon.pixmap(16));
        line += QLatin1String(");\">");
        line += QLatin1String("<a href=\"");
        line += itemUrl.toEncoded();
        line += QLatin1String("\">");
        line += QzTools::escape(item.name());
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

QString FtpSchemeReply::clickableSections(const QString &path)
{
    QString title = path;
    title.remove(QLatin1String("ftp://"));
    QStringList sections = title.split(QLatin1Char('/'), QString::SkipEmptyParts);
    if (sections.isEmpty()) {
        return QString("<a href=\"%1\">%1</a>").arg(path);
    }
    sections[0].prepend(QLatin1String("ftp://"));

    title.clear();
    for (int i = 0; i < sections.size(); ++i) {
        QStringList currentParentSections = sections.mid(0, i + 1);
        QUrl currentParentUrl = QUrl(currentParentSections.join(QLatin1String("/")));
        title += QString("<a href=\"%1\">%2</a>/").arg(currentParentUrl.toEncoded(), QzTools::escape(sections.at(i)));
    }

    return title;
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
            QByteArray lastSection = QByteArray::fromPercentEncoding(sections.takeLast().toUtf8());
            if (QzTools::isUtf8(lastSection.constData())) {
                m_probablyFileForDownload = QString::fromUtf8(lastSection);
            }
            else {
                m_probablyFileForDownload = QString::fromLatin1(lastSection);
            }
        }
        if (!m_probablyFileForDownload.isEmpty()) {
            m_isGoingToDownload = true;
            QString parentOfPath = QString("/%1/").arg(sections.join(QLatin1String("/")));
            m_ftpCdId = m_ftp->cd(QString::fromLatin1(QByteArray::fromPercentEncoding(parentOfPath.toUtf8())));
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
    connect(this, SIGNAL(commandFinished(int,bool)), this, SLOT(processCommand(int,bool)));
    connect(this, SIGNAL(done(bool)), this, SLOT(onDone(bool)));
}

void FtpDownloader::download(const QUrl &url, QIODevice* dev)
{
    m_url = QUrl(QString::fromLatin1(QByteArray::fromPercentEncoding(url.toString().toUtf8())));
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

bool FtpDownloader::isFinished()
{
    return m_isFinished;
}

QUrl FtpDownloader::url() const
{
    return m_url;
}

QIODevice* FtpDownloader::device() const
{
    return m_dev;
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
        && QFtp::error() == QFtp::NoError
       ) {
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
