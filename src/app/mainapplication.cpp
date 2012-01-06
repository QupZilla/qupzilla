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
#include "globalfunctions.h"
#include "profileupdater.h"
#include "searchenginesmanager.h"
#include "databasewriter.h"
#include "speeddial.h"
#include "webpage.h"

#ifdef Q_WS_WIN
#define DEFAULT_CHECK_UPDATES true
#define DEFAULT_THEME_NAME "windows"
#else
#define DEFAULT_CHECK_UPDATES false
#define DEFAULT_THEME_NAME "linux"
#endif

#if defined(PORTABLE_BUILD) && !defined(NO_SYSTEM_DATAPATH)
#define NO_SYSTEM_DATAPATH
#endif

MainApplication::MainApplication(const QList<CommandLineOptions::ActionPair> &cmdActions, int &argc, char** argv)
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
    , m_iconProvider(new IconProvider(this))
    , m_searchEnginesManager(0)
    , m_dbWriter(new DatabaseWriter())
    , m_isClosing(false)
    , m_isStateChanged(false)
    , m_isExited(false)
    , m_isRestoring(false)
    , m_databaseConnected(false)
{
#if defined(Q_WS_X11) & !defined(NO_SYSTEM_DATAPATH)
    DATADIR = USE_DATADIR;
#else
    DATADIR = qApp->applicationDirPath() + "/";
#endif

#ifdef PORTABLE_BUILD
    PROFILEDIR = DATADIR + "profiles/";
#else
    PROFILEDIR = QDir::homePath() + "/.qupzilla/";
#endif

    PLUGINSDIR = DATADIR + "plugins/";
    TRANSLATIONSDIR = DATADIR + "locale/";
    THEMESDIR = DATADIR + "themes/";

    setOverrideCursor(Qt::WaitCursor);
    setWindowIcon(QupZilla::qupzillaIcon());
    bool noAddons = false;
    QUrl startUrl("");
    QStringList messages;
    QString startProfile;

    if (argc > 1) {
        foreach(CommandLineOptions::ActionPair pair, cmdActions) {
            switch (pair.action) {
            case CommandLineOptions::StartWithoutAddons:
                noAddons = true;
                break;
            case CommandLineOptions::StartWithProfile:
                startProfile = pair.text;
                break;
            case CommandLineOptions::NewTab:
                messages.append("ACTION:NewTab");
                m_postLaunchActions.append(OpenNewTab);
                break;
            case CommandLineOptions::NewWindow:
                messages.append("ACTION:NewWindow");
                break;
            case CommandLineOptions::ShowDownloadManager:
                messages.append("ACTION:ShowDownloadManager");
                m_postLaunchActions.append(OpenDownloadManager);
                break;
            case CommandLineOptions::StartPrivateBrowsing:
                messages.append("ACTION:StartPrivateBrowsing");
                m_postLaunchActions.append(PrivateBrowsing);
                break;
            case CommandLineOptions::OpenUrl:
                startUrl = pair.text;
                messages.append("URL:" + startUrl.toString());
                break;
            default:
                break;
            }
        }
    }

    if (messages.isEmpty()) {
        messages.append(" ");
    }

    if (isRunning()) {
        foreach(QString message, messages) {
            sendMessage(message);
        }
        m_isExited = true;
        return;
    }

    connect(this, SIGNAL(messageReceived(QString)), this, SLOT(receiveAppMessage(QString)));

#ifdef Q_WS_MAC
    setQuitOnLastWindowClosed(false);
#else
    setQuitOnLastWindowClosed(true);
#endif

    setApplicationName("QupZilla");
    setApplicationVersion(QupZilla::VERSION);
    setOrganizationDomain("qupzilla");
    QDesktopServices::setUrlHandler("http", this, "addNewTab");

    checkSettingsDir();

    QSettings::setDefaultFormat(QSettings::IniFormat);
    if (startProfile.isEmpty()) {
        QSettings settings(PROFILEDIR + "profiles/profiles.ini", QSettings::IniFormat);
        if (settings.value("Profiles/startProfile", "default").toString().contains("/")) {
            m_activeProfil = PROFILEDIR + "profiles/default/";
        }
        else {
            m_activeProfil = PROFILEDIR + "profiles/" + settings.value("Profiles/startProfile", "default").toString() + "/";
        }
    }
    else {
        m_activeProfil = PROFILEDIR + "profiles/" + startProfile + "/";
    }

    ProfileUpdater u(m_activeProfil);
    u.checkProfile();
    connectDatabase();

    QSettings settings2(m_activeProfil + "settings.ini", QSettings::IniFormat);
    settings2.beginGroup("SessionRestore");
    if (settings2.value("isRunning", false).toBool()) {
        settings2.setValue("isCrashed", true);
    }
    settings2.setValue("isRunning", true);
    settings2.endGroup();

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_activeProfil);

    translateApp();
    QWebHistoryInterface::setDefaultInterface(new WebHistoryInterface(this));

    QupZilla* qupzilla = new QupZilla(QupZilla::FirstAppWindow, startUrl);
    m_mainWindows.append(qupzilla);
    connect(qupzilla, SIGNAL(message(MainApplication::MessageType, bool)), this, SLOT(sendMessages(MainApplication::MessageType, bool)));
    qupzilla->show();

    AutoSaver* saver = new AutoSaver();
    connect(saver, SIGNAL(saveApp()), this, SLOT(saveStateSlot()));

    if (settings2.value("Web-Browser-Settings/CheckUpdates", DEFAULT_CHECK_UPDATES).toBool()) {
        m_updater = new Updater(qupzilla);
    }

    if (noAddons) {
        settings2.setValue("Plugin-Settings/AllowedPlugins", QStringList());
        settings2.setValue("Plugin-Settings/EnablePlugins", false);
    }

    networkManager()->loadCertificates();
    plugins()->loadPlugins();
    loadSettings();

    QTimer::singleShot(0, this, SLOT(postLaunch()));
    QTimer::singleShot(2000, this, SLOT(restoreCursor()));
#ifdef Q_WS_WIN
    QTimer::singleShot(10 * 1000, this, SLOT(setupJumpList()));
#endif
}

void MainApplication::postLaunch()
{
    if (m_postLaunchActions.contains(PrivateBrowsing)) {
        togglePrivateBrowsingMode(true);
    }

    if (m_postLaunchActions.contains(OpenDownloadManager)) {
        downManager()->show();
    }

    if (m_postLaunchActions.contains(OpenNewTab)) {
        getWindow()->tabWidget()->addView();
    }
}

void MainApplication::loadSettings()
{
    QSettings settings(m_activeProfil + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Themes");
    QString activeTheme = settings.value("activeTheme", DEFAULT_THEME_NAME).toString();
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
#ifdef Q_WS_MAC
    if (QFile(m_activeThemePath + "mac.css").exists()) {
        cssFile.setFileName(m_activeThemePath + "mac.css");
        cssFile.open(QFile::ReadOnly);
        css.append(cssFile.readAll());
        cssFile.close();
    }
#endif
#if defined(Q_WS_WIN) || defined(Q_OS_OS2)
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
    bool allowFlash = settings.value("allowFlash", true).toBool();
    bool allowJavaScript = settings.value("allowJavaScript", true).toBool();
    bool allowJavaScriptOpenWindow = settings.value("allowJavaScriptOpenWindow", false).toBool();
    bool allowJava = settings.value("allowJava", true).toBool();
    bool allowPersistentStorage = settings.value("allowPersistentStorage", true).toBool();
    bool allowImages = settings.value("autoLoadImages", true).toBool();
    bool dnsPrefetch = settings.value("DNS-Prefetch", false).toBool();
    bool jsClipboard = settings.value("JavaScriptCanAccessClipboard", true).toBool();
    bool linkInFocuschain = settings.value("IncludeLinkInFocusChain", false).toBool();
    bool zoomTextOnly = settings.value("zoomTextOnly", false).toBool();
    bool printElBg = settings.value("PrintElementBackground", true).toBool();
    bool xssAuditing = settings.value("XSSAuditing", false).toBool();
    int maxCachedPages = settings.value("maximumCachedPages", 3).toInt();
    int scrollingLines = settings.value("wheelScrollLines", wheelScrollLines()).toInt();
    QUrl userStyleSheet = QUrl::fromLocalFile(settings.value("userStyleSheet", "").toString());
    m_defaultZoom = settings.value("DefaultZoom", 100).toInt();
    WebPage::UserAgent = settings.value("UserAgent", "").toString();
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
    m_websettings->setAttribute(QWebSettings::XSSAuditingEnabled, xssAuditing);
#ifdef USE_WEBGL
    m_websettings->setAttribute(QWebSettings::WebGLEnabled, true);
    m_websettings->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);
#endif

#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
    m_websettings->setAttribute(QWebSettings::HyperlinkAuditingEnabled, true);
    m_websettings->setAttribute(QWebSettings::JavascriptCanCloseWindows, true);
#endif

    settings.beginGroup("Browser-Fonts");
    m_websettings->setFontFamily(QWebSettings::StandardFont, settings.value("StandardFont", m_websettings->fontFamily(QWebSettings::StandardFont)).toString());
    m_websettings->setFontFamily(QWebSettings::CursiveFont, settings.value("CursiveFont", m_websettings->fontFamily(QWebSettings::CursiveFont)).toString());
    m_websettings->setFontFamily(QWebSettings::FantasyFont, settings.value("FantasyFont", m_websettings->fontFamily(QWebSettings::FantasyFont)).toString());
    m_websettings->setFontFamily(QWebSettings::FixedFont, settings.value("FixedFont", m_websettings->fontFamily(QWebSettings::FixedFont)).toString());
    m_websettings->setFontFamily(QWebSettings::SansSerifFont, settings.value("SansSerifFont", m_websettings->fontFamily(QWebSettings::SansSerifFont)).toString());
    m_websettings->setFontFamily(QWebSettings::SerifFont, settings.value("SerifFont", m_websettings->fontFamily(QWebSettings::SerifFont)).toString());
    m_websettings->setFontSize(QWebSettings::DefaultFontSize, settings.value("DefaultFontSize", m_websettings->fontSize(QWebSettings::DefaultFontSize)).toInt());
    m_websettings->setFontSize(QWebSettings::DefaultFixedFontSize, settings.value("FixedFontSize", m_websettings->fontSize(QWebSettings::DefaultFixedFontSize)).toInt());
    settings.endGroup();

    m_websettings->setUserStyleSheetUrl(userStyleSheet);
    m_websettings->setDefaultTextEncoding("System");
    m_websettings->setWebGraphic(QWebSettings::DefaultFrameIconGraphic, IconProvider::fromTheme("text-plain").pixmap(16, 16));

    // Allows to load files from qrc: scheme in qupzilla: pages
    QWebSecurityOrigin::addLocalScheme("qupzilla");

    if (allowPersistentStorage) {
        m_websettings->enablePersistentStorage(m_activeProfil);
    }
    m_websettings->setMaximumPagesInCache(maxCachedPages);

    setWheelScrollLines(scrollingLines);

    if (m_downloadManager) {
        m_downloadManager->loadSettings();
    }
}

void MainApplication::restoreCursor()
{
    QApplication::restoreOverrideCursor();
}

void MainApplication::setupJumpList()
{
    QtWin::setupJumpList();
}

QupZilla* MainApplication::getWindow()
{
    for (int i = 0; i < m_mainWindows.count(); i++) {
        if (!m_mainWindows.at(i)) {
            continue;
        }
        return m_mainWindows.at(i).data();
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

void MainApplication::togglePrivateBrowsingMode(bool state)
{
    webSettings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, state);
    history()->setSaving(!state);
    cookieJar()->turnPrivateJar(state);

    emit message(MainApplication::CheckPrivateBrowsing, state);
}

void MainApplication::sendMessages(MainApplication::MessageType mes, bool state)
{
    emit message(mes, state);
}

void MainApplication::receiveAppMessage(QString message)
{
    QWidget* actWin = getWindow();
    if (message.startsWith("URL:")) {
        QString url(message.remove("URL:"));
        addNewTab(WebView::guessUrlFromString(url));
        actWin = getWindow();
    }
    else if (message.startsWith("ACTION:")) {
        QString text = message.mid(7);
        if (text == "NewTab") {
            addNewTab();
            actWin = getWindow();
        }
        else if (text == "NewWindow") {
            actWin = makeNewWindow(false);
        }
        else if (text == "ShowDownloadManager") {
            downManager()->show();
            actWin = downManager();
        }
        else if (text == "StartPrivateBrowsing") {
            sendMessages(StartPrivateBrowsing, true);
            actWin = getWindow();
        }
    }

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
    if (!getWindow()) {
        return;
    }
    getWindow()->tabWidget()->addView(url);
}

QupZilla* MainApplication::makeNewWindow(bool tryRestore, const QUrl &startUrl)
{
    QupZilla::StartBehaviour behaviour;
    if (tryRestore) {
        behaviour = QupZilla::OtherRestoredWindow;
    }
    else {
        behaviour = QupZilla::NewWindow;
    }

    QupZilla* newWindow = new QupZilla(behaviour, startUrl);
    connect(newWindow, SIGNAL(message(MainApplication::MessageType, bool)), this, SLOT(sendMessages(MainApplication::MessageType, bool)));
    m_mainWindows.append(newWindow);
    newWindow->show();

    return newWindow;
}

void MainApplication::connectDatabase()
{
    if (m_databaseConnected) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(m_activeProfil + "browsedata.db");
    if (!QFile::exists(m_activeProfil + "browsedata.db")) {
        QFile(":data/browsedata.db").copy(m_activeProfil + "browsedata.db");
        QFile(m_activeProfil + "browsedata.db").setPermissions(QFile::ReadUser | QFile::WriteUser);

        db.setDatabaseName(m_activeProfil + "browsedata.db");
        qWarning("Cannot find SQLite database file! Copying and using the defaults!");
    }
    if (!db.open()) {
        qWarning("Cannot open SQLite database! Continuing without database....");
    }

    m_databaseConnected = true;
}

void MainApplication::translateApp()
{
    QLocale locale;
    QSettings settings(m_activeProfil + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Language");
    QString file = settings.value("language", locale.name() + ".qm").toString();
    QString shortLoc = file.left(2);
    QString longLoc = file.left(5);

    if (file == "" || !QFile::exists(TRANSLATIONSDIR + file)) {
        return;
    }

    QTranslator* app = new QTranslator();
    app->load(TRANSLATIONSDIR + file);
    QTranslator* sys = new QTranslator();

    if (QFile::exists(TRANSLATIONSDIR + "qt_" + longLoc + ".qm")) {
        sys->load(TRANSLATIONSDIR + "qt_" + longLoc + ".qm");
    }
    else if (QFile::exists(TRANSLATIONSDIR + "qt_" + shortLoc + ".qm")) {
        sys->load(TRANSLATIONSDIR + "qt_" + shortLoc + ".qm");
    }

    m_activeLanguage = file;

    installTranslator(app);
    installTranslator(sys);
}

void MainApplication::quitApplication()
{
    bool isPrivate = m_websettings->testAttribute(QWebSettings::PrivateBrowsingEnabled);

    if (m_downloadManager && !m_downloadManager->canClose()) {
        m_downloadManager->show();
        return;
    }

    m_isClosing = true;
    if (m_mainWindows.count() > 0) {
        saveStateSlot();
    }

    QSettings settings(m_activeProfil + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("SessionRestore");
    settings.setValue("isRunning", false);
    settings.setValue("isCrashed", false);
    settings.endGroup();

    bool deleteCookies = settings.value("Web-Browser-Settings/deleteCookiesOnClose", false).toBool();
    bool deleteHistory = settings.value("Web-Browser-Settings/deleteHistoryOnClose", false).toBool();

    if (deleteCookies && !isPrivate) {
        QFile::remove(m_activeProfil + "cookies.dat");
    }
    if (deleteHistory) {
        m_historymodel->clearHistory();
    }

    m_searchEnginesManager->saveSettings();
    cookieJar()->saveCookies();
    m_networkmanager->saveCertificates();
    m_plugins->c2f_saveSettings();
    m_plugins->speedDial()->saveSettings();
    AdBlockManager::instance()->save();
    QFile::remove(getActiveProfilPath() + "WebpageIcons.db");
    m_iconProvider->saveIconsToDatabase();

//    qDebug() << "Quitting application...";
    quit();
}

BrowsingLibrary* MainApplication::browsingLibrary()
{
    if (!m_browsingLibrary) {
        m_browsingLibrary = new BrowsingLibrary(getWindow());
    }
    return m_browsingLibrary;
}

PluginProxy* MainApplication::plugins()
{
    if (!m_plugins) {
        m_plugins = new PluginProxy();
    }
    return m_plugins;
}

CookieManager* MainApplication::cookieManager()
{
    if (!m_cookiemanager) {
        m_cookiemanager = new CookieManager();
    }
    return m_cookiemanager;
}

HistoryModel* MainApplication::history()
{
    if (!m_historymodel) {
        m_historymodel = new HistoryModel(getWindow());
    }
    return m_historymodel;
}

QWebSettings* MainApplication::webSettings()
{
    if (!m_websettings) {
        m_websettings = QWebSettings::globalSettings();
    }
    return m_websettings;
}

NetworkManager* MainApplication::networkManager()
{
    if (!m_networkmanager) {
        m_networkmanager = new NetworkManager(getWindow());
    }
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
    if (!m_rssmanager) {
        m_rssmanager = new RSSManager(getWindow());
    }
    return m_rssmanager;
}

BookmarksModel* MainApplication::bookmarksModel()
{
    if (!m_bookmarksModel) {
        m_bookmarksModel = new BookmarksModel(this);
    }
    return m_bookmarksModel;
}

DownloadManager* MainApplication::downManager()
{
    if (!m_downloadManager) {
        m_downloadManager = new DownloadManager();
    }
    return m_downloadManager;
}

AutoFillModel* MainApplication::autoFill()
{
    if (!m_autofill) {
        m_autofill = new AutoFillModel(getWindow());
    }
    return m_autofill;
}

SearchEnginesManager* MainApplication::searchEnginesManager()
{
    if (!m_searchEnginesManager) {
        m_searchEnginesManager = new SearchEnginesManager();
    }
    return m_searchEnginesManager;
}

DesktopNotificationsFactory* MainApplication::desktopNotifications()
{
    if (!m_desktopNotifications) {
        m_desktopNotifications = new DesktopNotificationsFactory(this);
    }
    return m_desktopNotifications;
}

void MainApplication::aboutToCloseWindow(QupZilla* window)
{
    if (!window) {
        return;
    }

    m_mainWindows.removeOne(window);
}

//Version of session.dat file
static const int sessionVersion = 0x0002;

bool MainApplication::saveStateSlot()
{
    if (m_websettings->testAttribute(QWebSettings::PrivateBrowsingEnabled) || m_isRestoring) {
        return false;
    }

    QSettings settings(m_activeProfil + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("SessionRestore");
    settings.setValue("restoreSession", false);

    QFile file(m_activeProfil + "session.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);

    stream << sessionVersion;
    stream << m_mainWindows.count();
    for (int i = 0; i < m_mainWindows.count(); i++) {
        QupZilla* qz = m_mainWindows.at(i).data();
        if (!qz) {
            continue;
        }
        stream << qz->tabWidget()->saveState();
        if (qz->isFullScreen()) {
            stream << QByteArray();
        }
        else {
            stream << qz->saveState();
        }
    }
    file.close();

    settings.setValue("restoreSession", true);
    settings.endGroup();

    QupZilla* qupzilla_ = getWindow();
    if (qupzilla_) {
        qupzilla_->tabWidget()->savePinnedTabs();
    }

    return true;
}

bool MainApplication::restoreStateSlot(QupZilla* window)
{
    if (m_postLaunchActions.contains(PrivateBrowsing)) {
        return false;
    }

    m_isRestoring = true;
    QSettings settings(m_activeProfil + "settings.ini", QSettings::IniFormat);
    int afterStart = settings.value("Web-URL-Settings/afterLaunch", 1).toInt();
    settings.beginGroup("SessionRestore");
    if (!settings.value("restoreSession", false).toBool()) {
        m_isRestoring = false;
        return false;
    }
    if (settings.value("isCrashed", false).toBool() && afterStart != 3) {
        QMessageBox::StandardButton button = QMessageBox::warning(window, tr("Last session crashed"),
                                             tr("<b>QupZilla crashed :-(</b><br/>Oops, the last session of QupZilla was interrupted unexpectedly. We apologize for this. Would you like to try restoring the last saved state?"),
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (button != QMessageBox::Yes) {
            m_isRestoring = false;
            return false;
        }
    }
    if (!QFile::exists(m_activeProfil + "session.dat")) {
        m_isRestoring = false;
        return false;
    }

    settings.setValue("isCrashed", false);
    QFile file(m_activeProfil + "session.dat");
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

            QupZilla* window = new QupZilla(QupZilla::OtherRestoredWindow);
            m_mainWindows.append(window);
            connect(window, SIGNAL(message(MainApplication::MessageType, bool)), this, SLOT(sendMessages(MainApplication::MessageType, bool)));
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
        |
    browsedata.db
    */

    if (QDir(PROFILEDIR).exists() && QFile(PROFILEDIR + "profiles/profiles.ini").exists()) {
        return true;
    }

    std::cout << "Creating new profile directory" << std::endl;

    QDir dir(PROFILEDIR);

    if (!dir.exists()) {
        dir.mkpath(PROFILEDIR);
    }

    dir.mkdir("profiles");
    dir.cd("profiles");

    //.qupzilla/profiles
    QFile(PROFILEDIR + "profiles/profiles.ini").remove();
    QFile(":data/profiles.ini").copy(PROFILEDIR + "profiles/profiles.ini");
    QFile(PROFILEDIR + "profiles/profiles.ini").setPermissions(QFile::ReadUser | QFile::WriteUser);

    dir.mkdir("default");
    dir.cd("default");

    //.qupzilla/profiles/default
    QFile(PROFILEDIR + "profiles/default/browsedata.db").remove();
    QFile(":data/browsedata.db").copy(PROFILEDIR + "profiles/default/browsedata.db");
    QFile(PROFILEDIR + "profiles/default/browsedata.db").setPermissions(QFile::ReadUser | QFile::WriteUser);

    return dir.isReadable();
}
