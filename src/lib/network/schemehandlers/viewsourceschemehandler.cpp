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
#include "networkmanager.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "sourcehighlighter.h"
#include "settings.h"

#include <QTextStream>
#include <QTextCodec>
#include <QWebSecurityOrigin>
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

    ViewSourceSchemeReply* reply = new ViewSourceSchemeReply(request);
    return reply;
}


ViewSourceSchemeReply::ViewSourceSchemeReply(const QNetworkRequest &req, QObject* parent)
    : QNetworkReply(parent)
{
    setOperation(QNetworkAccessManager::GetOperation);
    setRequest(req);
    setUrl(req.url());

	QUrl sourceUrl = QUrl(req.url().toString().mid(12).toUtf8());
	m_reply = mApp->networkManager()->get(QNetworkRequest(sourceUrl));
	connect(m_reply, SIGNAL(finished()), this, SLOT(loadPage()));

    m_buffer.open(QIODevice::ReadWrite);
    setError(QNetworkReply::NoError, tr("No Error"));

	open(QIODevice::ReadOnly);
}

qint64 ViewSourceSchemeReply::bytesAvailable() const
{
    return m_buffer.bytesAvailable() + QNetworkReply::bytesAvailable();
}

qint64 ViewSourceSchemeReply::readData(char* data, qint64 maxSize)
{
    return m_buffer.read(data, maxSize);
}

void ViewSourceSchemeReply::loadPage(){
	QWebSecurityOrigin::addLocalScheme("view-source");

	QTextStream stream(&m_buffer);
	QString charset = "utf-8";

	int replyStatus = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if (replyStatus == 301 || replyStatus == 302){
		stream << "<head>"
			<< "<meta HTTP-EQUIV=\"REFRESH\" content=\"0; url=view-source:"+m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString()+"\">"
			<< "</head>";
	}
	else{
	    QString html = m_reply->readAll();
	
		QRegExp rx("<meta.*charset=[\"]?([a-z0-9-]+)");
		if (rx.indexIn(html, 0) != -1)
			charset = rx.cap(1);
	
		SourceHighlighter sh(html);
		sh.setTitle("view-source:"+m_reply->url().toString());
		html = sh.highlight();
	
		QTextCodec *codec = QTextCodec::codecForHtml(html.toStdString().c_str());
	    stream.setCodec(codec);
		stream << html;
	}

	stream.flush();
    m_buffer.reset();

	setHeader(QNetworkRequest::ContentTypeHeader, QString("text/html; charset="+charset));
    setHeader(QNetworkRequest::ContentLengthHeader, m_buffer.bytesAvailable());
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
    setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QByteArray("Ok"));

    emit metaDataChanged();
    emit downloadProgress(m_buffer.size(), m_buffer.size());

    emit readyRead();
    emit finished();
	QWebSecurityOrigin::removeLocalScheme("view-source");
}
