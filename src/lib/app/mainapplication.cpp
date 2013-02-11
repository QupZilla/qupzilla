/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "history.h"
#include "networkmanager.h"
#include "rssmanager.h"
#include "updater.h"
#include "autosaver.h"
#include "pluginproxy.h"
#include "bookmarksmodel.h"
#include "downloadmanager.h"
#include "autofill.h"
#include "adblockmanager.h"
#include "desktopnotificationsfactory.h"
#include "iconprovider.h"
#include "qtwin.h"
#include "mainapplication.h"
#include "webhistoryinterface.h"
#include "qztools.h"
#include "profileupdater.h"
#include "searchenginesmanager.h"
#include "databasewriter.h"
#include "speeddial.h"
#include "webpage.h"
#include "settings.h"
#include "qzsettings.h"
#include "clearprivatedata.h"
#include "commandlineoptions.h"
#include "useragentmanager.h"
#include "restoremanager.h"
#include "proxystyle.h"
#include "checkboxdialog.h"
#include "registerqappassociation.h"
#include "html5permissions/html5permissionsmanager.h"

#ifdef USE_HUNSPELL
#include "qtwebkit/spellcheck/speller.h"
#endif

#ifdef Q_OS_MAC
#include "macmenureceiver.h"
#include <QFileOpenEvent>
#endif
#include <QNetworkDiskCache>
#include <QDesktopServices>
#include <QTranslator>
#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QDir>

#if defined(PORTABLE_BUILD) && !defined(NO_SYSTEM_DATAPATH)
#define NO_SYSTEM_DATAPATH
#endif

#if QT_VERSION < 0x050000
#include "qwebkitversion.h"
#endif

MainApplication::MainApplication(int &argc, char** argv)
    : QtSingleApplication(argc, argv)
    , m_cookiemanager(0)
    , m_browsingLibrary(0)
    , m_historymodel(0)
    , m_websettings(0)
    , m_networkmanager(0)
    , m_cookiejar(0)
    , m_rssmanager(0)
    , m_bookmarksModel(0)
    , m_downloadManager(0)
    , m_autofill(0)
    , m_networkCache(0)
    , m_desktopNotifications(0)
    , m_searchEnginesManager(0)
    , m_restoreManager(0)
    , m_proxyStyle(0)
    , m_html5permissions(0)
#ifdef USE_HUNSPELL
    , m_speller(0)
#endif
    , m_dbWriter(new DatabaseWriter(this))
    , m_uaManager(new UserAgentManager)
    , m_isPrivateSession(false)
    , m_isClosing(false)
    , m_isStateChanged(false)
    , m_isRestoring(false)
    , m_startingAfterCrash(false)
    , m_databaseConnected(false)
#ifdef Q_OS_WIN
    , m_registerQAppAssociation(0)
#endif
#ifdef Q_OS_MAC
    , m_macMenuReceiver(0)
#endif
{
#if defined(QZ_WS_X11) && !defined(NO_SYSTEM_DATAPATH)
    DATADIR = USE_DATADIR;
#else
    DATADIR = qApp->applicationDirPath() + "/";
#endif

#ifdef Q_OS_MAC
    DATADIR.append(QLatin1String("../Resources/"));
#endif

#ifdef PORTABLE_BUILD
    PROFILEDIR = DATADIR + "profiles/";
#else
    PROFILEDIR = QDir::homePath() + "/.qupzilla/";
#endif

    TRANSLATIONSDIR = DATADIR + "locale/";
    THEMESDIR = DATADIR + "themes/";

    setWindowIcon(QIcon(":icons/exeicons/qupzilla-window.png"));
    bool noAddons = false;
    bool newInstance = false;
    QUrl startUrl;
    QStringList messages;
    QString startProfile;

    if (argc > 1) {
        CommandLineOptions cmd(argc);

        foreach(const CommandLineOptions::ActionPair & pair, cmd.getActions()) {
            switch (pair.action) {
            case Qz::CL_StartWithoutAddons:
                noAddons = true;
                break;
            case Qz::CL_StartWithProfile:
                startProfile = pair.text;
                break;
            case Qz::CL_NewTab:
                messages.append(QLatin1String("ACTION:NewTab"));
                m_postLaunchActions.append(OpenNewTab);
                break;
            case Qz::CL_NewWindow:
                messages.append(QLatin1String("ACTION:NewWindow"));
                break;
            case Qz::CL_ShowDownloadManager:
                messages.append(QLatin1String("ACTION:ShowDownloadManager"));
                m_postLaunchActions.append(OpenDownloadManager);
                break;
            case Qz::CL_StartPrivateBrowsing:
                m_isPrivateSession = true;
                break;
            case Qz::CL_StartNewInstance:
                newInstance = true;
                break;
            case Qz::CL_OpenUrlInCurrentTab:
                startUrl = QUrl::fromUserInput(pair.text);
                messages.append("ACTION:OpenUrlInCurrentTab" + pair.text);
                break;
            case Qz::CL_OpenUrlInNewWindow:
                startUrl = QUrl::fromUserInput(pair.text);
                messages.append("ACTION:OpenUrlInNewWindow" + pair.text);
                break;
            case Qz::CL_OpenUrl:
                startUrl = QUrl::fromUserInput(pair.text);
                messages.append("URL:" + pair.text);
                break;
            case Qz::CL_ExitAction:
                m_isClosing = true;
                return;
            default:
                break;
            }
        }
    }

    // Don't start single application in private browsing
    if (!m_isPrivateSession) {
        QString appId = "QupZillaWebBrowser";

        if (newInstance) {
            if (startProfile.isEmpty() || startProfile == QLatin1String("default")) {
                std::cout << "New instance cannot be started with default profile!" << std::endl;
            }
            else {
                appId.append(startProfile);
            }
        }

        setAppId(appId);
    }

    if (messages.isEmpty()) {
        messages.append(QLatin1String(" "));
    }

    if (isRunning()) {
        foreach(const QString & message, messages) {
            sendMessage(message);
        }
        m_isClosing = true;
        return;
    }

#ifdef Q_OS_MAC
    setQuitOnLastWindowClosed(false);
#else
    setQuitOnLastWindowClosed(true);
#endif

    setApplicationName("QupZilla");
    setApplicationVersion(QupZilla::VERSION);
    setOrganizationDomain("qupzilla");
    QDesktopServices::setUrlHandler("http", this, "addNewTab");
    QDesktopServices::setUrlHandler("ftp", this, "addNewTab");

    checkSettingsDir();

    QSettings::setDefaultFormat(QSettings::IniFormat);
    if (startProfile.isEmpty()) {
        QSettings settings(PROFILEDIR + "profiles/profiles.ini", QSettings::IniFormat);
        if (settings.value("Profiles/startProfile", "default").toString().contains(QLatin1Char('/'))) {
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

    Settings::createSettings(m_activeProfil + "settings.ini");

    translateApp();

    QupZilla* qupzilla = new QupZilla(Qz::BW_FirstAppWindow, startUrl);
    m_mainWindows.append(qupzilla);

    connect(qupzilla, SIGNAL(message(Qz::AppMessageType, bool)), this, SLOT(sendMessages(Qz::AppMessageType, bool)));
    connect(qupzilla, SIGNAL(startingCompleted()), this, SLOT(restoreCursor()));

    loadSettings();
    networkManager()->loadCertificates();

    m_plugins = new PluginProxy;

    if (!noAddons) {
        m_plugins->loadPlugins();
    }

    if (!m_isPrivateSession) {
        Settings settings;
        m_startingAfterCrash = settings.value("SessionRestore/isRunning", false).toBool();
        bool checkUpdates = settings.value("Web-Browser-Settings/CheckUpdates", DEFAULT_CHECK_UPDATES).toBool();
        int afterLaunch = settings.value("Web-URL-Settings/afterLaunch", 1).toInt();
        settings.setValue("SessionRestore/isRunning", true);

#ifndef PORTABLE_BUILD
        bool alwaysCheckDefaultBrowser = settings.value("Web-Browser-Settings/CheckDefaultBrowser", DEFAULT_CHECK_DEFAULTBROWSER).toBool();
        if (alwaysCheckDefaultBrowser) {
            alwaysCheckDefaultBrowser = checkDefaultWebBrowser();
            settings.setValue("Web-Browser-Settings/CheckDefaultBrowser", alwaysCheckDefaultBrowser);
        }
#endif

        if (checkUpdates) {
            new Updater(qupzilla);
        }
        if (m_startingAfterCrash || afterLaunch == 3) {
            m_restoreManager = new RestoreManager(m_activeProfil + "session.dat");
            if (!m_restoreManager->isValid()) {
                destroyRestoreManager();
            }
        }
    }

    QTimer::singleShot(0, this, SLOT(postLaunch()));
#ifdef Q_OS_WIN
    QTimer::singleShot(10 * 1000, this, SLOT(setupJumpList()));
#endif
}

void MainApplication::postLaunch()
{
    if (m_postLaunchActions.contains(OpenDownloadManager)) {
        downManager()->show();
    }

    if (m_postLaunchActions.contains(OpenNewTab)) {
        getWindow()->tabWidget()->addView(QUrl(), Qz::NT_SelectedNewEmptyTab);
    }

    AutoSaver* saver = new AutoSaver();
    connect(saver, SIGNAL(saveApp()), this, SLOT(saveStateSlot()));

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_activeProfil);
    QWebHistoryInterface::setDefaultInterface(new WebHistoryInterface(this));

    connect(this, SIGNAL(messageReceived(QString)), this, SLOT(receiveAppMessage(QString)));
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));
}

void MainApplication::loadSettings()
{
    Settings settings;
    settings.beginGroup("Themes");
    QString activeTheme = settings.value("activeTheme", DEFAULT_THEME_NAME).toString();
    settings.endGroup();
    m_activeThemePath = THEMESDIR + activeTheme + "/";
    QFile cssFile(m_activeThemePath + "main.css");
    cssFile.open(QFile::ReadOnly);
    QString css = cssFile.readAll();
    cssFile.close();
#ifdef QZ_WS_X11
    if (QFile(m_activeThemePath + "linux.css").exists()) {
        cssFile.setFileName(m_activeThemePath + "linux.css");
        cssFile.open(QFile::ReadOnly);
        css.append(cssFile.readAll());
        cssFile.close();
    }
#endif
#ifdef Q_OS_MAC
    if (QFile(m_activeThemePath + "mac.css").exists()) {
        cssFile.setFileName(m_activeThemePath + "mac.css");
        cssFile.open(QFile::ReadOnly);
        css.append(cssFile.readAll());
        cssFile.close();
    }
#endif
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
    if (QFile(m_activeThemePath + "windows.css").exists()) {
        cssFile.setFileName(m_activeThemePath + "windows.css");
        cssFile.open(QFile::ReadOnly);
        css.append(cssFile.readAll());
        cssFile.close();
    }
#endif

    //RTL Support
    //loading 'rtl.css' when layout is right to left!
    if (isRightToLeft() && QFile(m_activeThemePath + "rtl.css").exists()) {
        cssFile.setFileName(m_activeThemePath + "rtl.css");
        cssFile.open(QFile::ReadOnly);
        css.append(cssFile.readAll());
        cssFile.close();
    }

    QString relativePath = QDir::current().relativeFilePath(m_activeThemePath);
    css.replace(QRegExp("url\\s*\\(\\s*([^\\*:\\);]+)\\s*\\)", Qt::CaseSensitive, QRegExp::RegExp2),
                QString("url(%1\\1)").arg(relativePath + "/"));
    setStyleSheet(css);

    webSettings();

    //Web browsing settings
    settings.beginGroup("Web-Browser-Settings");

    if (!m_isPrivateSession) {
        m_websettings->enablePersistentStorage(m_activeProfil);
        m_websettings->setAttribute(QWebSettings::LocalStorageEnabled, settings.value("HTML5StorageEnabled", true).toBool());
    }

    m_websettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    m_websettings->setAttribute(QWebSettings::PluginsEnabled, settings.value("allowFlash", true).toBool());
    m_websettings->setAttribute(QWebSettings::JavascriptEnabled, settings.value("allowJavaScript", true).toBool());
    m_websettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, settings.value("allowJavaScriptOpenWindow", false).toBool());
    m_websettings->setAttribute(QWebSettings::JavaEnabled, settings.value("allowJava", true).toBool());
    m_websettings->setAttribute(QWebSettings::DnsPrefetchEnabled, settings.value("DNS-Prefetch", false).toBool());
    m_websettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, settings.value("allowJavaScriptAccessClipboard", true).toBool());
    m_websettings->setAttribute(QWebSettings::LinksIncludedInFocusChain, settings.value("IncludeLinkInFocusChain", false).toBool());
    m_websettings->setAttribute(QWebSettings::ZoomTextOnly, settings.value("zoomTextOnly", false).toBool());
    m_websettings->setAttribute(QWebSettings::PrintElementBackgrounds, settings.value("PrintElementBackground", true).toBool());
    m_websettings->setAttribute(QWebSettings::XSSAuditingEnabled, settings.value("XSSAuditing", false).toBool());
    m_websettings->setMaximumPagesInCache(settings.value("maximumCachedPages", 3).toInt());
    m_websettings->setDefaultTextEncoding(settings.value("DefaultEncoding", m_websettings->defaultTextEncoding()).toString());

#if QTWEBKIT_FROM_2_3
    m_websettings->setAttribute(QWebSettings::CaretBrowsingEnabled, settings.value("CaretBrowsing", false).toBool());
    m_websettings->setAttribute(QWebSettings::ScrollAnimatorEnabled, settings.value("AnimateScrolling", true).toBool());
#endif

#ifdef USE_WEBGL
    m_websettings->setAttribute(QWebSettings::WebGLEnabled, true);
    m_websettings->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);
#endif

#if QTWEBKIT_FROM_2_2
    m_websettings->setAttribute(QWebSettings::HyperlinkAuditingEnabled, true);
    m_websettings->setAttribute(QWebSettings::JavascriptCanCloseWindows, settings.value("allowJavaScriptCloseWindow", false).toBool());
#endif

    setWheelScrollLines(settings.value("wheelScrollLines", wheelScrollLines()).toInt());
    m_websettings->setUserStyleSheetUrl(userStyleSheet(settings.value("userStyleSheet", QString()).toString()));
    settings.endGroup();

    settings.beginGroup("Browser-Fonts");
    m_websettings->setFontFamily(QWebSettings::StandardFont, settings.value("StandardFont", m_websettings->fontFamily(QWebSettings::StandardFont)).toString());
    m_websettings->setFontFamily(QWebSettings::CursiveFont, settings.value("CursiveFont", m_websettings->fontFamily(QWebSettings::CursiveFont)).toString());
    m_websettings->setFontFamily(QWebSettings::FantasyFont, settings.value("FantasyFont", m_websettings->fontFamily(QWebSettings::FantasyFont)).toString());
    m_websettings->setFontFamily(QWebSettings::FixedFont, settings.value("FixedFont", m_websettings->fontFamily(QWebSettings::FixedFont)).toString());
    m_websettings->setFontFamily(QWebSettings::SansSerifFont, settings.value("SansSerifFont", m_websettings->fontFamily(QWebSettings::SansSerifFont)).toString());
    m_websettings->setFontFamily(QWebSettings::SerifFont, settings.value("SerifFont", m_websettings->fontFamily(QWebSettings::SerifFont)).toString());
    m_websettings->setFontSize(QWebSettings::DefaultFontSize, settings.value("DefaultFontSize", m_websettings->fontSize(QWebSettings::DefaultFontSize)).toInt());
    m_websettings->setFontSize(QWebSettings::DefaultFixedFontSize, settings.value("FixedFontSize", m_websettings->fontSize(QWebSettings::DefaultFixedFontSize)).toInt());
    m_websettings->setFontSize(QWebSettings::MinimumFontSize, settings.value("MinimumFontSize", m_websettings->fontSize(QWebSettings::MinimumFontSize)).toInt());
    m_websettings->setFontSize(QWebSettings::MinimumLogicalFontSize, settings.value("MinimumLogicalFontSize", m_websettings->fontSize(QWebSettings::MinimumLogicalFontSize)).toInt());
    settings.endGroup();

    m_websettings->setWebGraphic(QWebSettings::DefaultFrameIconGraphic, qIconProvider->emptyWebIcon().pixmap(16, 16));
    m_websettings->setWebGraphic(QWebSettings::MissingImageGraphic, QPixmap());

    if (m_isPrivateSession) {
        m_websettings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
        history()->setSaving(false);
    }

    if (m_downloadManager) {
        m_downloadManager->loadSettings();
    }

    qzSettings->loadSettings();
    m_uaManager->loadSettings();
}

void MainApplication::reloadSettings()
{
    loadSettings();
    emit message(Qz::AM_ReloadSettings, true);
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
    QupZilla* activeW = qobject_cast<QupZilla*>(activeWindow());
    if (activeW) {
        return activeW;
    }

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
    sendMessages(Qz::AM_HistoryStateChanged, true);
}

bool MainApplication::isStateChanged()
{
    if (m_isStateChanged) {
        m_isStateChanged = false;
        return true;
    }
    return false;
}

QList<QupZilla*> MainApplication::mainWindows()
{
    QList<QupZilla*> list;

    for (int i = 0; i < m_mainWindows.count(); i++) {
        if (!m_mainWindows.at(i)) {
            continue;
        }

        list.append(m_mainWindows.at(i).data());
    }

    return list;
}

bool MainApplication::isClosing() const
{
    return m_isClosing;
}

bool MainApplication::isRestoring() const
{
    return m_isRestoring;
}

bool MainApplication::isPrivateSession() const
{
    return m_isPrivateSession;
}

bool MainApplication::isStartingAfterCrash() const
{
    return m_startingAfterCrash;
}

int MainApplication::windowCount() const
{
    return m_mainWindows.count();
}

QString MainApplication::currentLanguageFile() const
{
    return m_activeLanguage;
}

QString MainApplication::currentLanguage() const
{
    QString lang = m_activeLanguage;

    if (lang.isEmpty()) {
        return "en_US";
    }

    return lang.left(lang.length() - 3);
}

QString MainApplication::currentProfilePath() const
{
    return m_activeProfil;
}

void MainApplication::sendMessages(Qz::AppMessageType mes, bool state)
{
    emit message(mes, state);
}

void MainApplication::receiveAppMessage(QString message)
{
    QWidget* actWin = getWindow();
    QUrl actUrl;

    if (message.startsWith(QLatin1String("URL:"))) {
        QUrl url = QUrl::fromUserInput(message.mid(4));
        addNewTab(url);
        actWin = getWindow();
    }
    else if (message.startsWith(QLatin1String("ACTION:"))) {
        QString text = message.mid(7);
        if (text == QLatin1String("NewTab")) {
            addNewTab();
        }
        else if (text == QLatin1String("NewWindow")) {
            actWin = makeNewWindow(Qz::BW_NewWindow);
        }
        else if (text == QLatin1String("ShowDownloadManager")) {
            downManager()->show();
            actWin = downManager();
        }
        else if (text.startsWith(QLatin1String("OpenUrlInCurrentTab"))) {
            actUrl = QUrl::fromUserInput(text.mid(19));
        }
        else if (text.startsWith(QLatin1String("OpenUrlInNewWindow"))) {
            makeNewWindow(Qz::BW_NewWindow, QUrl::fromUserInput(text.mid(18)));
            return;
        }
    }

    if (!actWin && !isClosing()) { // It can only occur if download manager window was still open
        makeNewWindow(Qz::BW_NewWindow, actUrl);
        return;
    }

    QupZilla* qz = qobject_cast<QupZilla*>(actWin);

    actWin->setWindowState(actWin->windowState() & ~Qt::WindowMinimized);
    actWin->raise();
    actWin->activateWindow();
    actWin->setFocus();

    if (qz && !actUrl.isEmpty()) {
        qz->loadAddress(actUrl);
    }
}

void MainApplication::addNewTab(const QUrl &url)
{
    if (!getWindow()) {
        return;
    }

    getWindow()->tabWidget()->addView(url, url.isEmpty() ? Qz::NT_SelectedNewEmptyTab : Qz::NT_SelectedTabAtTheEnd);
}

QupZilla* MainApplication::makeNewWindow(Qz::BrowserWindow type, const QUrl &startUrl)
{
    if (m_mainWindows.count() == 0 && type != Qz::BW_MacFirstWindow) {
        type = Qz::BW_FirstAppWindow;
    }

    QupZilla* newWindow = new QupZilla(type, startUrl);
    m_mainWindows.append(newWindow);

    return newWindow;
}

#ifdef Q_OS_MAC
MacMenuReceiver* MainApplication::macMenuReceiver()
{
    if (!m_macMenuReceiver) {
        m_macMenuReceiver = new MacMenuReceiver(this);
    }
    return m_macMenuReceiver;
}

bool MainApplication::event(QEvent* e)
{
    switch (e->type()) {
    case QEvent::FileOpen: {
        QString fileName = static_cast<QFileOpenEvent*>(e)->file();
        addNewTab(QUrl::fromLocalFile(fileName));
        return true;
    }
    break;

    default:
        break;
    }

    return QtSingleApplication::event(e);
}
#endif

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

    if (m_isPrivateSession) {
        db.setConnectOptions("QSQLITE_OPEN_READONLY");
    }

    if (!db.open()) {
        qWarning("Cannot open SQLite database! Continuing without database....");
    }

    m_databaseConnected = true;
}

void MainApplication::translateApp()
{
    Settings settings;
    QString file = settings.value("Language/language", QLocale::system().name()).toString();

    if (!file.isEmpty() && !file.endsWith(QLatin1String(".qm"))) {
        file.append(".qm");
    }

    QTranslator* app = new QTranslator(this);
    app->load(file, TRANSLATIONSDIR);

    QTranslator* sys = new QTranslator(this);
    sys->load("qt_" + file, TRANSLATIONSDIR);

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
    m_networkmanager->disconnectObjects();

    if (m_mainWindows.count() > 0) {
        saveStateSlot();
    }

    // Saving settings in saveSettings() slot called from quit() so
    // everything gets saved also when quitting application in other
    // way than clicking Quit action in File menu or closing last window
    //
    //  * this can occur on Mac OS (see #157)

    quit();
}

void MainApplication::saveSettings()
{
    if (m_isPrivateSession) {
        return;
    }

    m_isClosing = true;
    m_networkmanager->disconnectObjects();

    Settings settings;
    settings.beginGroup("SessionRestore");
    settings.setValue("isRunning", false);
    settings.endGroup();

    settings.beginGroup("Web-Browser-Settings");
    bool deleteHistory = settings.value("deleteHistoryOnClose", false).toBool();
    bool deleteHtml5Storage = settings.value("deleteHTML5StorageOnClose", false).toBool();
    settings.endGroup();

    if (deleteHistory) {
        m_historymodel->clearHistory();
    }
    if (deleteHtml5Storage) {
        ClearPrivateData::clearLocalStorage();
    }

    m_searchEnginesManager->saveSettings();
    m_networkmanager->saveCertificates();
    m_plugins->shutdown();
    qIconProvider->saveIconsToDatabase();
    clearTempPath();

    qzSettings->saveSettings();
    AdBlockManager::instance()->save();
    QFile::remove(currentProfilePath() + "WebpageIcons.db");
    Settings::syncSettings();
}

BrowsingLibrary* MainApplication::browsingLibrary()
{
    if (!m_browsingLibrary) {
        m_browsingLibrary = new BrowsingLibrary(getWindow());
    }
    return m_browsingLibrary;
}

CookieManager* MainApplication::cookieManager()
{
    if (!m_cookiemanager) {
        m_cookiemanager = new CookieManager();
    }
    return m_cookiemanager;
}

History* MainApplication::history()
{
    if (!m_historymodel) {
        m_historymodel = new History(this);
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

PluginProxy* MainApplication::plugins()
{
    return m_plugins;
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

AutoFill* MainApplication::autoFill()
{
    if (!m_autofill) {
        m_autofill = new AutoFill(getWindow());
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

QNetworkDiskCache* MainApplication::networkCache()
{
    if (!m_networkCache) {
        Settings settings;
        const QString &basePath = settings.value("Web-Browser-Settings/CachePath",
                                  QString("%1networkcache/").arg(m_activeProfil)).toString();

        const QString &cachePath = basePath + "/" + qWebKitVersion() + "/";
        m_networkCache = new QNetworkDiskCache(this);
        m_networkCache->setCacheDirectory(cachePath);
    }

    return m_networkCache;
}

DesktopNotificationsFactory* MainApplication::desktopNotifications()
{
    if (!m_desktopNotifications) {
        m_desktopNotifications = new DesktopNotificationsFactory(this);
    }
    return m_desktopNotifications;
}

HTML5PermissionsManager* MainApplication::html5permissions()
{
    if (!m_html5permissions) {
        m_html5permissions = new HTML5PermissionsManager(this);
    }
    return m_html5permissions;
}

#ifdef USE_HUNSPELL
Speller* MainApplication::speller()
{
    if (!m_speller) {
        m_speller = new Speller(this);
    }
    return m_speller;
}
#endif

void MainApplication::startPrivateBrowsing()
{
    QStringList args;
    foreach(const QString & arg, arguments()) {
        if (arg.startsWith(QLatin1Char('-'))) {
            args.append(arg);
        }
    }

    args.append(QLatin1String("--private-browsing"));

    if (!QProcess::startDetached(applicationFilePath(), args)) {
        qWarning() << "MainApplication: Cannot start new browser process for private browsing!" << applicationFilePath() << args;
    }
}

void MainApplication::reloadUserStyleSheet()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    m_websettings->setUserStyleSheetUrl(userStyleSheet(settings.value("userStyleSheet", QString()).toString()));
    settings.endGroup();
}

bool MainApplication::checkDefaultWebBrowser()
{
#ifdef Q_OS_WIN
    bool showAgain = true;
    if (!associationManager()->isDefaultForAllCapabilities()) {
        CheckBoxDialog dialog(QDialogButtonBox::Yes | QDialogButtonBox::No);
        dialog.setText(tr("QupZilla is not currently your default browser. Would you like to make it your default browser?"));
        dialog.setCheckBoxText(tr("Always perform this check when starting QupZilla."));
        dialog.setWindowTitle(tr("Default Browser"));
        dialog.setIcon(qIconProvider->standardIcon(QStyle::SP_MessageBoxWarning));

        if (dialog.exec() == QDialog::Accepted) {
            associationManager()->registerAllAssociation();
        }

        showAgain = dialog.isChecked();
    }

    return showAgain;
#else
    return false;
#endif
}

#ifdef Q_OS_WIN
RegisterQAppAssociation* MainApplication::associationManager()
{
    if (!m_registerQAppAssociation) {
        QString desc = tr("QupZilla is a new, fast and secure open-source WWW browser. QupZilla is licensed under GPL version 3 or (at your option) any later version. It is based on WebKit core and Qt Framework.");
        QString fileIconPath = QApplication::applicationFilePath() + ",1";
        QString appIconPath = QApplication::applicationFilePath() + ",0";
        m_registerQAppAssociation = new RegisterQAppAssociation("QupZilla", QApplication::applicationFilePath(), appIconPath, desc, this);
        m_registerQAppAssociation->addCapability(".html", "QupZilla.HTML", "HTML File", fileIconPath, RegisterQAppAssociation::FileAssociation);
        m_registerQAppAssociation->addCapability(".htm", "QupZilla.HTM", "HTM File", fileIconPath, RegisterQAppAssociation::FileAssociation);
        m_registerQAppAssociation->addCapability("http", "QupZilla.HTTP", "URL:HyperText Transfer Protocol", appIconPath, RegisterQAppAssociation::UrlAssociation);
        m_registerQAppAssociation->addCapability("https", "QupZilla.HTTPS", "URL:HyperText Transfer Protocol with Privacy", appIconPath, RegisterQAppAssociation::UrlAssociation);
        m_registerQAppAssociation->addCapability("ftp", "QupZilla.FTP", "URL:File Transfer Protocol", appIconPath, RegisterQAppAssociation::UrlAssociation);
    }
    return m_registerQAppAssociation;
}
#endif

QUrl MainApplication::userStyleSheet(const QString &filePath) const
{
    QString userStyle = AdBlockManager::instance()->elementHidingRules() + "{ display:none !important;}";

    QFile file(filePath);
    if (!filePath.isEmpty() && file.open(QFile::ReadOnly)) {
        QString fileData = QString::fromUtf8(file.readAll());
        fileData.remove(QLatin1Char('\n'));
        userStyle.append(fileData);
        file.close();
    }

    const QString &encodedStyle = userStyle.toLatin1().toBase64();
    const QString &dataString = QString("data:text/css;charset=utf-8;base64,%1").arg(encodedStyle);

    return QUrl(dataString);
}

void MainApplication::aboutToCloseWindow(QupZilla* window)
{
    if (!window) {
        return;
    }

    if (m_mainWindows.count() == 1) {
        if (m_browsingLibrary) {
            m_browsingLibrary->close();
        }

        if (m_cookiemanager) {
            m_cookiemanager->close();
        }
    }

    m_mainWindows.removeOne(window);
}

bool MainApplication::saveStateSlot()
{
    if (m_isPrivateSession || m_isRestoring || m_mainWindows.count() == 0 || m_restoreManager) {
        return false;
    }

    QFile file(m_activeProfil + "session.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);

    stream << Qz::sessionVersion;
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

    QupZilla* qupzilla_ = getWindow();
    if (qupzilla_ && m_mainWindows.count() == 1) {
        qupzilla_->tabWidget()->savePinnedTabs();
    }

    // Saving cookies
    m_cookiejar->saveCookies();

    return true;
}

bool MainApplication::restoreStateSlot(QupZilla* window, RestoreData recoveryData)
{
    if (m_isPrivateSession || recoveryData.isEmpty()) {
        return false;
    }

    m_isRestoring = true;

    window->tabWidget()->closeRecoveryTab();

    if (window->tabWidget()->normalTabsCount() > 1) {
        // This can only happen when recovering crashed session!
        //
        // Don't restore tabs in current window as user already opened
        // some new tabs.
        // Instead create new one and restore pinned tabs there

        QupZilla* newWin = makeNewWindow(Qz::BW_OtherRestoredWindow);
        newWin->tabWidget()->restorePinnedTabs();
        newWin->restoreWindowState(recoveryData.takeFirst());
    }
    else {
        // QTabWidget::count() - count of tabs is not updated after closing
        // recovery tab ...
        int tabCount = window->tabWidget()->count();
        RestoreManager::WindowData data = recoveryData.takeFirst();
        data.currentTab += tabCount;

        window->restoreWindowState(data);
    }

    foreach(const RestoreManager::WindowData & data, recoveryData) {
        QupZilla* window = makeNewWindow(Qz::BW_OtherRestoredWindow);
        window->restoreWindowState(data);
    }

    destroyRestoreManager();
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

    std::cout << "QupZilla: Creating new profile directory" << std::endl;

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
    QFile versionFile(PROFILEDIR + "profiles/default/version");
    versionFile.open(QFile::WriteOnly);
    versionFile.write(QupZilla::VERSION.toUtf8());
    versionFile.close();

    return dir.isReadable();
}

void MainApplication::destroyRestoreManager()
{
    delete m_restoreManager;
    m_restoreManager = 0;
}

void MainApplication::setProxyStyle(ProxyStyle* style)
{
    m_proxyStyle = style;

    QApplication::setStyle(style);
}

QString MainApplication::currentStyle() const
{
    return m_proxyStyle->baseStyle()->objectName();
}

void MainApplication::clearTempPath()
{
    QString path = PROFILEDIR + "tmp/";
    QDir dir(path);

    if (dir.exists()) {
        QzTools::removeDir(path);
    }
}

QString MainApplication::tempPath() const
{
    QString path = PROFILEDIR + "tmp/";
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkdir(path);
    }

    return path;
}

MainApplication::~MainApplication()
{
    delete m_uaManager;
}
