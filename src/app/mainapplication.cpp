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
#include "mainapplication.h"
#include "qupzilla.h"
#include "tabwidget.h"
#include "bookmarkstoolbar.h"
#include "bookmarksmanager.h"
#include "cookiemanager.h"
#include "cookiejar.h"
#include "historymanager.h"
#include "historymodel.h"
#include "networkmanager.h"
#include "rssmanager.h"
#include "updater.h"
#include "autosaver.h"
#include "commandlineoptions.h"
#include "pluginproxy.h"
#include "bookmarksmodel.h"
#include "downloadmanager.h"
#include "autofillmodel.h"

MainApplication::MainApplication(int &argc, char **argv)
    : QtSingleApplication("QupZillaWebBrowser", argc, argv)
    ,m_bookmarksmanager(0)
    ,m_cookiemanager(0)
    ,m_historymanager(0)
    ,m_historymodel(0)
    ,m_websettings(0)
    ,m_networkmanager(0)
    ,m_cookiejar(0)
    ,m_rssmanager(0)
    ,m_plugins(0)
    ,m_bookmarksModel(0)
    ,m_downloadManager(0)
    ,m_autofill(0)
    ,m_isClosing(false)
    ,m_isChanged(false)
    ,m_isExited(false)
{
#if defined(Q_WS_X11) & !defined(DEVELOPING)
    DATADIR = "/usr/share/qupzilla/";
#else
    DATADIR = qApp->applicationDirPath()+"/";
#endif
    setOverrideCursor(Qt::WaitCursor);
    setWindowIcon(QIcon(":/icons/qupzilla.png"));
    bool noAddons = false;
    QUrl startUrl("");
    QString message;
    if (argc > 1) {
        CommandLineOptions cmd(argc, argv);
        switch (cmd.getAction()) {
        case CommandLineOptions::StartWithoutAddons:
            noAddons = true;
            break;
        case CommandLineOptions::OpenUrl:
            startUrl = QUrl(cmd.getActionString());
            message = "URL:"+startUrl.toString();
            break;
        default:
            m_isExited = true;
            return;
            break;
        }
    }

    if (isRunning()) {
        sendMessage(message);
        m_isExited = true;
        return;
    }

    connect(this, SIGNAL(messageReceived(QString)), this, SLOT(receiveAppMessage(QString)));

    setQuitOnLastWindowClosed(true);
    setApplicationName("QupZilla");
    setApplicationVersion(QupZilla::VERSION);
    setOrganizationDomain("qupzilla");

    QString homePath = QDir::homePath();
    homePath+="/.qupzilla/";

    checkProfileDir();

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings settings(homePath+"profiles/profiles.ini", QSettings::IniFormat);
    if (settings.value("Profiles/startProfile","default").toString().contains("/"))
        m_activeProfil=homePath+"profiles/default/";
    else
        m_activeProfil=homePath+"profiles/"+settings.value("Profiles/startProfile","default").toString()+"/";
    if (!QDir(m_activeProfil).exists())
        m_activeProfil=homePath+"profiles/default/";

    QSettings settings2(m_activeProfil+"settings.ini", QSettings::IniFormat);
    settings2.beginGroup("SessionRestore");
    if (settings2.value("isRunning",false).toBool() )
        settings2.setValue("isCrashed", true);
    settings2.setValue("isRunning", true);
    settings2.endGroup();

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_activeProfil);

    translateApp();
    connectDatabase();

    QupZilla* qupzilla = new QupZilla(true, startUrl);
    m_mainWindows.append(qupzilla);
    connect(qupzilla, SIGNAL(message(MainApplication::MessageType,bool)), this, SLOT(sendMessages(MainApplication::MessageType,bool)));
    qupzilla->show();

    AutoSaver* saver = new AutoSaver();
    connect(saver, SIGNAL(saveApp()), this, SLOT(saveStateSlot()));
    m_updater = new Updater(qupzilla);

    if (noAddons) {
        settings2.setValue("Plugin-Settings/AllowedPlugins",QStringList());
        settings2.setValue("Plugin-Settings/EnablePlugins",false);
    }

    networkManager()->loadCertExceptions();
    plugins()->loadPlugins();
    loadSettings();
}

void MainApplication::loadSettings()
{
    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);
    webSettings();
    //Web browsing settings
    settings.beginGroup("Web-Browser-Settings");
    bool allowFlash = settings.value("allowFlash",true).toBool();
    bool allowJavaScript = settings.value("allowJavaScript",true).toBool();
    bool allowJavaScriptOpenWindow = settings.value("allowJavaScriptOpenWindow",false).toBool();
    bool allowJava = settings.value("allowJava",true).toBool();
    bool allowPersistentStorage = settings.value("allowPersistentStorage",true).toBool();
    bool allowImages = settings.value("autoLoadImages",true).toBool();
    bool dnsPrefetch = settings.value("DNS-Prefetch", false).toBool();
    bool jsClipboard = settings.value("JavaScriptCanAccessClipboard", true).toBool();
    bool linkInFocuschain = settings.value("IncludeLinkInFocusChain", false).toBool();
    bool zoomTextOnly = settings.value("zoomTextOnly", false).toBool();
    bool printElBg = settings.value("PrintElementBackground", true).toBool();
    int maxCachedPages = settings.value("maximumCachedPages",3).toInt();
    int scrollingLines = settings.value("wheelScrollLines", wheelScrollLines()).toInt();
    settings.endGroup();

    m_websettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    m_websettings->setAttribute(QWebSettings::PluginsEnabled, allowFlash);
    m_websettings->setAttribute(QWebSettings::JavascriptEnabled, allowJavaScript);
    m_websettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, allowJavaScriptOpenWindow);
    m_websettings->setAttribute(QWebSettings::JavaEnabled, allowJava);
    m_websettings->setAttribute(QWebSettings::AutoLoadImages, allowImages);
    m_websettings->setAttribute(QWebSettings::DnsPrefetchEnabled, dnsPrefetch);
    m_websettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, jsClipboard);
    m_websettings->setAttribute(QWebSettings::LinksIncludedInFocusChain, linkInFocuschain);
    m_websettings->setAttribute(QWebSettings::ZoomTextOnly, zoomTextOnly);
    m_websettings->setAttribute(QWebSettings::PrintElementBackgrounds, printElBg);

    m_websettings->setFontFamily(QWebSettings::StandardFont, settings.value("StandardFont", m_websettings->fontFamily(QWebSettings::StandardFont)).toString());
    m_websettings->setFontFamily(QWebSettings::CursiveFont, settings.value("CursiveFont", m_websettings->fontFamily(QWebSettings::CursiveFont)).toString());
    m_websettings->setFontFamily(QWebSettings::FantasyFont, settings.value("FantasyFont", m_websettings->fontFamily(QWebSettings::FantasyFont)).toString());
    m_websettings->setFontFamily(QWebSettings::FixedFont, settings.value("FixedFont", m_websettings->fontFamily(QWebSettings::FixedFont)).toString());
    m_websettings->setFontFamily(QWebSettings::SansSerifFont, settings.value("SansSerifFont", m_websettings->fontFamily(QWebSettings::SansSerifFont)).toString());
    m_websettings->setFontFamily(QWebSettings::SerifFont, settings.value("SerifFont", m_websettings->fontFamily(QWebSettings::SerifFont)).toString());
    m_websettings->setFontSize(QWebSettings::DefaultFontSize, settings.value("DefaultFontSize", m_websettings->fontSize(QWebSettings::DefaultFontSize)).toInt() );
    m_websettings->setFontSize(QWebSettings::DefaultFixedFontSize, settings.value("FixedFontSize", m_websettings->fontSize(QWebSettings::DefaultFixedFontSize)).toInt() );

    if (allowPersistentStorage) m_websettings->enablePersistentStorage(m_activeProfil);
    m_websettings->setMaximumPagesInCache(maxCachedPages);

    setWheelScrollLines(scrollingLines);
}

QupZilla* MainApplication::getWindow()
{
    for(int i=0; i<m_mainWindows.count(); i++) {
        if (!m_mainWindows.at(i))
            continue;
        return m_mainWindows.at(i);
    }
    return 0;
}

bool MainApplication::isChanged()
{
    if (m_isChanged) {
        m_isChanged = false;
        return true;
    }
    return false;
}

void MainApplication::sendMessages(MainApplication::MessageType mes, bool state)
{
    qDebug() << mes << state;
    emit message(mes, state);
}

void MainApplication::receiveAppMessage(QString message)
{
    if (message.startsWith("URL:")) {
        QString url(message.remove("URL:"));
        addNewTab(WebView::guessUrlFromString(url));
    }

    QupZilla* actWin = getWindow();
    if (!actWin) { // It can only occur if download manager window was still open
        makeNewWindow(true);
        return;
    }
    actWin->setWindowState(actWin->windowState() & ~Qt::WindowMinimized);
    actWin->raise();
    actWin->activateWindow();
    actWin->setFocus();
}

void MainApplication::addNewTab(QUrl url)
{
    if (!getWindow())
        return;
    getWindow()->tabWidget()->addView(url);
}

void MainApplication::makeNewWindow(bool tryRestore, const QUrl &startUrl)
{
    QupZilla* newWindow = new QupZilla(tryRestore, startUrl);
    connect(newWindow, SIGNAL(message(MainApplication::MessageType,bool)), this, SLOT(sendMessages(MainApplication::MessageType,bool)));
    m_mainWindows.append(newWindow);
    newWindow->show();
}

void MainApplication::connectDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(m_activeProfil+"browsedata.db");
    if (!QFile::exists(m_activeProfil+"browsedata.db")) {
        QFile(DATADIR+"data/default/profiles/default/browsedata.db").copy(m_activeProfil+"browsedata.db");
        qWarning("Cannot find SQLite database file! Copying and using the defaults!");
    }
    if (!db.open())
        qWarning("Cannot open SQLite database! Continuing without database....");

}

void MainApplication::translateApp()
{
    QLocale locale;
    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Browser-View-Settings");
    QString file = settings.value("language",locale.name()+".qm").toString();
    QString shortLoc = file.left(2);

    if (file == "" || !QFile::exists(DATADIR+"locale/"+file) )
        return;

    QTranslator* app = new QTranslator();
    app->load(DATADIR+"locale/"+file);
    QTranslator* sys = new QTranslator();

    if (QFile::exists(DATADIR+"locale/qt_"+shortLoc+".qm"))
        sys->load(DATADIR+"locale/qt_"+shortLoc+".qm");

    m_activeLanguage = file;

    installTranslator(app);
    installTranslator(sys);
}

void MainApplication::quitApplication()
{
    if (m_downloadManager && !m_downloadManager->canClose()) {
        m_downloadManager->show();
        return;
    }

    m_isClosing = true;

    if (m_mainWindows.count() > 0)
        saveStateSlot();

    qDebug() << __FUNCTION__ << "called";
    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("SessionRestore");
    settings.setValue("isRunning",false);
    settings.setValue("isCrashed", false);
    settings.endGroup();

    bool deleteCookies = settings.value("Web-Browser-Settings/deleteCookiesOnClose",false).toBool();
    bool deleteHistory = settings.value("Web-Browser-Settings/deleteHistoryOnClose",false).toBool();

    if (deleteCookies)
        QFile::remove(m_activeProfil+"cookies.dat");
    if (deleteHistory) {
        QSqlQuery query;
        query.exec("DELETE FROM history");
        query.exec("VACUUM");
    }

    cookieJar()->saveCookies();
    m_networkmanager->saveCertExceptions();
    m_plugins->c2f_saveSettings();

    quit();
}

BookmarksManager* MainApplication::bookmarksManager()
{
    if (!m_bookmarksmanager)
        m_bookmarksmanager = new BookmarksManager(getWindow());
    return m_bookmarksmanager;
}

PluginProxy* MainApplication::plugins()
{
    if (!m_plugins)
        m_plugins = new PluginProxy();
    return m_plugins;
}

CookieManager* MainApplication::cookieManager()
{
    if (!m_cookiemanager)
        m_cookiemanager = new CookieManager();
    return m_cookiemanager;
}

HistoryManager* MainApplication::historyManager()
{
    if (!m_historymanager)
        m_historymanager = new HistoryManager(getWindow());
    return m_historymanager;
}

HistoryModel* MainApplication::history()
{
    if (!m_historymodel)
        m_historymodel = new HistoryModel(getWindow());
    return m_historymodel;
}

QWebSettings* MainApplication::webSettings()
{
    if (!m_websettings)
        m_websettings = QWebSettings::globalSettings();
    return m_websettings;
}

NetworkManager* MainApplication::networkManager()
{
    if (!m_networkmanager)
        m_networkmanager = new NetworkManager(getWindow());
    return m_networkmanager;
}

CookieJar* MainApplication::cookieJar()
{
    if (!m_cookiejar) {
        m_cookiejar = new CookieJar(getWindow());
        m_cookiejar->restoreCookies();
    }
    return m_cookiejar;
}

RSSManager* MainApplication::rssManager()
{
    if (!m_rssmanager)
        m_rssmanager = new RSSManager(getWindow());
    return m_rssmanager;
}

BookmarksModel* MainApplication::bookmarks()
{
    if (!m_bookmarksModel)
        m_bookmarksModel = new BookmarksModel();
    return m_bookmarksModel;
}

DownloadManager* MainApplication::downManager()
{
    if (!m_downloadManager)
        m_downloadManager = new DownloadManager();
    return m_downloadManager;
}

AutoFillModel* MainApplication::autoFill()
{
    if (!m_autofill)
        m_autofill = new AutoFillModel(getWindow());
    return m_autofill;
}

void MainApplication::aboutToCloseWindow(QupZilla* window)
{
    if (!window)
        return;

    m_mainWindows.removeOne(window);
    if (m_mainWindows.count() == 0 )
        quitApplication();
}

//Version of session.dat file
static const int sessionVersion = 0x0002;

bool MainApplication::saveStateSlot()
{
    if (m_websettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return false;

    qDebug() << "Saving state";

    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("SessionRestore");
    settings.setValue("restoreSession",false);

    QFile file(m_activeProfil+"session.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);

    stream << sessionVersion;
    stream << m_mainWindows.count();
    for (int i = 0; i < m_mainWindows.count(); i++) {
        stream << m_mainWindows.at(i)->tabWidget()->saveState();
        stream << m_mainWindows.at(i)->saveState();
    }
    file.close();

    settings.setValue("restoreSession",true);
    settings.endGroup();

    QupZilla* qupzilla_ = getWindow();
    if (qupzilla_) {
        settings.setValue("Browser-View-Settings/showBookmarksToolbar",qupzilla_->bookmarksToolbar()->isVisible());
        settings.setValue("Browser-View-Settings/showNavigationToolbar",qupzilla_->navigationToolbar()->isVisible());
        settings.setValue("Browser-View-Settings/showStatusbar",qupzilla_->statusBar()->isVisible());
        settings.setValue("Browser-View-Settings/showMenubar",qupzilla_->menuBar()->isVisible());
    }
    return true;
}

bool MainApplication::restoreStateSlot(QupZilla* window)
{
    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("SessionRestore");
    if (!settings.value("restoreSession",false).toBool())
        return false;
    if (settings.value("isCrashed",false).toBool()) {
        QMessageBox::StandardButton button = QMessageBox::warning(window, tr("Last session crashed"),
                                                                  tr("<b>QupZilla crashed :-(</b><br/>Oops, last session of QupZilla ends with its crash. We are very sorry. Would you try to restore saved state?"),
                                                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (button != QMessageBox::Yes)
            return false;
    }
    if (!QFile::exists(m_activeProfil+"session.dat"))
        return false;

    settings.setValue("isCrashed",false);
    QFile file(m_activeProfil+"session.dat");
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);

    QByteArray tabState;
    QByteArray qMainWindowState;
    int version;
    int windowCount;

    stream >> version;
    if (version != sessionVersion)
        return false;
    stream >> windowCount;
    stream >> tabState;
    stream >> qMainWindowState;

    file.close();

    window->tabWidget()->restoreState(tabState);
    window->restoreState(qMainWindowState);

    settings.endGroup();

    if (windowCount > 1) {
        for (int i = 0; i<(windowCount-1); i++) {
            stream >> tabState;
            stream >> qMainWindowState;

            QupZilla* window = new QupZilla(false);
            m_mainWindows.append(window);
            connect(window, SIGNAL(message(MainApplication::MessageType,bool)), this, SLOT(sendMessages(MainApplication::MessageType,bool)));
            QEventLoop eLoop;
            connect(window, SIGNAL(startingCompleted()), &eLoop, SLOT(quit()));
            eLoop.exec();

            window->tabWidget()->restoreState(tabState);
            window->restoreState(qMainWindowState);
            window->tabWidget()->closeTab(0);
            window->show();
        }
    }

    return true;
}

bool MainApplication::checkProfileDir()
{
    /*
    $HOMEDIR
        |
    .qupzilla/
        |
    profiles/-----------
        |              |
    default/      profiles.ini
        |
    browsedata.db
    */
    QString homePath = QDir::homePath();
    homePath+="/.qupzilla/";

    QByteArray rData;
    if (QDir(homePath).exists()) {
        QFile versionFile(homePath+"version");
        versionFile.open(QFile::ReadOnly);
        rData = versionFile.readAll();
        if (rData.contains(QupZilla::VERSION.toAscii())) {
            versionFile.close();
            return true;
        }
    versionFile.close();
#ifdef DEVELOPING
        return true;
#endif
    }

    std::cout << "Creating new profile directory" << std::endl;

    QDir dir = QDir::home();
    dir.mkdir(".qupzilla");
    dir.cd(".qupzilla");

    //.qupzilla
    QFile(homePath+"version").remove();
    QFile versionFile(homePath+"version");
    versionFile.open(QFile::WriteOnly);
    versionFile.write(QupZilla::VERSION.toAscii());
    versionFile.close();

    if (rData.contains("0.9.6")) // Data not changed from this version
        return true;

    dir.mkdir("profiles");
    dir.cd("profiles");

    //.qupzilla/profiles
    QFile(homePath+"profiles/profiles.ini").remove();
    QFile(DATADIR+"data/default/profiles/profiles.ini").copy(homePath+"profiles/profiles.ini");

    dir.mkdir("default");
    dir.cd("default");

    //.qupzilla/profiles/default
    QFile(homePath+"profiles/default/browsedata.db").remove();
    QFile(DATADIR+"data/default/profiles/default/browsedata.db").copy(homePath+"profiles/default/browsedata.db");
    QFile(homePath+"profiles/default/background.png").remove();
    QFile(DATADIR+"data/default/profiles/default/background.png").copy(homePath+"profiles/default/background.png");

    return dir.isReadable();
}
