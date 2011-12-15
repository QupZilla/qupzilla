/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#include "speeddial.h"
#include "pluginproxy.h"

QString authorString(const QString &name, const QString &mail)
{
    return QString("%1 &lt;<a href=\"mailto:%2\">%2</a>&gt;").arg(name, mail);
}

QupZillaSchemeHandler::QupZillaSchemeHandler(QObject* parent)
    : QObject(parent)
{
}

QNetworkReply* QupZillaSchemeHandler::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
{
    Q_UNUSED(outgoingData)

    if (op != QNetworkAccessManager::GetOperation) {
        return 0;
    }

    QupZillaSchemeReply* reply = new QupZillaSchemeReply(request);
    return reply;
}

QupZillaSchemeReply::QupZillaSchemeReply(const QNetworkRequest &req, QObject* parent)
    : QNetworkReply(parent)
{
    setOperation(QNetworkAccessManager::GetOperation);
    setRequest(req);
    setUrl(req.url());

    m_pageName = req.url().path();
    if (m_pageName == "about" || m_pageName == "reportbug" || m_pageName == "start" || m_pageName == "speeddial") {
        m_buffer.open(QIODevice::ReadWrite);
        setError(QNetworkReply::NoError, tr("No Error"));

        QTimer::singleShot(0, this, SLOT(loadPage()));
        open(QIODevice::ReadOnly);
    }
    else {
        setError(QNetworkReply::HostNotFoundError, tr("Not Found"));
        QTimer::singleShot(0, this, SLOT(delayedFinish()));
    }
}

void QupZillaSchemeReply::loadPage()
{
    QTextStream stream(&m_buffer);

    if (m_pageName == "about") {
        stream << aboutPage();
    }
    else if (m_pageName == "reportbug") {
        stream << reportbugPage();
    }
    else if (m_pageName == "start") {
        stream << startPage();
    }
    else if (m_pageName == "speeddial") {
        stream << speeddialPage();
    }

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

qint64 QupZillaSchemeReply::readData(char* data, qint64 maxSize)
{
    return m_buffer.read(data, maxSize);
}

QString QupZillaSchemeReply::reportbugPage()
{
    QString page;
    page.append(qz_readAllFileContents(":html/reportbug.html"));
    page.replace("%FAVICON%", "qrc:icons/qupzilla.png");
    page.replace("%BOX-BORDER%", "qrc:html/box-border.png");

    page.replace("%TITLE%", tr("Report Issue"));
    page.replace("%REPORT-ISSUE%", tr("Report Issue"));
    page.replace("%PLUGINS-TEXT%", tr("If you are experiencing problems with QupZilla, please try first disable"
                                      " all plugins. <br/>If it won't help, then please fill this form: "));
    page.replace("%EMAIL%", tr("Your E-mail"));
    page.replace("%TYPE%", tr("Issue type"));
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
    page.replace("%FAVICON%", "qrc:icons/qupzilla.png");
    page.replace("%BOX-BORDER%", "qrc:html/box-border.png");
    page.replace("%ABOUT-IMG%", "qrc:icons/other/about.png");

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
    page.replace("%FAVICON%", "qrc:icons/qupzilla.png");
    page.replace("%BOX-BORDER%", "qrc:html/box-border.png");
    page.replace("%ABOUT-IMG%", "qrc:icons/other/about.png");
    page.replace("%COPYRIGHT-INCLUDE%", Qt::escape(qz_readAllFileContents(":html/copyright")));

    page.replace("%TITLE%", tr("About QupZilla"));
    page.replace("%ABOUT-QUPZILLA%", tr("About QupZilla"));
    page.replace("%INFORMATIONS-ABOUT-VERSION%", tr("Informations about version"));
    page.replace("%BROWSER-IDENTIFICATION%", tr("Browser Identification"));
    page.replace("%PATHS%", tr("Paths"));
    page.replace("%COPYRIGHT%", tr("Copyright"));

    page.replace("%VERSION-INFO%",
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Version"), QupZilla::VERSION) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("WebKit version"), QupZilla::WEBKITVERSION) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Build time"), QupZilla::BUILDTIME) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Platform"), qz_buildSystem()));
    page.replace("%USER-AGENT%", WebPage::UserAgent);
    page.replace("%PATHS-TEXT%",
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Profile"), mApp->getActiveProfilPath()) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Settings"), mApp->getActiveProfilPath() + "settings.ini") +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Saved session"), mApp->getActiveProfilPath() + "session.dat") +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Pinned tabs"), mApp->getActiveProfilPath() + "pinnedtabs.dat") +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Data"), mApp->DATADIR) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Themes"), mApp->THEMESDIR) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Plugins"), mApp->PLUGINSDIR) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Translations"), mApp->TRANSLATIONSDIR));
    page.replace("%MAIN-DEVELOPER%", tr("Main developer"));
    page.replace("%MAIN-DEVELOPER-TEXT%", authorString(QupZilla::AUTHOR, "nowrep@gmail.com"));
    page.replace("%CONTRIBUTORS%", tr("Contributors"));
    page.replace("%CONTRIBUTORS-TEXT%", authorString("Daniele Cocca", "jmc@chakra-project.org") + "<br/>" +
                 authorString("Jan Rajnoha", "honza.rajny@hotmail.com")
                );
    page.replace("%TRANSLATORS%", tr("Translators"));
    page.replace("%TRANSLATORS-TEXT%", authorString("Heimen Stoffels", "vistausss@gmail.com") + " (Dutch)<br/>" +
                 authorString("Peter Vacula", "pvacula1989@gmail.com") + " (Slovak)<br/>" +
                 authorString("Ján Ďanovský", "dagsoftware@yahoo.com") + " (Slovak)<br/>" +
                 authorString("Jonathan Hooverman", "jonathan.hooverman@gmail.com") + " (German)<br/>" +
                 authorString("Unink-Lio", "unink4451@163.com") + " (Chinese)<br/>" +
                 authorString("Federico Fabiani", "federico.fabiani85@gmail.com") + " (Italy)<br/>" +
                 authorString("Francesco Marinucci", "framarinucci@gmail.com") + " (Italy)<br/>" +
                 authorString("Jorge Sevilla", "jsevi@ozu.es") + " (Spanish)<br/>" +
                 authorString("Michał Szymanowski", "tylkobuba@gmail.com") + " (Polish)"
                );

    return page;
}

QString QupZillaSchemeReply::speeddialPage()
{
    QString page;
    page.append(qz_readAllFileContents(":html/speeddial.html"));
    page.replace("%FAVICON%", "qrc:icons/qupzilla.png");
    page.replace("%IMG_PLUS%", "qrc:html/plus.png");
    page.replace("%BOX-BORDER%", "qrc:html/box-border-small.png");
    page.replace("%IMG_CLOSE%", "qrc:html/close.png");
    page.replace("%IMG_EDIT%", "qrc:html/edit.png");
    page.replace("%IMG_RELOAD%", "qrc:html/reload.png");

    page.replace("%SITE-TITLE%", tr("Speed Dial"));
    page.replace("%ADD-TITLE%", tr("Add New Page"));
    page.replace("%TITLE-EDIT%", tr("Edit"));
    page.replace("%TITLE-REMOVE%", tr("Remove"));
    page.replace("%TITLE-RELOAD%", tr("Reload"));
    page.replace("%JQUERY%", "qrc:html/jquery.js");
    page.replace("%JQUERY-UI%", "qrc:html/jquery-ui.js");
    page.replace("%LOADING-IMG%", mApp->plugins()->speedDial()->loadingImagePath());
    page.replace("%URL%", tr("Url"));
    page.replace("%TITLE%", tr("Title"));
    page.replace("%EDIT%", tr("Edit"));
    page.replace("%NEW-PAGE%", tr("New Page"));
    page.replace("%INITIAL-SCRIPT%", mApp->plugins()->speedDial()->initialScript());

    return page;
}
