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
#include "cookiemanager.h"
#include "cookiejar.h"
#include "browsinglibrary.h"
#include "historymodel.h"
#include "networkmanager.h"
#include "rssmanager.h"
#include "updater.h"
#include "autosaver.h"
#include "pluginproxy.h"
#include "bookmarksmodel.h"
#include "downloadmanager.h"
#include "autofillmodel.h"
#include "adblockmanager.h"
#include "desktopnotificationsfactory.h"
#include "iconprovider.h"
#include "qtwin.h"
#include "mainapplication.h"
#include "webhistoryinterface.h"

MainApplication::MainApplication(const QList<CommandLineOptions::ActionPair> &cmdActions, int &argc, char **argv)
    : QtSingleApplication("QupZillaWebBrowser", argc, argv)
    , m_cookiemanager(0)
    , m_browsingLibrary(0)
    , m_historymodel(0)
    , m_websettings(0)
    , m_networkmanager(0)
    , m_cookiejar(0)
    , m_rssmanager(0)
    , m_updater(0)
    , m_plugins(0)
    , m_bookmarksModel(0)
    , m_downloadManager(0)
    , m_autofill(0)
    , m_networkCache(new QNetworkDiskCache)
    , m_desktopNotifications(0)
    , m_iconProvider(new IconProvider)
    , m_isClosing(false)
    , m_isStateChanged(false)
    , m_isExited(false)
    , m_isRestoring(false)
{
    setOverrideCursor(Qt::WaitCursor);
#if defined(Q_WS_X11) & !defined(NO_SYSTEM_DATAPATH)
    DATADIR = "/usr/share/qupzilla/";
#else
    DATADIR = qApp->applicationDirPath()+"/";
#endif
    PLUGINSDIR = DATADIR + "plugins/";
    TRANSLATIONSDIR = DATADIR + "locale/";
    THEMESDIR = DATADIR + "themes/";

    setWindowIcon(QupZilla::qupzillaIcon());
    bool noAddons = false;
    QUrl startUrl("");
    QString message;
    QString startProfile;

    if (argc > 1) {
        foreach (CommandLineOptions::ActionPair pair, cmdActions) {
            switch (pair.action) {
            case CommandLineOptions::StartWithoutAddons:
                noAddons = true;
                break;
            case CommandLineOptions::StartWithProfile:
                startProfile = pair.text;
                break;
            case CommandLineOptions::NewTab:
                message = "ACTION:NewTab";
                break;
            case CommandLineOptions::NewWindow:
                message = "ACTION:NewWindow";
                break;
            case CommandLineOptions::ShowDownloadManager:
                message = "ACTION:ShowDownloadManager";
                break;
            case CommandLineOptions::OpenUrl:
                startUrl = pair.text;
                message = "URL:" + startUrl.toString();
                break;
            default:
                break;
            }
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

    checkSettingsDir();

    QSettings::setDefaultFormat(QSettings::IniFormat);
    if (startProfile.isEmpty()) {
        QSettings settings(homePath+"profiles/profiles.ini", QSettings::IniFormat);
        if (settings.value("Profiles/startProfile","default").toString().contains("/"))
            m_activeProfil=homePath+"profiles/default/";
        else
            m_activeProfil=homePath+"profiles/"+settings.value("Profiles/startProfile","default").toString()+"/";
    } else
        m_activeProfil = homePath+"profiles/"+startProfile+"/";

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
    QWebHistoryInterface::setDefaultInterface(new WebHistoryInterface(this));

    QupZilla* qupzilla = new QupZilla(true, startUrl);
    m_mainWindows.append(qupzilla);
    connect(qupzilla, SIGNAL(message(MainApplication::MessageType,bool)), this, SLOT(sendMessages(MainApplication::MessageType,bool)));
    qupzilla->show();

    AutoSaver* saver = new AutoSaver();
    connect(saver, SIGNAL(saveApp()), this, SLOT(saveStateSlot()));

    if (settings2.value("Web-Browser-Settings/CheckUpdates", true).toBool())
        m_updater = new Updater(qupzilla);

    if (noAddons) {
        settings2.setValue("Plugin-Settings/AllowedPlugins", QStringList());
        settings2.setValue("Plugin-Settings/EnablePlugins", false);
    }

    networkManager()->loadCertExceptions();
    plugins()->loadPlugins();
    loadSettings();

    QTimer::singleShot(2000, this, SLOT(restoreCursor()));
    QTimer::singleShot(10*1000, this, SLOT(setupJumpList()));
}

void MainApplication::loadSettings()
{
    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Themes");
    QString activeTheme = settings.value("activeTheme",
#ifdef Q_WS_X11
                                         "linux"
#else
                                         "windows"
#endif
                                         ).toString();
    settings.endGroup();
    m_activeThemePath = THEMESDIR + activeTheme + "/";
    QFile cssFile(m_activeThemePath + "main.css");
    cssFile.open(QFile::ReadOnly);
    QString css = cssFile.readAll();
    cssFile.close();
#ifdef Q_WS_X11
    if (QFile(m_activeThemePath + "linux.css").exists()) {
        cssFile.setFileName(m_activeThemePath + "linux.css");
        cssFile.open(QFile::ReadOnly);
        css.append(cssFile.readAll());
        cssFile.close();
    }
#endif
#ifdef Q_WS_WIN
    if (QFile(m_activeThemePath + "windows.css").exists()) {
        cssFile.setFileName(m_activeThemePath + "windows.css");
        cssFile.open(QFile::ReadOnly);
        css.append(cssFile.readAll());
        cssFile.close();
    }
#endif

    QString relativePath = QDir::current().relativeFilePath(m_activeThemePath);
    css.replace(QRegExp("url\\s*\\(\\s*([^\\*:\\);]+)\\s*\\)", Qt::CaseSensitive, QRegExp::RegExp2),
                QString("url(%1\\1)").arg(relativePath + "/"));
    setStyleSheet(css);

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
    QUrl userStyleSheet = settings.value("userStyleSheet", QUrl()).toUrl();
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

    m_websettings->setUserStyleSheetUrl(userStyleSheet);
    m_websettings->setDefaultTextEncoding("System");
    m_websettings->setWebGraphic(QWebSettings::DefaultFrameIconGraphic, IconProvider::fromTheme("text-plain").pixmap(16,16));

    if (allowPersistentStorage) m_websettings->enablePersistentStorage(m_activeProfil);
    m_websettings->setMaximumPagesInCache(maxCachedPages);

    setWheelScrollLines(scrollingLines);

    if (m_downloadManager)
        m_downloadManager->loadSettings();
}

void MainApplication::setupJumpList()
{
    QtWin::setupJumpList();
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

void MainApplication::setStateChanged()
{
    m_isStateChanged = true;
    sendMessages(HistoryStateChanged, true);
}

bool MainApplication::isStateChanged()
{
    if (m_isStateChanged) {
        m_isStateChanged = false;
        return true;
    }
    return false;
}

void MainApplication::sendMessages(MainApplication::MessageType mes, bool state)
{
    emit message(mes, state);
}

void MainApplication::receiveAppMessage(QString message)
{
    if (message.startsWith("URL:")) {
        QString url(message.remove("URL:"));
        addNewTab(WebView::guessUrlFromString(url));
    } else if (message.startsWith("ACTION:")) {
        QString text = message.mid(7);
        if (text == "NewTab")
            addNewTab();
        else if (text == "NewWindow")
            makeNewWindow(false);
        else if (text == "ShowDownloadManager")
            downManager()->show();
    }

    QupZilla* actWin = getWindow();
    if (!actWin && !isClosing()) { // It can only occur if download manager window was still open
        makeNewWindow(true);
        return;
    }
    actWin->setWindowState(actWin->windowState() & ~Qt::WindowMinimized);
    actWin->raise();
    actWin->activateWindow();
    actWin->setFocus();
}

void MainApplication::addNewTab(const QUrl &url)
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

    if (file == "" || !QFile::exists(TRANSLATIONSDIR + file) )
        return;

    QTranslator* app = new QTranslator();
    app->load(DATADIR+"locale/"+file);
    QTranslator* sys = new QTranslator();

    if (QFile::exists(TRANSLATIONSDIR + "qt_" + shortLoc + ".qm"))
        sys->load(TRANSLATIONSDIR + "qt_" + shortLoc + ".qm");

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

    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("SessionRestore");
    settings.setValue("isRunning",false);
    settings.setValue("isCrashed", false);
    settings.endGroup();

    bool deleteCookies = settings.value("Web-Browser-Settings/deleteCookiesOnClose",false).toBool();
    bool deleteHistory = settings.value("Web-Browser-Settings/deleteHistoryOnClose",false).toBool();

    if (deleteCookies)
        QFile::remove(m_activeProfil+"cookies.dat");
    if (deleteHistory)
        m_historymodel->clearHistory();

    cookieJar()->saveCookies();
    m_networkmanager->saveCertExceptions();
    m_plugins->c2f_saveSettings();
    AdBlockManager::instance()->save();
    QFile::remove(getActiveProfilPath() + "WebpageIcons.db");

//    qDebug() << "Quitting application...";
    quit();
}

BrowsingLibrary* MainApplication::browsingLibrary()
{
    if (!m_browsingLibrary)
        m_browsingLibrary = new BrowsingLibrary(getWindow());
    return m_browsingLibrary;
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

BookmarksModel* MainApplication::bookmarksModel()
{
    if (!m_bookmarksModel)
        m_bookmarksModel = new BookmarksModel(this);
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

DesktopNotificationsFactory* MainApplication::desktopNotifications()
{
    if (!m_desktopNotifications)
        m_desktopNotifications = new DesktopNotificationsFactory(this);
    return m_desktopNotifications;
}

void MainApplication::aboutToCloseWindow(QupZilla* window)
{
    if (!window)
        return;

    m_mainWindows.removeOne(window);
}

//Version of session.dat file
static const int sessionVersion = 0x0002;

bool MainApplication::saveStateSlot()
{
    if (m_websettings->testAttribute(QWebSettings::PrivateBrowsingEnabled) || m_isRestoring)
        return false;

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
        if (m_mainWindows.at(i)->isFullScreen())
            stream << QByteArray();
        else
            stream << m_mainWindows.at(i)->saveState();
    }
    file.close();

    settings.setValue("restoreSession",true);
    settings.endGroup();

    QupZilla* qupzilla_ = getWindow();
    if (qupzilla_)
        qupzilla_->tabWidget()->savePinnedTabs();

    return true;
}

bool MainApplication::restoreStateSlot(QupZilla* window)
{
    m_isRestoring = true;
    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);
    int afterStart = settings.value("Web-URL-Settings/afterLaunch", 1).toInt();
    settings.beginGroup("SessionRestore");
    if (!settings.value("restoreSession",false).toBool()) {
        m_isRestoring = false;
        return false;
    }
    if (settings.value("isCrashed",false).toBool() && afterStart != 2) {
        QMessageBox::StandardButton button = QMessageBox::warning(window, tr("Last session crashed"),
                                                                  tr("<b>QupZilla crashed :-(</b><br/>Oops, last session of QupZilla ends with its crash. We are very sorry. Would you try to restore saved state?"),
                                                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (button != QMessageBox::Yes) {
            m_isRestoring = false;
            return false;
        }
    }
    if (!QFile::exists(m_activeProfil+"session.dat")) {
        m_isRestoring = false;
        return false;
    }

    settings.setValue("isCrashed",false);
    QFile file(m_activeProfil+"session.dat");
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);

    QByteArray tabState;
    QByteArray qMainWindowState;
    int version;
    int windowCount;

    stream >> version;
    if (version != sessionVersion) {
        m_isRestoring = false;
        return false;
    }
    stream >> windowCount;
    stream >> tabState;
    stream >> qMainWindowState;

    window->tabWidget()->restoreState(tabState);
    window->restoreState(qMainWindowState);

    if (windowCount > 1) {
        for (int i = 1; i < windowCount; i++) {
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
//            window->tabWidget()->closeTab(0);
            window->show();
        }
    }
    file.close();

    m_isRestoring = false;
    return true;
}

void MainApplication::checkProfile(QString path)
{
    QByteArray rData;
    QFile versionFile(path+"version");
    versionFile.open(QFile::ReadOnly);
    rData = versionFile.readAll();
    if (rData.contains(QupZilla::VERSION.toAscii())) {
        versionFile.close();
        return;
    }
    versionFile.close();
#ifdef UNRELEASED_BUILD
    return;
#endif
    //Starting profile migration manager
}

bool MainApplication::checkSettingsDir()
{
    /*
    $HOMEDIR
        |
    .qupzilla/
        |
    profiles/-----------
        |              |
    default/      profiles.ini
        | ---------------
        |               |
    browsedata.db    background.png
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
#ifdef UNRELEASED_BUILD
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

    if (rData.contains("1.0.0-b3")) // Data not changed from this version
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
