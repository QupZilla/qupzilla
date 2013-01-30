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
#include "viewsourceschemehandler.h"
#include "qztools.h"
#include "iconprovider.h"
//#include "downloadoptionsdialog.h"
#include "networkmanager.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "sourcehighlighter.h"

#include <QTextStream>
#include <QDateTime>
#include <QTimer>
#include <QDir>
#include <QDebug>

ViewSourceSchemeHandler::ViewSourceSchemeHandler()
{
}

QNetworkReply* ViewSourceSchemeHandler::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
{
    Q_UNUSED(outgoingData)

    if (op != QNetworkAccessManager::GetOperation) {
        return 0;
    }

	qDebug() << "VS::createRequest";

	//QString urlSource = QUrl::fromPercentEncoding(request.url().toString().mid(12).toUtf8());

    //FollowRedirectReply* reply = new FollowRedirectReply(QUrl(urlSource), mApp->networkManager());
	//connect(reply, SIGNAL(finished()), this, SLOT(finished()));
	//ViewSourceSchemeReply* reply = new ViewSourceSchemeReply(QUrl(urlSource), mApp->networkManager());
    ViewSourceSchemeReply* reply = new ViewSourceSchemeReply(request);
	//QNetworkReply* reply = new QNetworkReply(request);
    return reply;
}

/*void ViewSourceSchemeHandler::finished()
{
	qDebug() << "finished";
}*/

/*void ViewSourceSchemeHandler::handleUrl(const QUrl &url)
{
    QFileIconProvider iconProvider;
    QFile file(url.toLocalFile());
    QFileInfo info(file);

    if (!info.exists() || info.isDir() || !info.isReadable()) {
        return;
    }

    const QString &fileName = info.fileName();
    const QPixmap &pixmap = iconProvider.icon(info).pixmap(30);
    const QString &type = iconProvider.type(info);

    DownloadOptionsDialog dialog(fileName, pixmap, type, url, mApp->getWindow());
    dialog.showExternalManagerOption(false);
    dialog.showFromLine(false);

    int status = dialog.exec();

    if (status == 1) {
        // Open
        QDesktopServices::openUrl(url);
    }
    else if (status == 2) {
        // Save
        const QString &savePath = QFileDialog::getSaveFileName(mApp->getWindow(),
                                  QObject::tr("Save file as..."),
                                  QDir::homePath() + "/" + QzTools::getFileNameFromUrl(url));

        if (!savePath.isEmpty()) {
            file.copy(savePath);
        }
    }
}*/



ViewSourceSchemeReply::ViewSourceSchemeReply(const QNetworkRequest &req, QObject* parent)
    : QNetworkReply(parent)
{
    setOperation(QNetworkAccessManager::GetOperation);
    setRequest(req);
    setUrl(req.url());

	QUrl sourceUrl = QUrl(req.url().toString().mid(12).toUtf8());
	m_reply = new FollowRedirectReply(sourceUrl, mApp->networkManager());
	connect(m_reply, SIGNAL(finished()), this, SLOT(loadPage()));
	//QNetworkReply* reply = mApp->networkManager()->get(req);

    m_buffer.open(QIODevice::ReadWrite);
    setError(QNetworkReply::NoError, tr("No Error"));

	//connect(this, SIGNAL(readyRead()), this, SLOT(readData()));
    //QTimer::singleShot(0, this, SLOT(loadPage()));
	open(QIODevice::ReadOnly);
	//connect(this, SIGNAL(finished()), this, SLOT(finished()));
}

qint64 ViewSourceSchemeReply::bytesAvailable() const
{
    return m_buffer.bytesAvailable() + QNetworkReply::bytesAvailable();
}

qint64 ViewSourceSchemeReply::readData(char* data, qint64 maxSize)
{
    return m_buffer.read(data, maxSize);
}


/*void ViewSourceSchemeReply::loadPage(){
	qDebug() << "loadPage()";
    QNetworkReply* reply = qobject_cast<QNetworkReply*> (sender());
    if (!reply) {
        return;
    }
    int replyStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	qDebug() << QString::number(replyStatus);

	QTextStream stream(&m_buffer);
	//stream.setCodec("UTF-8");
	stream << reply->readAll();

	stream.flush();
    m_buffer.reset();

    if ((replyStatus != 301 && replyStatus != 302)){// || m_redirectCount == 5) {
        emit finished();
        return;
    }
}*/

void ViewSourceSchemeReply::loadPage(){
    //FollowRedirectReply* reply = qobject_cast<FollowRedirectReply*> (sender());
    //if (!reply) {
	//	return;
    //}
	setUrl(m_reply->url());
    QString html = m_reply->readAll().data();

	QRegExp rx("<meta.*charset=[\"]?([a-z0-9-]+)");
	QString charset = "";
	if (rx.indexIn(html, 0) != -1)
		charset = rx.cap(1);
	else
		charset = "utf-8";

	//qDebug() << html;
	SourceHighlighter sh(html);
	sh.setTitle("view-source:"+m_reply->url().toString());
	html = sh.highlight();

	QTextStream stream(&m_buffer);
	//setLocale
	//qDebug() << stream.codec()->name();
	//qDebug() << stream.locale().name();
	//stream.setAutoDetectUnicode(false);
	//qDebug() << "detect " << stream.autoDetectUnicode();
    //stream.setCodec(charset.data());
	//stream.setCodec("UTF-8");
	//QzTools::readAllFileContents(":html/view-source.css");
	stream << html;

	stream.flush();
    m_buffer.reset();

    setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("text/html"));
	//setHeader(QNetworkRequest::ContentTypeHeader, QString("text/html; charset="+charset));
	setHeader(QNetworkRequest::LocationHeader, m_reply->url().toString());
    setHeader(QNetworkRequest::ContentLengthHeader, m_buffer.bytesAvailable());
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
    setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QByteArray("Ok"));
    emit metaDataChanged();
    emit downloadProgress(m_buffer.size(), m_buffer.size());

    emit readyRead();
    emit finished();
}

/*void ViewSourceSchemeReply::finished()
{
	qDebug() << "ViewSourceSchemeReply::finished";
}*/
