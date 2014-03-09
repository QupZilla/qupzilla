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
#include "datapaths.h"
#include "browserwindow.h"
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
#include "bookmarks.h"
#include "downloadmanager.h"
#include "autofill.h"
#include "adblockmanager.h"
#include "desktopnotificationsfactory.h"
#include "iconprovider.h"
#include "qtwin.h"
#include "mainapplication.h"
#include "webhistoryinterface.h"
#include "qztools.h"
#include "profilemanager.h"
#include "searchenginesmanager.h"
#include "speeddial.h"
#include "webpage.h"
#include "settings.h"
#include "qzsettings.h"
#include "clearprivatedata.h"
#include "commandlineoptions.h"
#include "useragentmanager.h"
#include "restoremanager.h"
#include "proxystyle.h"
#include "qzregexp.h"
#include "checkboxdialog.h"
#include "registerqappassociation.h"
#include "html5permissions/html5permissionsmanager.h"

#ifdef Q_OS_MAC
#include "macmenureceiver.h"
#include <QFileOpenEvent>
#include <QMenu>
#endif

#include <QNetworkDiskCache>
#include <QDesktopServices>
#include <QSqlDatabase>
#include <QTranslator>
#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QDir>

#if QT_VERSION < 0x050000
#include "qwebkitversion.h"
#else
#include <QStandardPaths>
#endif

MainApplication::MainApplication(int &argc, char** argv)
    : QtSingleApplication(argc, argv)
    , m_autoSaver(0)
    , m_cookiemanager(0)
    , m_browsingLibrary(0)
    , m_history(0)
    , m_websettings(0)
    , m_networkmanager(0)
    , m_cookiejar(0)
    , m_rssmanager(0)
    , m_plugins(0)
    , m_bookmarks(0)
    , m_downloadManager(0)
    , m_autofill(0)
    , m_networkCache(0)
    , m_desktopNotifications(0)
    , m_searchEnginesManager(0)
    , m_restoreManager(0)
    , m_proxyStyle(0)
    , m_html5permissions(0)
    , m_uaManager(new UserAgentManager(this))
    , m_isPrivateSession(false)
    , m_isPortable(false)
    , m_isClosing(false)
    , m_isRestoring(false)
    , m_isStartingAfterCrash(false)
#if defined(Q_OS_WIN) && !defined(Q_OS_OS2)
    , m_registerQAppAssociation(0)
#endif
#ifdef Q_OS_MAC
    , m_macMenuReceiver(0)
    , m_macDockMenu(0)
#endif
{
    setApplicationName("QupZilla");
    setApplicationVersion(Qz::VERSION);
    setOrganizationDomain("qupzilla");
    setWindowIcon(QIcon(":icons/exeicons/qupzilla-window.png"));

    bool noAddons = false;
    bool newInstance = false;
    QUrl startUrl;
    QStringList messages;
    QString startProfile;

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

    if (isPortable()) {
        std::cout << "QupZilla: Running in Portable Mode." << std::endl;
        DataPaths::setPortableVersion();
    }

    // Don't start single application in private browsing
    if (!m_isPrivateSession) {
        QString appId = isPortable() ? "QupZillaWebBrowserPortable" : "QupZillaWebBrowser";

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
        foreach (const QString &message, messages) {
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

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QDesktopServices::setUrlHandler("http", this, "addNewTab");
    QDesktopServices::setUrlHandler("ftp", this, "addNewTab");

    ProfileManager profileManager;
    profileManager.initConfigDir();
    profileManager.initCurrentProfile(startProfile);

    Settings::createSettings(DataPaths::currentProfilePath() + QLatin1String("/settings.ini"));

    m_autoSaver = new AutoSaver(this);
    connect(m_autoSaver, SIGNAL(save()), this, SLOT(saveStateSlot()));

    translateApp();

    BrowserWindow* qupzilla = new BrowserWindow(Qz::BW_FirstAppWindow, startUrl);
    m_mainWindows.prepend(qupzilla);

    connect(qupzilla, SIGNAL(startingCompleted()), this, SLOT(restoreCursor()));

    loadSettings();

    m_plugins = new PluginProxy;

    if (!noAddons) {
        m_plugins->loadPlugins();
    }

    if (!m_isPrivateSession) {
        Settings settings;
        m_isStartingAfterCrash = settings.value("SessionRestore/isRunning", false).toBool();
        int afterLaunch = settings.value("Web-URL-Settings/afterLaunch", 3).toInt();
        settings.setValue("SessionRestore/isRunning", true);

#ifndef DISABLE_UPDATES_CHECK
        bool checkUpdates = settings.value("Web-Browser-Settings/CheckUpdates", DEFAULT_CHECK_UPDATES).toBool();

        if (checkUpdates) {
            new Updater(qupzilla);
        }
#endif

        backupSavedSessions();

        if (m_isStartingAfterCrash || afterLaunch == 3) {
            m_restoreManager = new RestoreManager();
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

MainApplication::~MainApplication()
{
    // Delete all classes that are saving data in destructor
    delete m_bookmarks;

#ifdef Q_OS_MAC
    delete m_macDockMenu;
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

    if (m_postLaunchActions.contains(ToggleFullScreen)) {
        getWindow()->toggleFullScreen();
    }

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, DataPaths::currentProfilePath());
    QWebHistoryInterface::setDefaultInterface(new WebHistoryInterface(this));

    connect(this, SIGNAL(messageReceived(QString)), this, SLOT(receiveAppMessage(QString)));
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));

    if (!isPortable()) {
        Settings settings;
        bool alwaysCheckDefaultBrowser = settings.value("Web-Browser-Settings/CheckDefaultBrowser", DEFAULT_CHECK_DEFAULTBROWSER).toBool();
        if (alwaysCheckDefaultBrowser) {
            alwaysCheckDefaultBrowser = checkDefaultWebBrowser();
            settings.setValue("Web-Browser-Settings/CheckDefaultBrowser", alwaysCheckDefaultBrowser);
        }
    }
}

void MainApplication::loadSettings()
{
    Settings settings;
    settings.beginGroup("Themes");
    QString activeTheme = settings.value("activeTheme", DEFAULT_THEME_NAME).toString();
    settings.endGroup();

    loadTheme(activeTheme);

    // Create global QWebSettings object
    webSettings();

    // Web browsing settings
    settings.beginGroup("Web-Browser-Settings");

    if (!m_isPrivateSession) {
        m_websettings->enablePersistentStorage(DataPaths::currentProfilePath());
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
    m_websettings->setAttribute(QWebSettings::SpatialNavigationEnabled, settings.value("SpatialNavigation", false).toBool());

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

    m_websettings->setWebGraphic(QWebSettings::DefaultFrameIconGraphic, IconProvider::emptyWebIcon().pixmap(16, 16));
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

void MainApplication::doReloadSettings()
{
    loadSettings();
    emit reloadSettings();
}

void MainApplication::restoreCursor()
{
    QApplication::restoreOverrideCursor();
}

void MainApplication::setupJumpList()
{
    QtWin::setupJumpList();
}

BrowserWindow* MainApplication::getWindow()
{
    BrowserWindow* activeW = qobject_cast<BrowserWindow*>(activeWindow());
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
    m_autoSaver->changeOcurred();
}

QList<BrowserWindow*> MainApplication::mainWindows()
{
    QList<BrowserWindow*> list;

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
    return m_mainWindows.count();
}

QString MainApplication::currentLanguageFile() const
{
    return m_currentLanguage;
}

QString MainApplication::currentLanguage() const
{
    QString lang = m_currentLanguage;

    if (lang.isEmpty()) {
        return "en_US";
    }

    return lang.left(lang.length() - 3);
}

void MainApplication::receiveAppMessage(QString message)
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
            actWin = makeNewWindow(Qz::BW_NewWindow);
        }
        else if (text == QLatin1String("ShowDownloadManager")) {
            downManager()->show();
            actWin = downManager();
        }
        else if (text == QLatin1String("ToggleFullScreen") && actWin) {
            BrowserWindow* qz = static_cast<BrowserWindow*>(actWin);
            qz->toggleFullScreen();
        }
        else if (text.startsWith(QLatin1String("OpenUrlInCurrentTab"))) {
            actUrl = QUrl::fromUserInput(text.mid(19));
        }
        else if (text.startsWith(QLatin1String("OpenUrlInNewWindow"))) {
            makeNewWindow(Qz::BW_NewWindow, QUrl::fromUserInput(text.mid(18)));
            return;
        }
    }

    if (!actWin) {
        if (!isClosing()) {
            // It can only occur if download manager window was still opened
            makeNewWindow(Qz::BW_NewWindow, actUrl);
        }
        return;
    }

    actWin->setWindowState(actWin->windowState() & ~Qt::WindowMinimized);
    actWin->raise();
    actWin->activateWindow();
    actWin->setFocus();

    BrowserWindow* qz = qobject_cast<BrowserWindow*>(actWin);

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

BrowserWindow* MainApplication::makeNewWindow(Qz::BrowserWindowType type, const QUrl &startUrl)
{
    if (m_mainWindows.count() == 0 && type != Qz::BW_MacFirstWindow) {
        type = Qz::BW_FirstAppWindow;
    }

    BrowserWindow* newWindow = new BrowserWindow(type, startUrl);
    m_mainWindows.prepend(newWindow);

    return newWindow;
}

#ifdef Q_OS_MAC
extern void qt_mac_set_dock_menu(QMenu* menu);

QMenu* MainApplication::macDockMenu()
{
    if (!m_macDockMenu) {
        m_macDockMenu = new QMenu(0);
        qt_mac_set_dock_menu(m_macDockMenu);
    }
    return m_macDockMenu;
}

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

void MainApplication::loadTheme(const QString &name)
{
    QString activeThemePath;
    const QStringList themePaths = DataPaths::allPaths(DataPaths::Themes);

    foreach (const QString &path, themePaths) {
        const QString theme = QString("%1/%2").arg(path, name);
        if (QFile::exists(theme + "/main.css")) {
            activeThemePath = theme;
            break;
        }
    }

    if (activeThemePath.isEmpty()) {
        qWarning() << "Cannot load theme " << name;
        activeThemePath = DataPaths::path(DataPaths::Themes) + DEFAULT_THEME_NAME;
    }

    QFile cssFile(activeThemePath + "/main.css");
    cssFile.open(QFile::ReadOnly);
    QString css = cssFile.readAll();
    cssFile.close();

    /*
     * #id[style=QtStyle] (QtStyle = QMacStyle, QWindowsVistaStyle, QGtkStyle, ...)
     * should be enough instead of loading special stylesheets
     */
#ifdef Q_OS_UNIX
    if (QFile(activeThemePath + "/linux.css").exists()) {
        cssFile.setFileName(activeThemePath + "/linux.css");
        cssFile.open(QFile::ReadOnly);
        css.append(cssFile.readAll());
        cssFile.close();
    }
#endif

#ifdef Q_OS_MAC
    if (QFile(activeThemePath + "/mac.css").exists()) {
        cssFile.setFileName(activeThemePath + "/mac.css");
        cssFile.open(QFile::ReadOnly);
        css.append(cssFile.readAll());
        cssFile.close();
    }
#endif

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
    if (QFile(activeThemePath + "/windows.css").exists()) {
        cssFile.setFileName(activeThemePath + "/windows.css");
        cssFile.open(QFile::ReadOnly);
        css.append(cssFile.readAll());
        cssFile.close();
    }
#endif

    // RTL Support
    // Loading 'rtl.css' when layout is right to left!
    if (isRightToLeft() && QFile(activeThemePath + "/rtl.css").exists()) {
        cssFile.setFileName(activeThemePath + "/rtl.css");
        cssFile.open(QFile::ReadOnly);
        css.append(cssFile.readAll());
        cssFile.close();
    }

    QString relativePath = QDir::current().relativeFilePath(activeThemePath);
    css.replace(QzRegExp("url\\s*\\(\\s*([^\\*:\\);]+)\\s*\\)", Qt::CaseSensitive),
                QString("url(%1\\1)").arg(relativePath + "/"));

    setStyleSheet(css);
}

void MainApplication::translateApp()
{
    Settings settings;
    QString file = settings.value("Language/language", QLocale::system().name()).toString();

    if (!file.isEmpty() && !file.endsWith(QLatin1String(".qm"))) {
        file.append(".qm");
    }

    QString dir = DataPaths::path(DataPaths::Translations);

    if (!file.isEmpty()) {
        const QStringList translationsPaths = DataPaths::allPaths(DataPaths::Translations);

        foreach (const QString &path, translationsPaths) {
            // If "xx_yy" translation doesn't exists, try to use "xx*" translation
            // It can only happen when Language is chosen from system locale

            if (!QFile(path + file).exists()) {
                QDir dir(path);
                QString lang = file.left(2) + QLatin1String("*.qm");

                const QStringList translations = dir.entryList(QStringList(lang));

                // If no translation can be found, default English will be used
                file = translations.isEmpty() ? QString() : translations.first();
            }

            if (QFile(path + file).exists()) {
                dir = path;
                break;
            }
        }
    }

    QTranslator* app = new QTranslator(this);
    app->load(file, dir);

    QTranslator* sys = new QTranslator(this);
    sys->load("qt_" + file, dir);

    m_currentLanguage = file;

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

    if (QFile::exists(sessionFile + ".old")) {
        QFile::remove(sessionFile + ".old1");
        QFile::copy(sessionFile + ".old", sessionFile + ".old1");
    }

    QFile::remove(sessionFile + ".old");
    QFile::copy(sessionFile, sessionFile + ".old");
}

void MainApplication::quitApplication()
{
    if (m_downloadManager && !m_downloadManager->canClose()) {
        m_downloadManager->show();
        return;
    }

    if (m_mainWindows.count() > 0) {
        m_autoSaver->saveIfNecessary();
    }

    m_isClosing = true;
    m_networkmanager->disconnectObjects();

    // Saving settings in saveSettings() slot called from quit() so
    // everything gets saved also when quitting application in other
    // way than clicking Quit action in File menu or closing last window
    //
    //  * this can occur on Mac OS (see #157)

    if (!m_isPrivateSession) {
        removeLockFile();
    }

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
        m_history->clearHistory();
    }
    if (deleteHtml5Storage) {
        ClearPrivateData::clearLocalStorage();
    }

    m_searchEnginesManager->saveSettings();
    m_networkmanager->saveSettings();
    m_plugins->shutdown();

    DataPaths::clearTempData();

    qzSettings->saveSettings();
    AdBlockManager::instance()->save();
    IconProvider::instance()->saveIconsToDatabase();
    QFile::remove(DataPaths::currentProfilePath() + QLatin1String("/WebpageIcons.db"));
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
    if (!m_history) {
        m_history = new History(this);
    }
    return m_history;
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
        m_networkmanager = new NetworkManager(this);
    }
    return m_networkmanager;
}

CookieJar* MainApplication::cookieJar()
{
    if (!m_cookiejar) {
        m_cookiejar = new CookieJar(this);
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

Bookmarks* MainApplication::bookmarks()
{
    if (!m_bookmarks) {
        m_bookmarks = new Bookmarks(this);
    }
    return m_bookmarks;
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
        m_autofill = new AutoFill(this);
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
        const QString defaultBasePath = QString("%1networkcache/").arg(DataPaths::currentProfilePath());
        const QString basePath = settings.value("Web-Browser-Settings/CachePath", defaultBasePath).toString();
        const QString cachePath = QString("%1/%2-Qt%3/").arg(basePath, qWebKitVersion(), qVersion());

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

MainApplication* MainApplication::instance()
{
    return static_cast<MainApplication*>(QCoreApplication::instance());
}

void MainApplication::startPrivateBrowsing(const QUrl &startUrl)
{
    const QUrl url = !startUrl.isEmpty() ? startUrl : qobject_cast<QAction*>(sender())->data().toUrl();

    QStringList args;
    foreach (const QString &arg, arguments()) {
        if (arg.startsWith(QLatin1Char('-')) &&
                arg != QLatin1String("--private-browsing") &&
                arg != QLatin1String("-pb")) {
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
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    m_websettings->setUserStyleSheetUrl(userStyleSheet(settings.value("userStyleSheet", QString()).toString()));
    settings.endGroup();
}

bool MainApplication::checkDefaultWebBrowser()
{
#if defined(Q_OS_WIN) && !defined(Q_OS_OS2)
    bool showAgain = true;
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

        showAgain = dialog.isChecked();
    }

    return showAgain;
#else
    return false;
#endif
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

QUrl MainApplication::userStyleSheet(const QString &filePath) const
{
    QString userStyle;

#ifndef Q_OS_UNIX
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
    userStyle += QString("::selection {background: %1; color: %2;} ").arg(highlightColor, highlightedTextColor);
#endif

    userStyle += AdBlockManager::instance()->elementHidingRules();

    QFile file(filePath);
    if (!filePath.isEmpty() && file.open(QFile::ReadOnly)) {
        QString fileData = QString::fromUtf8(file.readAll());
        fileData.remove(QLatin1Char('\n'));
        userStyle.append(fileData);
        file.close();
    }

    const QString encodedStyle = userStyle.toLatin1().toBase64();
    const QString dataString = QString("data:text/css;charset=utf-8;base64,%1").arg(encodedStyle);

    return QUrl(dataString);
}

void MainApplication::aboutToCloseWindow(BrowserWindow* window)
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

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << Qz::sessionVersion;
    stream << m_mainWindows.count();

    for (int i = 0; i < m_mainWindows.count(); i++) {
        BrowserWindow* qz = m_mainWindows.at(i).data();
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

    BrowserWindow* qupzilla_ = getWindow();
    if (qupzilla_ && m_mainWindows.count() == 1) {
        qupzilla_->tabWidget()->savePinnedTabs();
    }

    QFile file(DataPaths::currentProfilePath() + QLatin1String("/session.dat"));
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();

    return true;
}

bool MainApplication::restoreStateSlot(BrowserWindow* window, RestoreData recoveryData)
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

        BrowserWindow* newWin = makeNewWindow(Qz::BW_OtherRestoredWindow);
        newWin->restoreWindowState(recoveryData.first());
        recoveryData.remove(0);
    }
    else {
        // QTabWidget::count() - count of tabs is not updated after closing
        // recovery tab ...
        // update: it seems with ComboTabBar QTabWidget::count() is updated,
        // we add pinnedTabCounts to currentTab!
        int tabCount = window->tabWidget()->pinnedTabsCount();
        RestoreManager::WindowData data = recoveryData.first();
        data.currentTab += tabCount;
        recoveryData.remove(0);

        window->restoreWindowState(data);
    }

    foreach (const RestoreManager::WindowData &data, recoveryData) {
        BrowserWindow* window = makeNewWindow(Qz::BW_OtherRestoredWindow);
        window->restoreWindowState(data);
        // for correct geometry calculation in BrowserWindow::setupUi()
        mApp->processEvents();
    }

    destroyRestoreManager();
    m_isRestoring = false;

    return true;
}

void MainApplication::destroyRestoreManager()
{
    delete m_restoreManager;
    m_restoreManager = 0;
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

QString MainApplication::currentStyle() const
{
    return m_proxyStyle->baseStyle()->objectName();
}
