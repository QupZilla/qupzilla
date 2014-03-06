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
#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#define mApp MainApplication::getInstance()

#include <QList>
#include <QUrl>
#include <QPointer>

#include "restoremanager.h"
#include "qtsingleapplication.h"
#include "qzcommon.h"

class QMenu;
class QWebSettings;
class QNetworkDiskCache;

class BrowserWindow;
class CookieManager;
class BrowsingLibrary;
class History;
class NetworkManager;
class CookieJar;
class RSSManager;
class Updater;
class PluginProxy;
class Bookmarks;
class DownloadManager;
class AutoFill;
class DesktopNotificationsFactory;
class IconProvider;
class SearchEnginesManager;
class DatabaseWriter;
class UserAgentManager;
class ProxyStyle;
class RegisterQAppAssociation;
class HTML5PermissionsManager;
class AutoSaver;
class Speller;

#ifdef Q_OS_MAC
class MacMenuReceiver;
#endif

class QUPZILLA_EXPORT MainApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    QString DATADIR;
    QString PROFILEDIR;
    QString TRANSLATIONSDIR;
    QString THEMESDIR;

    explicit MainApplication(int &argc, char** argv);
    ~MainApplication();

    void connectDatabase();
    void reloadSettings();
    bool restoreStateSlot(BrowserWindow* window, RestoreData recoveryData);
    BrowserWindow* makeNewWindow(Qz::BrowserWindowType type, const QUrl &startUrl = QUrl());
    void aboutToCloseWindow(BrowserWindow* window);

    QList<BrowserWindow*> mainWindows();

    static MainApplication* getInstance() { return static_cast<MainApplication*>(QCoreApplication::instance()); }

    bool isClosing() const;
    bool isRestoring() const;
    bool isPrivateSession() const;
    bool isPortable() const;
    bool isStartingAfterCrash() const;
    int windowCount() const;
    QString currentLanguageFile() const;
    QString currentLanguage() const;
    QString currentProfilePath() const;

    bool checkSettingsDir();
    void destroyRestoreManager();
    void clearTempPath();

    ProxyStyle* proxyStyle() const;
    void setProxyStyle(ProxyStyle* style);

    QString currentStyle() const;
    QString tempPath() const;

    BrowserWindow* getWindow();
    CookieManager* cookieManager();
    BrowsingLibrary* browsingLibrary();
    History* history();
    QWebSettings* webSettings();
    NetworkManager* networkManager();
    CookieJar* cookieJar();
    RSSManager* rssManager();
    PluginProxy* plugins();
    Bookmarks* bookmarks();
    DownloadManager* downManager();
    AutoFill* autoFill();
    SearchEnginesManager* searchEnginesManager();
    QNetworkDiskCache* networkCache();
    DesktopNotificationsFactory* desktopNotifications();
    HTML5PermissionsManager* html5permissions();
#ifdef USE_HUNSPELL
    Speller* speller();
#endif

    DatabaseWriter* dbWriter() { return m_dbWriter; }
    UserAgentManager* uaManager() { return m_uaManager; }
    RestoreManager* restoreManager() { return m_restoreManager; }

#if defined(Q_OS_WIN) && !defined(Q_OS_OS2)
    RegisterQAppAssociation* associationManager();
#endif

#ifdef Q_OS_MAC
    MacMenuReceiver* macMenuReceiver();
    QMenu* macDockMenu();
    bool event(QEvent* e);
#endif

public slots:
    bool saveStateSlot();
    void quitApplication();
    void sendMessages(Qz::AppMessageType mes, bool state);
    void receiveAppMessage(QString message);
    void setStateChanged();
    void addNewTab(const QUrl &url = QUrl());

    void startPrivateBrowsing(const QUrl &startUrl = QUrl());
    void reloadUserStyleSheet();
    bool checkDefaultWebBrowser();

signals:
    void message(Qz::AppMessageType mes, bool state);

private slots:
    void loadSettings();
    void postLaunch();
    void setupJumpList();
    void restoreCursor();

    void saveSettings();

private:
    enum PostLaunchAction { OpenDownloadManager, OpenNewTab, ToggleFullScreen };

    void loadTheme(const QString &name);
    void translateApp();
    void restoreOtherWindows();
    void backupSavedSessions();

    QUrl userStyleSheet(const QString &filePath) const;

    AutoSaver* m_autoSaver;
    CookieManager* m_cookiemanager;
    BrowsingLibrary* m_browsingLibrary;
    History* m_history;
    QWebSettings* m_websettings;
    NetworkManager* m_networkmanager;
    CookieJar* m_cookiejar;
    RSSManager* m_rssmanager;
    PluginProxy* m_plugins;
    Bookmarks* m_bookmarks;
    DownloadManager* m_downloadManager;
    AutoFill* m_autofill;
    QNetworkDiskCache* m_networkCache;
    DesktopNotificationsFactory* m_desktopNotifications;
    SearchEnginesManager* m_searchEnginesManager;
    RestoreManager* m_restoreManager;
    ProxyStyle* m_proxyStyle;
    HTML5PermissionsManager* m_html5permissions;
#ifdef USE_HUNSPELL
    Speller* m_speller;
#endif
    DatabaseWriter* m_dbWriter;
    UserAgentManager* m_uaManager;
    QList<QPointer<BrowserWindow> > m_mainWindows;

    QString m_activeProfil;
    QString m_activeLanguage;
    QString m_activeThemePath;

    bool m_isPrivateSession;
    bool m_isPortable;
    bool m_isClosing;
    bool m_isRestoring;
    bool m_startingAfterCrash;

    bool m_databaseConnected;
    QList<PostLaunchAction> m_postLaunchActions;

#if defined(Q_OS_WIN) && !defined(Q_OS_OS2)
    RegisterQAppAssociation* m_registerQAppAssociation;
#endif

#ifdef Q_OS_MAC
    MacMenuReceiver* m_macMenuReceiver;
    QMenu* m_macDockMenu;
#endif
};

#endif // MAINAPPLICATION_H
