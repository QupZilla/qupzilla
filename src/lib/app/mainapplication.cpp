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
#include "mainapplication.h"
#include "qtwin.h"
#include "history.h"
#include "qztools.h"
#include "updater.h"
#include "autofill.h"
#include "settings.h"
#include "qzregexp.h"
#include "autosaver.h"
#include "datapaths.h"
#include "tabwidget.h"
#include "cookiejar.h"
#include "bookmarks.h"
#include "qzsettings.h"
#include "rssmanager.h"
#include "proxystyle.h"
#include "pluginproxy.h"
#include "iconprovider.h"
#include "browserwindow.h"
#include "networkmanager.h"
#include "checkboxdialog.h"
#include "profilemanager.h"
#include "adblockmanager.h"
#include "restoremanager.h"
#include "browsinglibrary.h"
#include "downloadmanager.h"
#include "clearprivatedata.h"
#include "useragentmanager.h"
#include "commandlineoptions.h"
#include "webhistoryinterface.h"
#include "searchenginesmanager.h"
#include "desktopnotificationsfactory.h"
#include "html5permissions/html5permissionsmanager.h"

#include <QNetworkDiskCache>
#include <QDesktopServices>
#include <QFontDatabase>
#include <QSqlDatabase>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QTranslator>
#include <QThreadPool>
#include <QSettings>
#include <QProcess>
#include <QTimer>
#include <QDir>

#if QT_VERSION < 0x050000
#include "qwebkitversion.h"
#else
#include <QStandardPaths>
#endif

#if defined(Q_OS_WIN) && !defined(Q_OS_OS2)
#include "registerqappassociation.h"
#endif

MainApplication::MainApplication(int &argc, char** argv)
    : QtSingleApplication(argc, argv)
    , m_isPrivate(false)
    , m_isPortable(false)
    , m_isClosing(false)
    , m_isRestoring(false)
    , m_isStartingAfterCrash(false)
    , m_history(0)
    , m_bookmarks(0)
    , m_autoFill(0)
    , m_cookieJar(0)
    , m_plugins(0)
    , m_networkCache(0)
    , m_browsingLibrary(0)
    , m_rssManager(0)
    , m_networkManager(0)
    , m_restoreManager(0)
    , m_downloadManager(0)
    , m_userAgentManager(0)
    , m_searchEnginesManager(0)
    , m_html5PermissionsManager(0)
    , m_desktopNotifications(0)
    , m_autoSaver(0)
    , m_proxyStyle(0)
    , m_lastActiveWindow(0)
#if defined(Q_OS_WIN) && !defined(Q_OS_OS2)
    , m_registerQAppAssociation(0)
#endif
{
#if QT_VERSION >= 0x050000
    setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

    setApplicationName(QLatin1String("QupZilla"));
    setApplicationVersion(Qz::VERSION);
    setOrganizationDomain(QLatin1String("qupzilla"));
    setWindowIcon(QIcon::fromTheme(QSL("qupzilla"), QIcon(QSL(":icons/exeicons/qupzilla-window.png"))));

    // Set fallback icon theme (eg. on Windows/Mac)
    if (QIcon::fromTheme(QSL("view-refresh")).isNull()) {
        QIcon::setThemeSearchPaths(QStringList() << QL1S(":/oxygen-fallback"));
        QIcon::setThemeName(QSL("oxygen-fallback"));
    }

    // QSQLITE database plugin is required
    if (!QSqlDatabase::isDriverAvailable(QSL("QSQLITE"))) {
        QMessageBox::critical(0, QSL("Error"), QSL("Qt SQLite database plugin is not available. Please install it and restart the application."));
        m_isClosing = true;
        return;
    }

#ifdef Q_OS_WIN
    // Set default app font (needed for N'ko)
    int fontId = QFontDatabase::addApplicationFont(QSL("font.ttf"));
    if (fontId != -1) {
        const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.empty())
            setFont(QFont(families.first()));
    }
#endif

    QUrl startUrl;
    QString startProfile;
    QStringList messages;

    bool noAddons = false;
    bool newInstance = false;

    if (argc > 1) {
        CommandLineOptions cmd(argc);

        foreach (const CommandLineOptions::ActionPair &pair, cmd.getActions()) {
            switch (pair.action) {
            case Qz::CL_StartWithoutAddons:
                noAddons = true;
                break;
            case Qz::CL_StartWithProfile:
                startProfile = pair.text;
                break;
            case Qz::CL_StartPortable:
                m_isPortable = true;
                break;
            case Qz::CL_NewTab:
                messages.append(QLatin1String("ACTION:NewTab"));
                m_postLaunchActions.append(OpenNewTab);
                break;
            case Qz::CL_NewWindow:
                messages.append(QLatin1String("ACTION:NewWindow"));
                break;
            case Qz::CL_ToggleFullScreen:
                messages.append(QLatin1String("ACTION:ToggleFullScreen"));
                m_postLaunchActions.append(ToggleFullScreen);
                break;
            case Qz::CL_ShowDownloadManager:
                messages.append(QLatin1String("ACTION:ShowDownloadManager"));
                m_postLaunchActions.append(OpenDownloadManager);
                break;
            case Qz::CL_StartPrivateBrowsing:
                m_isPrivate = true;
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

    if (isPortable()) {
        std::cout << "QupZilla: Running in Portable Mode." << std::endl;
        DataPaths::setPortableVersion();
    }

    // Don't start single application in private browsing
    if (!isPrivate()) {
        QString appId = QLatin1String("QupZillaWebBrowser");

        if (isPortable()) {
            appId.append(QLatin1String("Portable"));
        }

        if (newInstance) {
            if (startProfile.isEmpty() || startProfile == QLatin1String("default")) {
                std::cout << "New instance cannot be started with default profile!" << std::endl;
            }
            else {
                // Generate unique appId so it is possible to start more separate instances
                // of the same profile. It is dangerous to run more instances of the same profile,
                // but if the user wants it, we should allow it.
                appId.append(startProfile + QString::number(QDateTime::currentMSecsSinceEpoch()));
            }
        }

        setAppId(appId);
    }

    // If there is nothing to tell other instance, we need to at least wake it
    if (messages.isEmpty()) {
        messages.append(QLatin1String(" "));
    }

    if (isRunning()) {
        m_isClosing = true;
        foreach (const QString &message, messages) {
            sendMessage(message);
        }
        return;
    }

#ifdef Q_OS_MAC
    setQuitOnLastWindowClosed(false);
#else
    setQuitOnLastWindowClosed(true);
#endif

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QDesktopServices::setUrlHandler("http", this, "addNewTab");
    QDesktopServices::setUrlHandler("ftp", this, "addNewTab");

    ProfileManager profileManager;
    profileManager.initConfigDir();
    profileManager.initCurrentProfile(startProfile);

    Settings::createSettings(DataPaths::currentProfilePath() + QLatin1String("/settings.ini"));

    m_autoSaver = new AutoSaver(this);
    connect(m_autoSaver, SIGNAL(save()), this, SLOT(saveSession()));

    translateApp();
    loadSettings();

    m_plugins = new PluginProxy;

    if (!noAddons) {
        m_plugins->loadPlugins();
    }

    BrowserWindow* window = createWindow(Qz::BW_FirstAppWindow, startUrl);
    connect(window, SIGNAL(startingCompleted()), this, SLOT(restoreOverrideCursor()));

    connect(this, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(onFocusChanged()));


    if (!isPrivate()) {
        Settings settings;
        m_isStartingAfterCrash = settings.value("SessionRestore/isRunning", false).toBool();
        int afterLaunch = settings.value("Web-URL-Settings/afterLaunch", 3).toInt();
        settings.setValue("SessionRestore/isRunning", true);

#ifndef DISABLE_UPDATES_CHECK
        bool checkUpdates = settings.value("Web-Browser-Settings/CheckUpdates", DEFAULT_CHECK_UPDATES).toBool();

        if (checkUpdates) {
            new Updater(window);
        }
#endif

        backupSavedSessions();

        if (m_isStartingAfterCrash || afterLaunch == 3) {
            m_restoreManager = new RestoreManager();
            if (!m_restoreManager->isValid()) {
                destroyRestoreManager();
            } else {
                // Pinned tabs are saved into session.dat, so remove the old saved pinned tabs
                QFile::remove(DataPaths::currentProfilePath() + QL1S("/pinnedtabs.dat"));
            }
        }
    }

    QTimer::singleShot(0, this, SLOT(postLaunch()));
}

MainApplication::~MainApplication()
{
    IconProvider::instance()->saveIconsToDatabase();

    // Wait for all QtConcurrent jobs to finish
    QThreadPool::globalInstance()->waitForDone();

    // Delete all classes that are saving data in destructor
    delete m_bookmarks;
    delete m_cookieJar;
    delete m_plugins;

    Settings::syncSettings();
}

bool MainApplication::isClosing() const
{
    return m_isClosing;
}

bool MainApplication::isRestoring() const
{
    return m_isRestoring;
}

bool MainApplication::isPrivate() const
{
    return m_isPrivate;
}

bool MainApplication::isPortable() const
{
#ifdef PORTABLE_BUILD
    return true;
#else
    return m_isPortable;
#endif
}

bool MainApplication::isStartingAfterCrash() const
{
    return m_isStartingAfterCrash;
}

int MainApplication::windowCount() const
{
    return m_windows.count();
}

QList<BrowserWindow*> MainApplication::windows() const
{
    return m_windows;
}

BrowserWindow* MainApplication::getWindow() const
{
    if (m_lastActiveWindow) {
        return m_lastActiveWindow;
    }

    return m_windows.isEmpty() ? 0 : m_windows.first();
}

BrowserWindow* MainApplication::createWindow(Qz::BrowserWindowType type, const QUrl &startUrl)
{
    if (windowCount() == 0 && type != Qz::BW_MacFirstWindow) {
        type = Qz::BW_FirstAppWindow;
    }

    BrowserWindow* window = new BrowserWindow(type, startUrl);
    connect(window, SIGNAL(destroyed(QObject*)), this, SLOT(windowDestroyed(QObject*)));

    m_windows.prepend(window);
    return window;
}

bool MainApplication::restoreSession(BrowserWindow* window, RestoreData restoreData)
{
    if (m_isPrivate || restoreData.isEmpty()) {
        return false;
    }

    m_isRestoring = true;
    setOverrideCursor(Qt::BusyCursor);

    window->setUpdatesEnabled(false);
    window->tabWidget()->closeRecoveryTab();

    if (window->tabWidget()->normalTabsCount() > 1) {
        // This can only happen when recovering crashed session!
        //
        // Don't restore tabs in current window as user already opened
        // some new tabs.
        // Instead create new one and restore pinned tabs there
        BrowserWindow* newWin = createWindow(Qz::BW_OtherRestoredWindow);
        newWin->setUpdatesEnabled(false);
        newWin->restoreWindowState(restoreData.first());
        newWin->setUpdatesEnabled(true);
        restoreData.remove(0);
    }
    else {
        // QTabWidget::count() - count of tabs is not updated after closing
        // recovery tab ...
        // update: it seems with ComboTabBar QTabWidget::count() is updated,
        // we add pinnedTabCounts to currentTab!
        int tabCount = window->tabWidget()->pinnedTabsCount();
        RestoreManager::WindowData data = restoreData.first();
        data.currentTab += tabCount;
        restoreData.remove(0);
        window->restoreWindowState(data);
    }

    window->setUpdatesEnabled(true);

    processEvents();

    foreach (const RestoreManager::WindowData &data, restoreData) {
        BrowserWindow* window = createWindow(Qz::BW_OtherRestoredWindow);
        window->setUpdatesEnabled(false);
        window->restoreWindowState(data);
        window->setUpdatesEnabled(true);

        processEvents();
    }

    destroyRestoreManager();
    restoreOverrideCursor();
    m_isRestoring = false;

    return true;
}

void MainApplication::destroyRestoreManager()
{
    delete m_restoreManager;
    m_restoreManager = 0;
}

void MainApplication::reloadSettings()
{
    loadSettings();
    emit settingsReloaded();
}

ProxyStyle* MainApplication::proxyStyle() const
{
    return m_proxyStyle;
}

void MainApplication::setProxyStyle(ProxyStyle* style)
{
    m_proxyStyle = style;

    QApplication::setStyle(style);
}

QString MainApplication::styleName() const
{
    if (m_proxyStyle && m_proxyStyle->baseStyle())
        return m_proxyStyle->baseStyle()->objectName();

    return style()->objectName();
}

QString MainApplication::currentLanguageFile() const
{
    return m_languageFile;
}

QString MainApplication::currentLanguage() const
{
    QString lang = m_languageFile;

    if (lang.isEmpty()) {
        return "en_US";
    }

    return lang.left(lang.length() - 3);
}

History* MainApplication::history()
{
    if (!m_history) {
        m_history = new History(this);
    }
    return m_history;
}

Bookmarks* MainApplication::bookmarks()
{
    if (!m_bookmarks) {
        m_bookmarks = new Bookmarks(this);
    }
    return m_bookmarks;
}

AutoFill* MainApplication::autoFill()
{
    if (!m_autoFill) {
        m_autoFill = new AutoFill(this);
    }
    return m_autoFill;
}

CookieJar* MainApplication::cookieJar()
{
    if (!m_cookieJar) {
        m_cookieJar = new CookieJar(this);
    }
    return m_cookieJar;
}

PluginProxy* MainApplication::plugins()
{
    return m_plugins;
}

QNetworkDiskCache* MainApplication::networkCache()
{
    if (!m_networkCache) {
        Settings settings;
        const QString defaultBasePath = QString("%1/networkcache/").arg(DataPaths::currentProfilePath());
        const QString basePath = settings.value("Web-Browser-Settings/CachePath", defaultBasePath).toString();
        const QString cachePath = QString("%1/%2-Qt%3/").arg(basePath, qWebKitVersion(), qVersion());

        m_networkCache = new QNetworkDiskCache(this);
        m_networkCache->setCacheDirectory(cachePath);
    }

    return m_networkCache;
}

BrowsingLibrary* MainApplication::browsingLibrary()
{
    if (!m_browsingLibrary) {
        m_browsingLibrary = new BrowsingLibrary(getWindow());
    }
    return m_browsingLibrary;
}

RSSManager* MainApplication::rssManager()
{
    if (!m_rssManager) {
        m_rssManager = new RSSManager(getWindow());
    }
    return m_rssManager;
}

NetworkManager* MainApplication::networkManager()
{
    if (!m_networkManager) {
        m_networkManager = new NetworkManager(this);
    }
    return m_networkManager;
}

RestoreManager* MainApplication::restoreManager()
{
    return m_restoreManager;
}

DownloadManager* MainApplication::downloadManager()
{
    if (!m_downloadManager) {
        m_downloadManager = new DownloadManager();
    }
    return m_downloadManager;
}

UserAgentManager* MainApplication::userAgentManager()
{
    if (!m_userAgentManager) {
        m_userAgentManager = new UserAgentManager(this);
    }
    return m_userAgentManager;
}

SearchEnginesManager* MainApplication::searchEnginesManager()
{
    if (!m_searchEnginesManager) {
        m_searchEnginesManager = new SearchEnginesManager(this);
    }
    return m_searchEnginesManager;
}

HTML5PermissionsManager* MainApplication::html5PermissionsManager()
{
    if (!m_html5PermissionsManager) {
        m_html5PermissionsManager = new HTML5PermissionsManager(this);
    }
    return m_html5PermissionsManager;
}

DesktopNotificationsFactory* MainApplication::desktopNotifications()
{
    if (!m_desktopNotifications) {
        m_desktopNotifications = new DesktopNotificationsFactory(this);
    }
    return m_desktopNotifications;
}

// static
MainApplication* MainApplication::instance()
{
    return static_cast<MainApplication*>(QCoreApplication::instance());
}

void MainApplication::addNewTab(const QUrl &url)
{
    BrowserWindow* window = getWindow();

    if (window) {
        window->tabWidget()->addView(url, url.isEmpty() ? Qz::NT_SelectedNewEmptyTab : Qz::NT_SelectedTabAtTheEnd);
    }
}

void MainApplication::startPrivateBrowsing(const QUrl &startUrl)
{
    QUrl url = startUrl;
    if (QAction* act = qobject_cast<QAction*>(sender())) {
        url = act->data().toUrl();
    }

    QStringList args;
    foreach (const QString &arg, arguments()) {
        if (arg.startsWith(QLatin1Char('-')) && arg != QLatin1String("--private-browsing") && arg != QLatin1String("-pb")) {
            args.append(arg);
        }
    }

    args.append(QLatin1String("--private-browsing"));

    if (!url.isEmpty()) {
        args << url.toEncoded();
    }

    if (!QProcess::startDetached(applicationFilePath(), args)) {
        qWarning() << "MainApplication: Cannot start new browser process for private browsing!" << applicationFilePath() << args;
    }
}

void MainApplication::reloadUserStyleSheet()
{
    const QUrl userCss = userStyleSheet(Settings().value("Web-Browser-Settings/userStyleSheet", QString()).toString());
    QWebSettings::globalSettings()->setUserStyleSheetUrl(userCss);
}

void MainApplication::restoreOverrideCursor()
{
    QApplication::restoreOverrideCursor();
}

void MainApplication::changeOcurred()
{
    m_autoSaver->changeOcurred();
}

void MainApplication::quitApplication()
{
    if (m_downloadManager && !m_downloadManager->canClose()) {
        m_downloadManager->show();
        return;
    }

    if (m_windows.count() > 0) {
        m_autoSaver->saveIfNecessary();
    }

    m_isClosing = true;

    // Saving settings in saveSettings() slot called from quit() so
    // everything gets saved also when quitting application in other
    // way than clicking Quit action in File menu or closing last window
    // eg. on Mac (#157)

    if (!isPrivate()) {
        removeLockFile();
    }

    quit();
}

void MainApplication::postLaunch()
{
    if (m_postLaunchActions.contains(OpenDownloadManager)) {
        downloadManager()->show();
    }

    if (m_postLaunchActions.contains(OpenNewTab)) {
        getWindow()->tabWidget()->addView(QUrl(), Qz::NT_SelectedNewEmptyTab);
    }

    if (m_postLaunchActions.contains(ToggleFullScreen)) {
        getWindow()->toggleFullScreen();
    }

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, DataPaths::currentProfilePath());
    QWebHistoryInterface::setDefaultInterface(new WebHistoryInterface(this));

    connect(this, SIGNAL(messageReceived(QString)), this, SLOT(messageReceived(QString)));
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));

    checkDefaultWebBrowser();
    QtWin::createJumpList();
}

void MainApplication::saveSession()
{
    if (m_isPrivate || m_isRestoring || m_windows.count() == 0 || m_restoreManager) {
        return;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << Qz::sessionVersion;
    stream << m_windows.count();

    foreach (BrowserWindow* w, m_windows) {
        stream << w->tabWidget()->saveState();
        if (w->isFullScreen()) {
            stream << QByteArray();
        }
        else {
            stream << w->saveState();
        }
    }

    int afterLaunch = Settings().value("Web-URL-Settings/afterLaunch", 3).toInt();

    if (afterLaunch != 3) {
        // Pinned tabs are saved only for last window into pinnedtabs.dat
        BrowserWindow* qupzilla_ = getWindow();
        if (qupzilla_ && m_windows.count() == 1) {
            qupzilla_->tabWidget()->savePinnedTabs();
        }
    }

    QFile file(DataPaths::currentProfilePath() + QLatin1String("/session.dat"));
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
}

void MainApplication::saveSettings()
{
    if (isPrivate()) {
        return;
    }

    m_isClosing = true;

    Settings settings;
    settings.beginGroup("SessionRestore");
    settings.setValue("isRunning", false);
    settings.endGroup();

    settings.beginGroup("Web-Browser-Settings");
    bool deleteHistory = settings.value("deleteHistoryOnClose", false).toBool();
    bool deleteHtml5Storage = settings.value("deleteHTML5StorageOnClose", false).toBool();
    settings.endGroup();

    if (deleteHistory) {
        m_history->clearHistory();
    }
    if (deleteHtml5Storage) {
        ClearPrivateData::clearLocalStorage();
    }

    m_searchEnginesManager->saveSettings();
    m_networkManager->saveSettings();
    m_plugins->shutdown();

    DataPaths::clearTempData();

    qzSettings->saveSettings();
    AdBlockManager::instance()->save();
    QFile::remove(DataPaths::currentProfilePath() + QLatin1String("/WebpageIcons.db"));
}

void MainApplication::messageReceived(const QString &message)
{
    QWidget* actWin = getWindow();
    QUrl actUrl;

    if (message.startsWith(QLatin1String("URL:"))) {
        const QUrl url = QUrl::fromUserInput(message.mid(4));
        addNewTab(url);
        actWin = getWindow();
    }
    else if (message.startsWith(QLatin1String("ACTION:"))) {
        const QString text = message.mid(7);
        if (text == QLatin1String("NewTab")) {
            addNewTab();
        }
        else if (text == QLatin1String("NewWindow")) {
            actWin = createWindow(Qz::BW_NewWindow);
        }
        else if (text == QLatin1String("ShowDownloadManager")) {
            downloadManager()->show();
            actWin = downloadManager();
        }
        else if (text == QLatin1String("ToggleFullScreen") && actWin) {
            BrowserWindow* qz = static_cast<BrowserWindow*>(actWin);
            qz->toggleFullScreen();
        }
        else if (text.startsWith(QLatin1String("OpenUrlInCurrentTab"))) {
            actUrl = QUrl::fromUserInput(text.mid(19));
        }
        else if (text.startsWith(QLatin1String("OpenUrlInNewWindow"))) {
            createWindow(Qz::BW_NewWindow, QUrl::fromUserInput(text.mid(18)));
            return;
        }
    }
    else {
        // User attempted to start another instance, let's open a new window
        actWin = createWindow(Qz::BW_NewWindow);
    }

    if (!actWin) {
        if (!isClosing()) {
            // It can only occur if download manager window was still opened
            createWindow(Qz::BW_NewWindow, actUrl);
        }
        return;
    }

    actWin->setWindowState(actWin->windowState() & ~Qt::WindowMinimized);
    actWin->raise();
    actWin->activateWindow();
    actWin->setFocus();

    BrowserWindow* win = qobject_cast<BrowserWindow*>(actWin);

    if (win && !actUrl.isEmpty()) {
        win->loadAddress(actUrl);
    }
}

void MainApplication::windowDestroyed(QObject* window)
{
    // qobject_cast doesn't work because QObject::destroyed is emitted from destructor
    Q_ASSERT(static_cast<BrowserWindow*>(window));
    Q_ASSERT(m_windows.contains(static_cast<BrowserWindow*>(window)));

    m_windows.removeOne(static_cast<BrowserWindow*>(window));
}

void MainApplication::onFocusChanged()
{
    BrowserWindow* activeBrowserWindow = qobject_cast<BrowserWindow*>(activeWindow());

    if (activeBrowserWindow) {
        m_lastActiveWindow = activeBrowserWindow;
    }
}

void MainApplication::loadSettings()
{
    Settings settings;
    settings.beginGroup("Themes");
    QString activeTheme = settings.value("activeTheme", DEFAULT_THEME_NAME).toString();
    settings.endGroup();

    loadTheme(activeTheme);

    QWebSettings* webSettings = QWebSettings::globalSettings();

    // Web browsing settings
    settings.beginGroup("Web-Browser-Settings");

    if (!m_isPrivate) {
        webSettings->enablePersistentStorage(DataPaths::currentProfilePath());
        webSettings->setAttribute(QWebSettings::LocalStorageEnabled, settings.value("HTML5StorageEnabled", true).toBool());
    }

    webSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    webSettings->setAttribute(QWebSettings::PluginsEnabled, settings.value("allowFlash", true).toBool());
    webSettings->setAttribute(QWebSettings::JavascriptEnabled, settings.value("allowJavaScript", true).toBool());
    webSettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, settings.value("allowJavaScriptOpenWindow", false).toBool());
    webSettings->setAttribute(QWebSettings::JavaEnabled, settings.value("allowJava", true).toBool());
    webSettings->setAttribute(QWebSettings::DnsPrefetchEnabled, settings.value("DNS-Prefetch", false).toBool());
    webSettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, settings.value("allowJavaScriptAccessClipboard", true).toBool());
    webSettings->setAttribute(QWebSettings::LinksIncludedInFocusChain, settings.value("IncludeLinkInFocusChain", false).toBool());
    webSettings->setAttribute(QWebSettings::ZoomTextOnly, settings.value("zoomTextOnly", false).toBool());
    webSettings->setAttribute(QWebSettings::PrintElementBackgrounds, settings.value("PrintElementBackground", true).toBool());
    webSettings->setAttribute(QWebSettings::XSSAuditingEnabled, settings.value("XSSAuditing", false).toBool());
    webSettings->setMaximumPagesInCache(settings.value("maximumCachedPages", 3).toInt());
    webSettings->setDefaultTextEncoding(settings.value("DefaultEncoding", webSettings->defaultTextEncoding()).toString());
    webSettings->setAttribute(QWebSettings::SpatialNavigationEnabled, settings.value("SpatialNavigation", false).toBool());

#if QTWEBKIT_FROM_2_3
    webSettings->setAttribute(QWebSettings::CaretBrowsingEnabled, settings.value("CaretBrowsing", false).toBool());
    webSettings->setAttribute(QWebSettings::ScrollAnimatorEnabled, settings.value("AnimateScrolling", true).toBool());
#endif

#ifdef USE_WEBGL
    webSettings->setAttribute(QWebSettings::WebGLEnabled, true);
    webSettings->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);
#endif

#if QTWEBKIT_FROM_2_2
    webSettings->setAttribute(QWebSettings::HyperlinkAuditingEnabled, true);
    webSettings->setAttribute(QWebSettings::JavascriptCanCloseWindows, settings.value("allowJavaScriptCloseWindow", false).toBool());
#endif

    setWheelScrollLines(settings.value("wheelScrollLines", wheelScrollLines()).toInt());
    const QString userCss = settings.value("userStyleSheet", QString()).toString();
    settings.endGroup();

    webSettings->setUserStyleSheetUrl(userStyleSheet(userCss));

    settings.beginGroup("Browser-Fonts");
    webSettings->setFontFamily(QWebSettings::StandardFont, settings.value("StandardFont", webSettings->fontFamily(QWebSettings::StandardFont)).toString());
    webSettings->setFontFamily(QWebSettings::CursiveFont, settings.value("CursiveFont", webSettings->fontFamily(QWebSettings::CursiveFont)).toString());
    webSettings->setFontFamily(QWebSettings::FantasyFont, settings.value("FantasyFont", webSettings->fontFamily(QWebSettings::FantasyFont)).toString());
    webSettings->setFontFamily(QWebSettings::FixedFont, settings.value("FixedFont", webSettings->fontFamily(QWebSettings::FixedFont)).toString());
    webSettings->setFontFamily(QWebSettings::SansSerifFont, settings.value("SansSerifFont", webSettings->fontFamily(QWebSettings::SansSerifFont)).toString());
    webSettings->setFontFamily(QWebSettings::SerifFont, settings.value("SerifFont", webSettings->fontFamily(QWebSettings::SerifFont)).toString());
    webSettings->setFontSize(QWebSettings::DefaultFontSize, settings.value("DefaultFontSize", 15).toInt());
    webSettings->setFontSize(QWebSettings::DefaultFixedFontSize, settings.value("FixedFontSize", 14).toInt());
    webSettings->setFontSize(QWebSettings::MinimumFontSize, settings.value("MinimumFontSize", 3).toInt());
    webSettings->setFontSize(QWebSettings::MinimumLogicalFontSize, settings.value("MinimumLogicalFontSize", 5).toInt());
    settings.endGroup();

    webSettings->setWebGraphic(QWebSettings::DefaultFrameIconGraphic, IconProvider::emptyWebIcon().pixmap(16, 16));
    webSettings->setWebGraphic(QWebSettings::MissingImageGraphic, QPixmap());

    if (isPrivate()) {
        webSettings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
        history()->setSaving(false);
    }

    if (m_downloadManager) {
        m_downloadManager->loadSettings();
    }

    qzSettings->loadSettings();
    userAgentManager()->loadSettings();
}

void MainApplication::loadTheme(const QString &name)
{
    QString activeThemePath;
    const QStringList themePaths = DataPaths::allPaths(DataPaths::Themes);

    foreach (const QString &path, themePaths) {
        const QString theme = QString("%1/%2").arg(path, name);
        if (QFile::exists(theme + QLatin1String("/main.css"))) {
            activeThemePath = theme;
            break;
        }
    }

    if (activeThemePath.isEmpty()) {
        qWarning() << "Cannot load theme " << name;
        activeThemePath = QString("%1/%2").arg(DataPaths::path(DataPaths::Themes), DEFAULT_THEME_NAME);
    }

    QString qss = QzTools::readAllFileContents(activeThemePath + QLatin1String("/main.css"));

#if defined(Q_OS_MAC)
    qss.append(QzTools::readAllFileContents(activeThemePath + QLatin1String("/mac.css")));
#elif defined(Q_OS_UNIX)
    qss.append(QzTools::readAllFileContents(activeThemePath + QLatin1String("/linux.css")));
#elif defined(Q_OS_WIN) || defined(Q_OS_OS2)
    qss.append(QzTools::readAllFileContents(activeThemePath + QLatin1String("/windows.css")));
#endif

    if (isRightToLeft()) {
        qss.append(QzTools::readAllFileContents(activeThemePath + QLatin1String("/rtl.css")));
    }

    QString relativePath = QDir::current().relativeFilePath(activeThemePath);
    qss.replace(QzRegExp(QSL("url\\s*\\(\\s*([^\\*:\\);]+)\\s*\\)"), Qt::CaseSensitive), QString("url(%1/\\1)").arg(relativePath));
    setStyleSheet(qss);
}

void MainApplication::translateApp()
{
    QString file = Settings().value(QSL("Language/language"), QLocale::system().name()).toString();

    // It can only be "C" locale, for which we will use default English language
    if (file.size() < 2)
        file.clear();

    if (!file.isEmpty() && !file.endsWith(QL1S(".qm")))
        file.append(QL1S(".qm"));

    // Either we load default language (with empty file), or we attempt to load xx.qm (xx_yy.qm)
    Q_ASSERT(file.isEmpty() || file.size() >= 5);

    QString translationPath = DataPaths::path(DataPaths::Translations);

    if (!file.isEmpty()) {
        const QStringList translationsPaths = DataPaths::allPaths(DataPaths::Translations);

        foreach (const QString &path, translationsPaths) {
            // If "xx_yy" translation doesn't exists, try to use "xx*" translation
            // It can only happen when language is chosen from system locale

            if (!QFile(QString("%1/%2").arg(path, file)).exists()) {
                QDir dir(path);
                QString lang = file.left(2) + QL1S("*.qm");

                const QStringList translations = dir.entryList(QStringList(lang));

                // If no translation can be found, default English will be used
                file = translations.isEmpty() ? QString() : translations.first();
            }

            if (!file.isEmpty() && QFile(QString("%1/%2").arg(path, file)).exists()) {
                translationPath = path;
                break;
            }
        }
    }

    // Load application translation
    QTranslator* app = new QTranslator(this);
    app->load(file, translationPath);

    // Load Qt translation (first try to load from Qt path)
    QTranslator* sys = new QTranslator(this);
    sys->load(QL1S("qt_") + file, QLibraryInfo::location(QLibraryInfo::TranslationsPath));

    // If there is no translation in Qt path for specified language, try to load it from our path
    if (sys->isEmpty()) {
        sys->load(QL1S("qt_") + file, translationPath);
    }

    m_languageFile = file;

    installTranslator(app);
    installTranslator(sys);
}

void MainApplication::backupSavedSessions()
{
    // session.dat      - current
    // session.dat.old  - first backup
    // session.dat.old1 - second backup

    const QString sessionFile = DataPaths::currentProfilePath() + QLatin1String("/session.dat");

    if (!QFile::exists(sessionFile)) {
        return;
    }

    if (QFile::exists(sessionFile + QLatin1String(".old"))) {
        QFile::remove(sessionFile + QLatin1String(".old1"));
        QFile::copy(sessionFile + QLatin1String(".old"), sessionFile + QLatin1String(".old1"));
    }

    QFile::remove(sessionFile + QLatin1String(".old"));
    QFile::copy(sessionFile, sessionFile + QLatin1String(".old"));
}

void MainApplication::checkDefaultWebBrowser()
{
    if (isPortable()) {
        return;
    }

#if defined(Q_OS_WIN) && !defined(Q_OS_OS2)
    Settings settings;
    bool checkNow = settings.value("Web-Browser-Settings/CheckDefaultBrowser", DEFAULT_CHECK_DEFAULTBROWSER).toBool();

    if (!checkNow) {
        return;
    }

    bool checkAgain = true;

    if (!associationManager()->isDefaultForAllCapabilities()) {
        CheckBoxDialog dialog(QDialogButtonBox::Yes | QDialogButtonBox::No, getWindow());
        dialog.setText(tr("QupZilla is not currently your default browser. Would you like to make it your default browser?"));
        dialog.setCheckBoxText(tr("Always perform this check when starting QupZilla."));
        dialog.setDefaultCheckState(Qt::Checked);
        dialog.setWindowTitle(tr("Default Browser"));
        dialog.setIcon(IconProvider::standardIcon(QStyle::SP_MessageBoxWarning));

        if (dialog.exec() == QDialog::Accepted) {
            associationManager()->registerAllAssociation();
        }

        checkAgain = dialog.isChecked();
    }

    settings.setValue("Web-Browser-Settings/CheckDefaultBrowser", checkAgain);
#endif
}

QUrl MainApplication::userStyleSheet(const QString &filePath) const
{
    QString userCss;

#if !defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    // Don't grey out selection on losing focus (to prevent graying out found text)
    QString highlightColor;
    QString highlightedTextColor;
#ifdef Q_OS_MAC
    highlightColor = QLatin1String("#b6d6fc");
    highlightedTextColor = QLatin1String("#000");
#else
    QPalette pal = style()->standardPalette();
    highlightColor = pal.color(QPalette::Highlight).name();
    highlightedTextColor = pal.color(QPalette::HighlightedText).name();
#endif
    userCss += QString("::selection {background: %1; color: %2;} ").arg(highlightColor, highlightedTextColor);
#endif

    userCss += AdBlockManager::instance()->elementHidingRules();
    userCss += QzTools::readAllFileContents(filePath).remove(QLatin1Char('\n'));

    const QString encodedStyle = userCss.toLatin1().toBase64();
    const QString dataString = QString("data:text/css;charset=utf-8;base64,%1").arg(encodedStyle);

    return QUrl(dataString);
}

#if defined(Q_OS_WIN) && !defined(Q_OS_OS2)
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

#ifdef Q_OS_MAC
#include <QFileOpenEvent>

bool MainApplication::event(QEvent* e)
{
    switch (e->type()) {
    case QEvent::FileOpen:
        addNewTab(QUrl::fromLocalFile(static_cast<QFileOpenEvent*>(e)->file()));
        return true;

    case QEvent::ApplicationActivate:
        if (m_windows.isEmpty())
            createWindow(Qz::BW_NewWindow);
        break;

    default:
        break;
    }

    return QtSingleApplication::event(e);
}
#endif
