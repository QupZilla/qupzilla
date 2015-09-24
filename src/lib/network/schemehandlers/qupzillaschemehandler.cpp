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
#include "qupzillaschemehandler.h"
#include "qztools.h"
#include "browserwindow.h"
#include "mainapplication.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "speeddial.h"
#include "pluginproxy.h"
#include "plugininterface.h"
#include "settings.h"
#include "datapaths.h"
#include "iconprovider.h"

#include <QTextStream>
#include <QTimer>
#include <QSettings>
#include <QWebSecurityOrigin>

#if QT_VERSION < 0x050000
#include "qwebkitversion.h"
#else
#include <QWebPage>
#endif

static QString authorString(const char* name, const QString &mail)
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
    knownPages << "about" << "reportbug" << "start" << "speeddial" << "config" << "restore";

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
    QWebSecurityOrigin::addLocalScheme("qupzilla");
    QTextStream stream(&m_buffer);
    stream.setCodec("UTF-8");

    if (m_pageName == QLatin1String("about")) {
        stream << aboutPage();
    }
    else if (m_pageName == QLatin1String("reportbug")) {
        stream << reportbugPage();
    }
    else if (m_pageName == QLatin1String("start")) {
        stream << startPage();
    }
    else if (m_pageName == QLatin1String("speeddial")) {
        stream << speeddialPage();
    }
    else if (m_pageName == QLatin1String("config")) {
        stream << configPage();
    }
    else if (m_pageName == QLatin1String("restore")) {
        stream << restorePage();
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
    QWebSecurityOrigin::removeLocalScheme("qupzilla");
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

    bPage.append(QzTools::readAllFileContents(":html/reportbug.html"));
    bPage.replace(QLatin1String("%FAVICON%"), QLatin1String("qrc:icons/qupzilla.png"));
    bPage.replace(QLatin1String("%BOX-BORDER%"), QLatin1String("qrc:html/box-border.png"));

    bPage.replace(QLatin1String("%TITLE%"), tr("Report Issue"));
    bPage.replace(QLatin1String("%REPORT-ISSUE%"), tr("Report Issue"));
    bPage.replace(QLatin1String("%PLUGINS-TEXT%"), tr("If you are experiencing problems with QupZilla, please try to disable"
                  " all extensions first. <br/>If this does not fix it, then please fill out this form: "));
    bPage.replace(QLatin1String("%EMAIL%"), tr("Your E-mail"));
    bPage.replace(QLatin1String("%TYPE%"), tr("Issue type"));
    bPage.replace(QLatin1String("%DESCRIPTION%"), tr("Issue description"));
    bPage.replace(QLatin1String("%SEND%"), tr("Send"));
    bPage.replace(QLatin1String("%E-MAIL-OPTIONAL%"), tr("E-mail is optional<br/><b>Note: </b>Please read how to make a "
                  "bug report <a href=%1>here</a> first.").arg("https://github.com/QupZilla/qupzilla/wiki/Bug-Reports target=_blank"));
    bPage.replace(QLatin1String("%FIELDS-ARE-REQUIRED%"), tr("Please fill out all required fields!"));

    bPage.replace(QLatin1String("%INFO_OS%"), QzTools::operatingSystem());
    bPage.replace(QLatin1String("%INFO_APP%"),
#ifdef GIT_REVISION
                  QString("%1 (%2)").arg(Qz::VERSION, GIT_REVISION)
#else
                  Qz::VERSION
#endif
                 );
    bPage.replace(QLatin1String("%INFO_QT%"), QString("%1 (built with %2)").arg(qVersion(), QT_VERSION_STR));
    bPage.replace(QLatin1String("%INFO_WEBKIT%"), qWebKitVersion()),
                  bPage = QzTools::applyDirectionToPage(bPage);

    return bPage;
}

QString QupZillaSchemeReply::startPage()
{
    static QString sPage;

    if (!sPage.isEmpty()) {
        return sPage;
    }

    sPage.append(QzTools::readAllFileContents(":html/start.html"));
    sPage.replace(QLatin1String("%FAVICON%"), QLatin1String("qrc:icons/qupzilla.png"));
    sPage.replace(QLatin1String("%BOX-BORDER%"), QLatin1String("qrc:html/box-border.png"));
    sPage.replace(QLatin1String("%ABOUT-IMG%"), QLatin1String("qrc:icons/other/about.png"));

    sPage.replace(QLatin1String("%TITLE%"), tr("Start Page"));
    sPage.replace(QLatin1String("%BUTTON-LABEL%"), tr("Search on Web"));
    sPage.replace(QLatin1String("%SEARCH-BY%"), tr("Search results provided by DuckDuckGo"));
    sPage.replace(QLatin1String("%WWW%"), Qz::WIKIADDRESS);
    sPage.replace(QLatin1String("%ABOUT-QUPZILLA%"), tr("About QupZilla"));
    sPage.replace(QLatin1String("%PRIVATE-BROWSING%"), mApp->isPrivate() ? tr("<h1>Private Browsing</h1>") : QString());
    sPage = QzTools::applyDirectionToPage(sPage);

    return sPage;
}

QString QupZillaSchemeReply::aboutPage()
{
    static QString aPage;

    if (aPage.isEmpty()) {
        aPage.append(QzTools::readAllFileContents(":html/about.html"));
        aPage.replace(QLatin1String("%FAVICON%"), QLatin1String("qrc:icons/qupzilla.png"));
        aPage.replace(QLatin1String("%BOX-BORDER%"), QLatin1String("qrc:html/box-border.png"));
        aPage.replace(QLatin1String("%ABOUT-IMG%"), QLatin1String("qrc:icons/other/about.png"));
        aPage.replace(QLatin1String("%COPYRIGHT-INCLUDE%"), QzTools::escape(QzTools::readAllFileContents(":html/copyright")));

        aPage.replace(QLatin1String("%TITLE%"), tr("About QupZilla"));
        aPage.replace(QLatin1String("%ABOUT-QUPZILLA%"), tr("About QupZilla"));
        aPage.replace(QLatin1String("%INFORMATIONS-ABOUT-VERSION%"), tr("Information about version"));
        aPage.replace(QLatin1String("%COPYRIGHT%"), tr("Copyright"));

        aPage.replace(QLatin1String("%VERSION-INFO%"),
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Version"),
#ifdef GIT_REVISION
                              QString("%1 (%2)").arg(Qz::VERSION, GIT_REVISION)
#else
                              Qz::VERSION
#endif
                                                          ) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("WebKit version"), qWebKitVersion()));
        aPage.replace(QLatin1String("%MAIN-DEVELOPER%"), tr("Main developer"));
        aPage.replace(QLatin1String("%MAIN-DEVELOPER-TEXT%"), authorString(Qz::AUTHOR, "nowrep@gmail.com"));
        aPage.replace(QLatin1String("%CONTRIBUTORS%"), tr("Contributors"));
        aPage.replace(QLatin1String("%CONTRIBUTORS-TEXT%"),
                      authorString("Mladen Pejaković", "pejakm@autistici.org") + "<br/>" +
                      authorString("Alexander Samilov", "alexsamilovskih@gmail.com") + "<br/>" +
                      authorString("Seyyed Razi Alavizadeh", "s.r.alavizadeh@gmail.com") + "<br/>" +
                      authorString("Franz Fellner", "alpine.art.de@googlemail.com") + "<br/>" +
                      authorString("Bryan M Dunsmore", "dunsmoreb@gmail.com") + "<br/>" +
                      authorString("Mariusz Fik", "fisiu@opensuse.org") + "<br/>" +
                      authorString("Jan Rajnoha", "honza.rajny@hotmail.com")  + "<br/>" +
                      authorString("Daniele Cocca", "jmc@chakra-project.org")
                     );
        aPage.replace(QLatin1String("%TRANSLATORS%"), tr("Translators"));
        aPage.replace(QLatin1String("%TRANSLATORS-TEXT%"),
                      authorString("Heimen Stoffels", "vistausss@gmail.com") + " (Dutch)<br/>" +
                      authorString("Peter Vacula", "pvacula1989@gmail.com") + " (Slovak)<br/>" +
                      authorString("Ján Ďanovský", "dagsoftware@yahoo.com") + " (Slovak)<br/>" +
                      authorString("Jonathan Hooverman", "jonathan.hooverman@gmail.com") + " (German)<br/>" +
                      authorString("Federico Fabiani", "federico.fabiani85@gmail.com") + " (Italian)<br/>" +
                      authorString("Francesco Marinucci", "framarinucci@gmail.com") + " (Italian)<br/>" +
                      authorString("Jorge Sevilla", "jsevi@ozu.es") + " (Spanish)<br/>" +
                      authorString("Ștefan Comănescu", "sdfanq@gmail.com") + " (Romanian)<br/>" +
                      authorString("Michał Szymanowski", "tylkobuba@gmail.com") + " (Polish)<br/>" +
                      authorString("Mariusz Fik", "fisiu@opensuse.org") + " (Polish)<br/>" +
                      authorString("Jérôme Giry", "baikalink@hotmail.fr") + " (French)<br/>" +
                      authorString("Nicolas Ourceau", "lamessen@hotmail.fr") + " (French)<br/>" +
                      authorString("Vasilis Tsivikis", "vasitsiv.dev@gmail.com") + " (Greek)<br/>" +
                      authorString("Rustam Salakhutdinov", "salahutd@gmail.com") + " (Russian)<br/>" +
                      authorString("Oleg Brezhnev", "oleg-423@yandex.ru") + " (Russian)<br/>" +
                      authorString("Sérgio Marques", "smarquespt@gmail.com") + " (Portuguese)<br/>" +
                      authorString("Alexandre Carvalho", "alexandre05@live.com") + " (Brazilian Portuguese)<br/>" +
                      authorString("Mladen Pejaković", "pejakm@autistici.org") + " (Serbian)<br/>" +
                      authorString("Unink-Lio", "unink4451@163.com") + " (Chinese)<br/>" +
                      authorString("Yu Hai", "yohanprc@eml.cc") + " (Chinese)<br/>" +
                      authorString("Wu Cheng-Hong", "stu2731652@gmail.com") + " (Traditional Chinese)<br/>" +
                      authorString("Widya Walesa", "walecha99@gmail.com") + " (Indonesian)<br/>" +
                      authorString("Beqa Arabuli", "arabulibeqa@gmail.com") + " (Georgian)<br/>" +
                      authorString("Daiki Noda", "sys.pdr.pdm9@gmail.com") + " (Japanese)<br/>" +
                      authorString("Gábor Oberle", "oberleg@myopera.com") + " (Hungarian)<br/>" +
                      authorString("Piccoro McKay Lenz", "mckaygerhard@gmail.com") + " (Venezulean Spanish)<br/>" +
                      authorString("Stanislav Kuznietsov", "stanislav_kuznetsov@ukr.net") + " (Ukrainian)<br/>" +
                      authorString("Seyyed Razi Alavizadeh", "s.r.alavizadeh@gmail.com") + " (Persian)<br/>" +
                      authorString("Guillem Prats", "guprej@gmail.com") + " (Catalan)<br/>" +
                      authorString("Clara Villalba", "cvilmon@gmail.com") + " (Catalan)<br/>" +
                      authorString("Muhammad Fawwaz Orabi", "mfawwaz93@gmail.com") + " (Arabic)<br/>" +
                      authorString("Lasso Kante", "kantemou@gmail.com") + " (N'ko)<br/>" +
                      authorString("Kizito Birabwa", "kbirabwa@yahoo.co.uk") + " (Luganda)<br/>" +
                      authorString("Juan Carlos Sánchez", "hollow1984angel@gmail.com") + " (Mexican Spanish)<br/>" +
                      authorString("Xabier Aramendi", "azpidatziak@gmail.com") + " (Basque)<br/>" +
                      authorString("Ferhat AYDIN", "ferhataydin44@gmail.com") + " (Turkish)"
                     );
        aPage = QzTools::applyDirectionToPage(aPage);
    }

    return aPage;
}

QString QupZillaSchemeReply::speeddialPage()
{
    static QString dPage;

    if (dPage.isEmpty()) {
        dPage.append(QzTools::readAllFileContents(":html/speeddial.html"));
        dPage.replace(QLatin1String("%FAVICON%"), QLatin1String("qrc:icons/qupzilla.png"));
        dPage.replace(QLatin1String("%IMG_PLUS%"), QLatin1String("qrc:html/plus.png"));
        dPage.replace(QLatin1String("%IMG_CLOSE%"), QLatin1String("qrc:html/close.png"));
        dPage.replace(QLatin1String("%IMG_EDIT%"), QLatin1String("qrc:html/edit.png"));
        dPage.replace(QLatin1String("%IMG_RELOAD%"), QLatin1String("qrc:html/reload.png"));
        dPage.replace(QLatin1String("%JQUERY%"), QLatin1String("qrc:html/jquery.js"));
        dPage.replace(QLatin1String("%JQUERY-UI%"), QLatin1String("qrc:html/jquery-ui.js"));
        dPage.replace(QLatin1String("%LOADING-IMG%"), QLatin1String("qrc:html/loading.gif"));
        dPage.replace(QLatin1String("%IMG_SETTINGS%"), QLatin1String("qrc:html/configure.png"));

        dPage.replace(QLatin1String("%SITE-TITLE%"), tr("Speed Dial"));
        dPage.replace(QLatin1String("%ADD-TITLE%"), tr("Add New Page"));
        dPage.replace(QLatin1String("%TITLE-EDIT%"), tr("Edit"));
        dPage.replace(QLatin1String("%TITLE-REMOVE%"), tr("Remove"));
        dPage.replace(QLatin1String("%TITLE-RELOAD%"), tr("Reload"));
        dPage.replace(QLatin1String("%TITLE-WARN%"), tr("Are you sure to remove this speed dial?"));
        dPage.replace(QLatin1String("%TITLE-WARN-REL%"), tr("Are you sure to reload all speed dials?"));
        dPage.replace(QLatin1String("%TITLE-FETCHTITLE%"), tr("Load title from page"));
        dPage.replace(QLatin1String("%URL%"), tr("Url"));
        dPage.replace(QLatin1String("%TITLE%"), tr("Title"));
        dPage.replace(QLatin1String("%APPLY%"), tr("Apply"));
        dPage.replace(QLatin1String("%CLOSE%"), tr("Close"));
        dPage.replace(QLatin1String("%NEW-PAGE%"), tr("New Page"));
        dPage.replace(QLatin1String("%SETTINGS-TITLE%"), tr("Speed Dial settings"));
        dPage.replace(QLatin1String("%TXT_PLACEMENT%"), tr("Placement: "));
        dPage.replace(QLatin1String("%TXT_AUTO%"), tr("Auto"));
        dPage.replace(QLatin1String("%TXT_COVER%"), tr("Cover"));
        dPage.replace(QLatin1String("%TXT_FIT%"), tr("Fit"));
        dPage.replace(QLatin1String("%TXT_FWIDTH%"), tr("Fit Width"));
        dPage.replace(QLatin1String("%TXT_FHEIGHT%"), tr("Fit Height"));
        dPage.replace(QLatin1String("%TXT_NOTE%"), tr("Use background image"));
        dPage.replace(QLatin1String("%TXT_SELECTIMAGE%"), tr("Select image"));
        dPage.replace(QLatin1String("%TXT_NRROWS%"), tr("Maximum pages in a row:"));
        dPage.replace(QLatin1String("%TXT_SDSIZE%"), tr("Change size of pages:"));
        dPage.replace(QLatin1String("%TXT_CNTRDLS%"), tr("Center speed dials"));
        dPage = QzTools::applyDirectionToPage(dPage);
    }

    QString page = dPage;
    SpeedDial* dial = mApp->plugins()->speedDial();

    page.replace(QLatin1String("%INITIAL-SCRIPT%"), dial->initialScript());
    page.replace(QLatin1String("%IMG_BACKGROUND%"), dial->backgroundImage());
    page.replace(QLatin1String("%B_SIZE%"), dial->backgroundImageSize());
    page.replace(QLatin1String("%ROW-PAGES%"), QString::number(dial->pagesInRow()));
    page.replace(QLatin1String("%SD-SIZE%"), QString::number(dial->sdSize()));
    page.replace(QLatin1String("%SD-CENTER%"), dial->sdCenter() ? QSL("true") : QSL("false"));

    return page;
}

QString QupZillaSchemeReply::restorePage()
{
    static QString rPage;

    if (rPage.isEmpty()) {
        rPage.append(QzTools::readAllFileContents(":html/restore.html"));
        rPage.replace(QLatin1String("%FAVICON%"), QLatin1String("qrc:icons/qupzilla.png"));
        rPage.replace(QLatin1String("%BOX-BORDER%"), QLatin1String("qrc:html/box-border.png"));
        rPage.replace(QLatin1String("%IMAGE%"), QzTools::pixmapToByteArray(IconProvider::standardIcon(QStyle::SP_MessageBoxWarning).pixmap(45, 45)));
        rPage.replace(QLatin1String("%TITLE%"), tr("Restore Session"));
        rPage.replace(QLatin1String("%OOPS%"), tr("Oops, QupZilla crashed."));
        rPage.replace(QLatin1String("%APOLOGIZE%"), tr("We apologize for this. Would you like to restore the last saved state?"));
        rPage.replace(QLatin1String("%TRY-REMOVING%"), tr("Try removing one or more tabs that you think cause troubles"));
        rPage.replace(QLatin1String("%START-NEW%"), tr("Or you can start completely new session"));
        rPage = QzTools::applyDirectionToPage(rPage);
    }

    return rPage;
}

QString QupZillaSchemeReply::configPage()
{
    static QString cPage;

    if (cPage.isEmpty()) {
        cPage.append(QzTools::readAllFileContents(":html/config.html"));
        cPage.replace(QLatin1String("%FAVICON%"), QLatin1String("qrc:icons/qupzilla.png"));
        cPage.replace(QLatin1String("%BOX-BORDER%"), QLatin1String("qrc:html/box-border.png"));
        cPage.replace(QLatin1String("%ABOUT-IMG%"), QLatin1String("qrc:icons/other/about.png"));

        cPage.replace(QLatin1String("%TITLE%"), tr("Configuration Information"));
        cPage.replace(QLatin1String("%CONFIG%"), tr("Configuration Information"));
        cPage.replace(QLatin1String("%INFORMATIONS-ABOUT-VERSION%"), tr("Information about version"));
        cPage.replace(QLatin1String("%CONFIG-ABOUT%"), tr("This page contains information about QupZilla's current configuration - relevant for troubleshooting. Please include this information when submitting bug reports."));
        cPage.replace(QLatin1String("%BROWSER-IDENTIFICATION%"), tr("Browser Identification"));
        cPage.replace(QLatin1String("%PATHS%"), tr("Paths"));
        cPage.replace(QLatin1String("%BUILD-CONFIG%"), tr("Build Configuration"));
        cPage.replace(QLatin1String("%PREFS%"), tr("Preferences"));
        cPage.replace(QLatin1String("%OPTION%"), tr("Option"));
        cPage.replace(QLatin1String("%VALUE%"), tr("Value"));
        cPage.replace(QLatin1String("%PLUGINS%"), tr("Extensions"));
        cPage.replace(QLatin1String("%PL-NAME%"), tr("Name"));
        cPage.replace(QLatin1String("%PL-VER%"), tr("Version"));
        cPage.replace(QLatin1String("%PL-AUTH%"), tr("Author"));
        cPage.replace(QLatin1String("%PL-DESC%"), tr("Description"));

        cPage.replace(QLatin1String("%VERSION-INFO%"),
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Application version"),
#ifdef GIT_REVISION
                              QString("%1 (%2)").arg(Qz::VERSION, GIT_REVISION)
#else
                              Qz::VERSION
#endif
                                                          ) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Qt version"), QT_VERSION_STR) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("WebKit version"), qWebKitVersion()) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Build time"), Qz::BUILDTIME) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Platform"), QzTools::operatingSystem()));

        cPage.replace(QLatin1String("%PATHS-TEXT%"),
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Profile"), DataPaths::currentProfilePath()) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Settings"), DataPaths::currentProfilePath() + "/settings.ini") +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Saved session"), DataPaths::currentProfilePath() + "/session.dat") +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Pinned tabs"), DataPaths::currentProfilePath() + "/pinnedtabs.dat") +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Data"), DataPaths::path(DataPaths::AppData)) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Themes"), DataPaths::path(DataPaths::Themes)) +
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Translations"), DataPaths::path(DataPaths::Translations)));

#ifdef QUPZILLA_DEBUG_BUILD
        QString debugBuild = tr("<b>Enabled</b>");
#else
        QString debugBuild = tr("Disabled");
#endif

#ifdef Q_OS_WIN
#if defined(Q_OS_WIN) && defined(W7API)
        QString w7APIEnabled = tr("<b>Enabled</b>");
#else
        QString w7APIEnabled = tr("Disabled");
#endif
#endif

        QString portableBuild = mApp->isPortable() ? tr("<b>Enabled</b>") : tr("Disabled");

        cPage.replace(QLatin1String("%BUILD-CONFIG-TEXT%"),
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Debug build"), debugBuild) +
#ifdef Q_OS_WIN
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Windows 7 API"), w7APIEnabled) +
#endif
                      QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Portable build"), portableBuild));

        cPage = QzTools::applyDirectionToPage(cPage);
    }

    QString page = cPage;
    page.replace(QLatin1String("%USER-AGENT%"), mApp->getWindow()->weView()->page()->userAgentForUrl(QUrl()));

    QString pluginsString;
    const QList<Plugins::Plugin> &availablePlugins = mApp->plugins()->getAvailablePlugins();

    foreach (const Plugins::Plugin &plugin, availablePlugins) {
        PluginSpec spec = plugin.pluginSpec;
        pluginsString.append(QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>").arg(
                                 spec.name, spec.version, QzTools::escape(spec.author), spec.description));
    }

    if (pluginsString.isEmpty()) {
        pluginsString = QString("<tr><td colspan=4 class=\"no-available-plugins\">%1</td></tr>").arg(tr("No available extensions."));
    }

    page.replace(QLatin1String("%PLUGINS-INFO%"), pluginsString);

    QString allGroupsString;
    QSettings* settings = Settings::globalSettings();
    foreach (const QString &group, settings->childGroups()) {
        QString groupString = QString("<tr><th colspan=\"2\">[%1]</th></tr>").arg(group);
        settings->beginGroup(group);

        foreach (const QString &key, settings->childKeys()) {
            const QVariant keyValue = settings->value(key);
            QString keyString;

            switch (keyValue.type()) {
            case QVariant::ByteArray:
                keyString = QLatin1String("QByteArray");
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
                keyString = QLatin1String("\"empty\"");
            }

            groupString.append(QString("<tr><td>%1</td><td>%2</td></tr>").arg(key, QzTools::escape(keyString)));
        }

        settings->endGroup();
        allGroupsString.append(groupString);
    }

    page.replace(QLatin1String("%PREFS-INFO%"), allGroupsString);

    return page;
}
