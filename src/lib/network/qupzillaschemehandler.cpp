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
#include "qupzillaschemehandler.h"
#include "globalfunctions.h"
#include "qupzilla.h"
#include "mainapplication.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "speeddial.h"
#include "pluginproxy.h"
#include "plugininterface.h"
#include "settings.h"

#include <QTextDocument>
#include <QTextStream>
#include <QTimer>
#include <QSettings>

QString authorString(const char* name, const QString &mail)
{
    return QString("%1 &lt;<a href=\"mailto:%2\">%2</a>&gt;").arg(QString::fromUtf8(name), mail);
}

QupZillaSchemeHandler::QupZillaSchemeHandler()
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

    QStringList knownPages;
    knownPages << "about" << "reportbug" << "start" << "speeddial" << "config";

    if (knownPages.contains(m_pageName)) {
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
    stream.setCodec("UTF-8");

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
    else if (m_pageName == "config") {
        stream << configPage();
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
    static QString bPage;

    if (!bPage.isEmpty()) {
        return bPage;
    }

    bPage.append(qz_readAllFileContents(":html/reportbug.html"));
    bPage.replace("%FAVICON%", "qrc:icons/qupzilla.png");
    bPage.replace("%BOX-BORDER%", "qrc:html/box-border.png");

    bPage.replace("%TITLE%", tr("Report Issue"));
    bPage.replace("%REPORT-ISSUE%", tr("Report Issue"));
    bPage.replace("%PLUGINS-TEXT%", tr("If you are experiencing problems with QupZilla, please try to disable"
                                       " all extensions first. <br/>If this does not fix it, then please fill out this form: "));
    bPage.replace("%EMAIL%", tr("Your E-mail"));
    bPage.replace("%TYPE%", tr("Issue type"));
    bPage.replace("%DESCRIPTION%", tr("Issue description"));
    bPage.replace("%SEND%", tr("Send"));
    bPage.replace("%E-MAIL-OPTIONAL%", tr("E-mail is optional<br/><b>Note: </b>Please read how to make a bug report <a href=%1>here</a> first.").arg("https://github.com/nowrep/QupZilla/wiki/Bug-Reports target=_blank"));
    bPage.replace("%FIELDS-ARE-REQUIRED%", tr("Please fill out all required fields!"));

    return bPage;
}

QString QupZillaSchemeReply::startPage()
{
    static QString sPage;

    if (!sPage.isEmpty()) {
        return sPage;
    }

    sPage.append(qz_readAllFileContents(":html/start.html"));
    sPage.replace("%FAVICON%", "qrc:icons/qupzilla.png");
    sPage.replace("%BOX-BORDER%", "qrc:html/box-border.png");
    sPage.replace("%ABOUT-IMG%", "qrc:icons/other/about.png");

    sPage.replace("%TITLE%", tr("Start Page"));
    sPage.replace("%BUTTON-LABEL%", tr("Google Search"));
    sPage.replace("%SEARCH-BY-GOOGLE%", tr("Search results provided by Google"));
    sPage.replace("%WWW%", QupZilla::WIKIADDRESS);
    sPage.replace("%ABOUT-QUPZILLA%", tr("About QupZilla"));

    return sPage;
}

QString QupZillaSchemeReply::aboutPage()
{
    static QString aPage;

    if (aPage.isEmpty()) {
        aPage.append(qz_readAllFileContents(":html/about.html"));
        aPage.replace("%FAVICON%", "qrc:icons/qupzilla.png");
        aPage.replace("%BOX-BORDER%", "qrc:html/box-border.png");
        aPage.replace("%ABOUT-IMG%", "qrc:icons/other/about.png");
        aPage.replace("%COPYRIGHT-INCLUDE%", Qt::escape(qz_readAllFileContents(":html/copyright")));

        aPage.replace("%TITLE%", tr("About QupZilla"));
        aPage.replace("%ABOUT-QUPZILLA%", tr("About QupZilla"));
        aPage.replace("%INFORMATIONS-ABOUT-VERSION%", tr("Information about version"));
        aPage.replace("%COPYRIGHT%", tr("Copyright"));

        aPage.replace("%VERSION-INFO%",
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Version"), QupZilla::VERSION
#ifdef GIT_REVISION
                              + " (" + GIT_REVISION + ")"
#endif
                                                          ) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("WebKit version"), QupZilla::WEBKITVERSION));
        aPage.replace("%MAIN-DEVELOPER%", tr("Main developer"));
        aPage.replace("%MAIN-DEVELOPER-TEXT%", authorString(QupZilla::AUTHOR.toUtf8(), "nowrep@gmail.com"));
        aPage.replace("%CONTRIBUTORS%", tr("Contributors"));
        aPage.replace("%CONTRIBUTORS-TEXT%",
                      authorString("Mladen Pejaković", "pejakm@gmail.com") + "<br/>" +
                      authorString("Bryan M Dunsmore", "dunsmoreb@gmail.com") + "<br/>" +
                      authorString("Mariusz Fik", "fisiu@opensuse.org") + "<br/>" +
                      authorString("Jan Rajnoha", "honza.rajny@hotmail.com")  + "<br/>" +
                      authorString("Daniele Cocca", "jmc@chakra-project.org")
                     );
        aPage.replace("%TRANSLATORS%", tr("Translators"));
        aPage.replace("%TRANSLATORS-TEXT%",
                      authorString("Heimen Stoffels", "vistausss@gmail.com") + " (Dutch)<br/>" +
                      authorString("Peter Vacula", "pvacula1989@gmail.com") + " (Slovak)<br/>" +
                      authorString("Ján Ďanovský", "dagsoftware@yahoo.com") + " (Slovak)<br/>" +
                      authorString("Jonathan Hooverman", "jonathan.hooverman@gmail.com") + " (German)<br/>" +
                      authorString("Federico Fabiani", "federico.fabiani85@gmail.com") + " (Italian)<br/>" +
                      authorString("Francesco Marinucci", "framarinucci@gmail.com") + " (Italian)<br/>" +
                      authorString("Jorge Sevilla", "jsevi@ozu.es") + " (Spanish)<br/>" +
                      authorString("Michał Szymanowski", "tylkobuba@gmail.com") + " (Polish)<br/>" +
                      authorString("Mariusz Fik", "fisiu@opensuse.org") + " (Polish)<br/>" +
                      authorString("Jérôme Giry", "baikalink@hotmail.fr") + " (French)<br/>" +
                      authorString("Nicolas Ourceau", "lamessen@hotmail.fr") + " (French)<br/>" +
                      authorString("Vasilis Tsivikis", "vasitsiv.dev@gmail.com") + " (Greek)<br/>" +
                      authorString("Rustam Salakhutdinov", "salahutd@gmail.com") + " (Russian)<br/>" +
                      authorString("Oleg Brezhnev", "oleg-423@yandex.ru") + " (Russian)<br/>" +
                      authorString("Sérgio Marques", "smarquespt@gmail.com") + " (Portuguese)<br/>" +
                      authorString("Alexandre Carvalho", "alexandre05@live.com") + " (Brazilian Portuguese)<br/>" +
                      authorString("Mladen Pejaković", "pejakm@gmail.com") + " (Serbian)<br/>" +
                      authorString("Unink-Lio", "unink4451@163.com") + " (Chinese)<br/>" +
                      authorString("Wu Cheng-Hong", "stu2731652@gmail.com") + " (Traditional Chinese)<br/>" +
                      authorString("Widya Walesa", "walecha99@gmail.com") + " (Indonesian)<br/>" +
                      authorString("Beqa Arabuli", "arabulibeqa@gmail.com") + " (Georgian)<br/>" +
                      authorString("Daiki Noda", "sys.pdr.pdm9@gmail.com") + " (Japanese)"
                     );
    }

    return aPage;
}

QString QupZillaSchemeReply::speeddialPage()
{
    static QString dPage;

    if (dPage.isEmpty()) {
        dPage.append(qz_readAllFileContents(":html/speeddial.html"));
        dPage.replace("%FAVICON%", "qrc:icons/qupzilla.png");
        dPage.replace("%IMG_PLUS%", "qrc:html/plus.png");
        dPage.replace("%BOX-BORDER%", "qrc:html/box-border-small.png");
        dPage.replace("%IMG_CLOSE%", "qrc:html/close.png");
        dPage.replace("%IMG_EDIT%", "qrc:html/edit.png");
        dPage.replace("%IMG_RELOAD%", "qrc:html/reload.png");

        dPage.replace("%SITE-TITLE%", tr("Speed Dial"));
        dPage.replace("%ADD-TITLE%", tr("Add New Page"));
        dPage.replace("%TITLE-EDIT%", tr("Edit"));
        dPage.replace("%TITLE-REMOVE%", tr("Remove"));
        dPage.replace("%TITLE-RELOAD%", tr("Reload"));
        dPage.replace("%TITLE-FETCHTITLE%", tr("Load title from page"));
        dPage.replace("%JQUERY%", "qrc:html/jquery.js");
        dPage.replace("%JQUERY-UI%", "qrc:html/jquery-ui.js");
        dPage.replace("%LOADING-IMG%", "qrc:html/loading.gif");
        dPage.replace("%URL%", tr("Url"));
        dPage.replace("%TITLE%", tr("Title"));
        dPage.replace("%APPLY%", tr("Apply"));
        dPage.replace("%CLOSE%", tr("Close"));
        dPage.replace("%NEW-PAGE%", tr("New Page"));
        dPage.replace("%IMG_SETTINGS%", "qrc:html/setting.png");
        dPage.replace("%SETTINGS-TITLE%", tr("Speed Dial settings"));
        dPage.replace("%TXT_PLACEMENT%", tr("Placement: "));
        dPage.replace("%TXT_AUTO%", tr("Auto"));
        dPage.replace("%TXT_COVER%", tr("Cover"));
        dPage.replace("%TXT_FIT%", tr("Fit"));
        dPage.replace("%TXT_FWIDTH%", tr("Fit Width"));
        dPage.replace("%TXT_FHEIGHT%", tr("Fit Height"));
        dPage.replace("%TXT_NOTE%", tr("Use background image"));
        dPage.replace("%TXT_SELECTIMAGE%", tr("Select image"));
        dPage.replace("%TXT_NRROWS%", tr("Maximum pages in a row:"));
        dPage.replace("%TXT_SDSIZE%", tr("Change size of pages:"));
    }

    QString page = dPage;
    SpeedDial* dial = mApp->plugins()->speedDial();

    page.replace("%INITIAL-SCRIPT%", dial->initialScript());
    page.replace("%IMG_BACKGROUND%", dial->backgroundImage());
    page.replace("%B_SIZE%", dial->backgroundImageSize());
    page.replace("%ROW-PAGES%", QString::number(dial->pagesInRow()));
    page.replace("%SD-SIZE%", QString::number(dial->sdSize()));

    return page;
}


QString QupZillaSchemeReply::configPage()
{
    static QString cPage;

    if (cPage.isEmpty()) {
        cPage.append(qz_readAllFileContents(":html/config.html"));
        cPage.replace("%FAVICON%", "qrc:icons/qupzilla.png");
        cPage.replace("%BOX-BORDER%", "qrc:html/box-border.png");
        cPage.replace("%ABOUT-IMG%", "qrc:icons/other/about.png");

        cPage.replace("%TITLE%", tr("Configuration Information"));
        cPage.replace("%CONFIG%", tr("Configuration Information"));
        cPage.replace("%INFORMATIONS-ABOUT-VERSION%", tr("Information about version"));
        cPage.replace("%CONFIG-ABOUT%", tr("This page contains information about QupZilla's current configuration - relevant for troubleshooting. Please include this information when submitting bug reports."));
        cPage.replace("%BROWSER-IDENTIFICATION%", tr("Browser Identification"));
        cPage.replace("%PATHS%", tr("Paths"));
        cPage.replace("%BUILD-CONFIG%", tr("Build Configuration"));
        cPage.replace("%PREFS%", tr("Preferences"));
        cPage.replace("%OPTION%", tr("Option"));
        cPage.replace("%VALUE%", tr("Value"));
        cPage.replace("%PLUGINS%", tr("Extensions"));
        cPage.replace("%PL-NAME%", tr("Name"));
        cPage.replace("%PL-VER%", tr("Version"));
        cPage.replace("%PL-AUTH%", tr("Author"));
        cPage.replace("%PL-DESC%", tr("Description"));

        cPage.replace("%VERSION-INFO%",
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Application version"), QupZilla::VERSION
#ifdef GIT_REVISION
                              + " (" + GIT_REVISION + ")"
#endif
                                                          ) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Qt version"), QT_VERSION_STR) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("WebKit version"), QupZilla::WEBKITVERSION) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Build time"), QupZilla::BUILDTIME) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Platform"), qz_buildSystem()));

        cPage.replace("%PATHS-TEXT%",
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Profile"), mApp->getActiveProfilPath()) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Settings"), mApp->getActiveProfilPath() + "settings.ini") +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Saved session"), mApp->getActiveProfilPath() + "session.dat") +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Pinned tabs"), mApp->getActiveProfilPath() + "pinnedtabs.dat") +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Data"), mApp->DATADIR) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Themes"), mApp->THEMESDIR) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Translations"), mApp->TRANSLATIONSDIR));

        QString debugBuild = tr("Disabled");
        QString webGLEnabled = tr("Disabled");
        QString w7APIEnabled = tr("Disabled");
        QString KDEIntegration = tr("Disabled");
        QString portableBuild = tr("Disabled");

#ifdef QUPZILLA_DEBUG_BUILD
        debugBuild = tr("<b>Enabled</b>");
#endif
#ifdef USE_WEBGL
        webGLEnabled = tr("<b>Enabled</b>");
#endif
#if defined(Q_WS_WIN) && defined(W7API)
        w7APIEnabled = tr("<b>Enabled</b>");
#endif
#if defined(Q_WS_X11) && defined(KDE)
        KDEIntegration = tr("<b>Enabled</b>");
#endif
#ifdef PORTABLE_BUILD
        portableBuild = tr("<b>Enabled</b>");
#endif

        cPage.replace("%BUILD-CONFIG-TEXT%",
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Debug build"), debugBuild) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("WebGL support"), webGLEnabled) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Windows 7 API"), w7APIEnabled) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("KDE integration"), KDEIntegration) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Portable build"), portableBuild));
    }

    QString page = cPage;
    page.replace("%USER-AGENT%", mApp->getWindow()->weView()->webPage()->userAgentForUrl(QUrl()));

    QString pluginsString;
    const QList<Plugins::Plugin> &availablePlugins = mApp->plugins()->getAvailablePlugins();

    foreach(const Plugins::Plugin & plugin, availablePlugins) {
        PluginSpec spec = plugin.pluginSpec;
        pluginsString.append(QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>").arg(
                                 spec.name, spec.version, Qt::escape(spec.author), spec.description));
    }

    if (pluginsString.isEmpty()) {
        pluginsString = QString("<tr><td colspan=4 class=\"no-available-plugins\">%1</td></tr>").arg(tr("No available extensions."));
    }

    page.replace("%PLUGINS-INFO%", pluginsString);

    QString allGroupsString;
    QSettings* settings = Settings::globalSettings();
    foreach(const QString & group, settings->childGroups()) {
        QString groupString = QString("<tr><th colspan=\"2\">[%1]</th></tr>").arg(group);
        settings->beginGroup(group);

        foreach(const QString & key, settings->childKeys()) {
            const QVariant &keyValue = settings->value(key);
            QString keyString;

            switch (keyValue.type()) {
            case QVariant::ByteArray:
                keyString = "QByteArray";
                break;

            case QVariant::Point: {
                const QPoint point = keyValue.toPoint();
                keyString = QString("QPoint(%1, %2)").arg(QString::number(point.x()), QString::number(point.y()));
                break;
            }

            case QVariant::StringList:
                keyString = keyValue.toStringList().join(",");
                break;

            default:
                keyString = keyValue.toString();
            }

            if (keyString.isEmpty()) {
                keyString = "\"empty\"";
            }

            groupString.append(QString("<tr><td>%1</td><td>%2</td></tr>").arg(key, Qt::escape(keyString)));
        }

        settings->endGroup();
        allGroupsString.append(groupString);
    }

    page.replace("%PREFS-INFO%", allGroupsString);

    return page;
}
