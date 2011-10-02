/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "qupzillaschemehandler.h"
#include "globalfunctions.h"
#include "qupzilla.h"
#include "mainapplication.h"
#include "webpage.h"

QupZillaSchemeHandler::QupZillaSchemeHandler(QObject* parent) :
    QObject(parent)
{
}

QNetworkReply* QupZillaSchemeHandler::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
{
    Q_UNUSED(outgoingData)

    if (op != QNetworkAccessManager::GetOperation)
        return 0;

    QupZillaSchemeReply* reply = new QupZillaSchemeReply(request);
    return reply;
}

QupZillaSchemeReply::QupZillaSchemeReply(const QNetworkRequest &req, QObject *parent)
    : QNetworkReply(parent)
{
    setOperation(QNetworkAccessManager::GetOperation);
    setRequest(req);
    setUrl(req.url());

    m_pageName = req.url().path();
    if (m_pageName == "about" || m_pageName == "reportbug" || m_pageName == "start") {
        m_buffer.open(QIODevice::ReadWrite);
        setError(QNetworkReply::NoError, tr("No Error"));

        QTimer::singleShot(0, this, SLOT(loadPage()));
        open(QIODevice::ReadOnly);
    } else {
        setError(QNetworkReply::HostNotFoundError, tr("Not Found"));
        QTimer::singleShot(0, this, SLOT(delayedFinish()));
    }
}

void QupZillaSchemeReply::loadPage()
{
    QTextStream stream(&m_buffer);
    if (m_pageName == "about")
        stream << aboutPage();
    else if (m_pageName == "reportbug")
        stream << reportbugPage();
    else if (m_pageName == "start")
        stream << startPage();

    stream.flush();
    m_buffer.reset();

    setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("text/html"));
    setHeader(QNetworkRequest::ContentLengthHeader, m_buffer.bytesAvailable());
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
    setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QByteArray("Ok"));
    emit metaDataChanged();
    emit downloadProgress(m_buffer.size(), m_buffer.size());

    emit readyRead();
    emit finished();
}

void QupZillaSchemeReply::delayedFinish()
{
    emit error(QNetworkReply::HostNotFoundError);
    emit finished();
}

qint64 QupZillaSchemeReply::bytesAvailable() const
{
    return m_buffer.bytesAvailable() + QNetworkReply::bytesAvailable();
}

qint64 QupZillaSchemeReply::readData(char *data, qint64 maxSize)
{
    return m_buffer.read(data, maxSize);
}

QString QupZillaSchemeReply::reportbugPage()
{
    QString page;
    page.append(qz_readAllFileContents(":html/reportbug.html"));
    page.replace("%FAVICON%", qz_pixmapToByteArray(QPixmap(":icons/qupzilla.png")));
    page.replace("%BOX-BORDER%", qz_pixmapToByteArray(QPixmap(":html/box-border.png")));

    page.replace("%TITLE%", tr("Report issue"));
    page.replace("%REPORT-ISSUE%", tr("Report issue"));
    page.replace("%PLUGINS-TEXT%", tr("If You are experiencing problems with QupZilla, please try first disable"
                                      " all plugins. <br/>If it won't help, then please fill this form: "));
    page.replace("%EMAIL%", tr("Your E-mail"));
    page.replace("%TYPE%", tr("Issue type"));
    page.replace("%PRIORITY%", tr("Priority"));
    page.replace("%LOW%", tr("Low"));
    page.replace("%NORMAL%", tr("Normal"));
    page.replace("%HIGH%", tr("High"));
    page.replace("%DESCRIPTION%", tr("Issue description"));
    page.replace("%SEND%", tr("Send"));
    page.replace("%E-MAIL-OPTIONAL%", tr("E-mail is optional"));
    page.replace("%FIELDS-ARE-REQUIRED%", tr("Please fill all required fields!"));
    return page;
}

QString QupZillaSchemeReply::startPage()
{
    QString page;
    page.append(qz_readAllFileContents(":html/start.html"));
    page.replace("%FAVICON%", qz_pixmapToByteArray(QPixmap(":icons/qupzilla.png")));
    page.replace("%BOX-BORDER%", qz_pixmapToByteArray(QPixmap(":html/box-border.png")));
    page.replace("%ABOUT-IMG%", qz_pixmapToByteArray(QPixmap(":icons/other/about.png")));

    page.replace("%TITLE%", tr("Start Page"));
    page.replace("%BUTTON-LABEL%", tr("Google Search"));
    page.replace("%SEARCH-BY-GOOGLE%", tr("Search results provided by Google"));
    page.replace("%WWW%", QupZilla::WIKIADDRESS);
    page.replace("%ABOUT-QUPZILLA%", tr("About QupZilla"));

    return page;
}

QString QupZillaSchemeReply::aboutPage()
{
    QString page;
    page.append(qz_readAllFileContents(":html/about.html"));
    page.replace("%FAVICON%", qz_pixmapToByteArray(QPixmap(":icons/qupzilla.png")));
    page.replace("%BOX-BORDER%", qz_pixmapToByteArray(QPixmap(":html/box-border.png")));
    page.replace("%ABOUT-IMG%", qz_pixmapToByteArray(QPixmap(":icons/other/about.png")));
    page.replace("%COPYRIGHT-INCLUDE%", qz_readAllFileContents(":html/copyright"));

    page.replace("%TITLE%", tr("About QupZilla"));
    page.replace("%ABOUT-QUPZILLA%", tr("About QupZilla"));
    page.replace("%INFORMATIONS-ABOUT-VERSION%", tr("Informations about version"));
    page.replace("%BROWSER-IDENTIFICATION%", tr("Browser Identification"));
    page.replace("%PATHS%", tr("Paths"));
    page.replace("%COPYRIGHT%", tr("Copyright"));

    QString platform;
#ifdef Q_WS_X11
    platform = tr("Linux");
#endif
#ifdef Q_WS_WIN
    platform = tr("Windows");
#endif
    page.replace("%VERSION-INFO%",
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Version"), QupZilla::VERSION) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("WebKit version"), QupZilla::WEBKITVERSION) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Build time"), QupZilla::BUILDTIME) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Platform"), platform)
                 );
    page.replace("%USER-AGENT%", mApp->getWindow()->weView()->webPage()->userAgentForUrl(QUrl()));
    page.replace("%PATHS-TEXT%",
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Settings"), mApp->getActiveProfilPath() + "settings.ini") +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Saved session"), mApp->getActiveProfilPath() + "session.dat") +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Pinned tabs"), mApp->getActiveProfilPath() + "pinnedtabs.dat") +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Themes"), mApp->THEMESDIR) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Plugins"), mApp->PLUGINSDIR) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Translations"), mApp->TRANSLATIONSDIR)
                 );

    return page;
}
