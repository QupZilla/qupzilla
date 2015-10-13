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

#define mApp MainApplication::instance()

#include <QList>
#include <QPointer>

#include "qtsingleapplication/qtsingleapplication.h"
#include "restoremanager.h"
#include "qzcommon.h"

class QMenu;
class QNetworkDiskCache;

class History;
class AutoFill;
class MainMenu;
class Bookmarks;
class CookieJar;
class AutoSaver;
class RSSManager;
class PluginProxy;
class BrowserWindow;
class NetworkManager;
class BrowsingLibrary;
class DownloadManager;
class UserAgentManager;
class SearchEnginesManager;
class HTML5PermissionsManager;
class RegisterQAppAssociation;
class DesktopNotificationsFactory;

class QUPZILLA_EXPORT MainApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    explicit MainApplication(int &argc, char** argv);
    ~MainApplication();

    bool isClosing() const;
    bool isRestoring() const;
    bool isPrivate() const;
    bool isPortable() const;
    bool isStartingAfterCrash() const;

    int windowCount() const;
    QList<BrowserWindow*> windows() const;

    BrowserWindow* getWindow() const;
    BrowserWindow* createWindow(Qz::BrowserWindowType type, const QUrl &startUrl = QUrl());

    bool restoreSession(BrowserWindow* window, RestoreData restoreData);
    void destroyRestoreManager();
    void reloadSettings();

    // Name of current Qt style
    QString styleName() const;

    // TODO: Move translating into own class (LocaleManager ?)
    QString currentLanguageFile() const;
    QString currentLanguage() const;

    History* history();
    Bookmarks* bookmarks();

    AutoFill* autoFill();
    CookieJar* cookieJar();
    PluginProxy* plugins();
    QNetworkDiskCache* networkCache();
    BrowsingLibrary* browsingLibrary();

    RSSManager* rssManager();
    NetworkManager* networkManager();
    RestoreManager* restoreManager();
    DownloadManager* downloadManager();
    UserAgentManager* userAgentManager();
    SearchEnginesManager* searchEnginesManager();
    HTML5PermissionsManager* html5PermissionsManager();
    DesktopNotificationsFactory* desktopNotifications();

    static MainApplication* instance();

public slots:
    void addNewTab(const QUrl &url = QUrl());
    void startPrivateBrowsing(const QUrl &startUrl = QUrl());

    void reloadUserStyleSheet();
    void restoreOverrideCursor();

    void changeOcurred();
    void quitApplication();

signals:
    void settingsReloaded();

private slots:
    void postLaunch();

    void saveSession();
    void saveSettings();

    void messageReceived(const QString &message);
    void windowDestroyed(QObject* window);
    void onFocusChanged();
    void checkDefaultWebBrowser();

private:
    enum PostLaunchAction {
        OpenDownloadManager,
        OpenNewTab,
        ToggleFullScreen
    };

    void loadSettings();
    void loadTheme(const QString &name);

    void translateApp();
    void backupSavedSessions();

    QUrl userStyleSheet(const QString &filePath) const;

    bool m_isPrivate;
    bool m_isPortable;
    bool m_isClosing;
    bool m_isRestoring;
    bool m_isStartingAfterCrash;

    History* m_history;
    Bookmarks* m_bookmarks;

    AutoFill* m_autoFill;
    CookieJar* m_cookieJar;
    PluginProxy* m_plugins;
    QNetworkDiskCache* m_networkCache;
    BrowsingLibrary* m_browsingLibrary;

    RSSManager* m_rssManager;
    NetworkManager* m_networkManager;
    RestoreManager* m_restoreManager;
    DownloadManager* m_downloadManager;
    UserAgentManager* m_userAgentManager;
    SearchEnginesManager* m_searchEnginesManager;
    HTML5PermissionsManager* m_html5PermissionsManager;
    DesktopNotificationsFactory* m_desktopNotifications;

    AutoSaver* m_autoSaver;

    QList<BrowserWindow*> m_windows;
    QPointer<BrowserWindow> m_lastActiveWindow;

    QList<PostLaunchAction> m_postLaunchActions;

    QString m_languageFile;

#if defined(Q_OS_WIN) && !defined(Q_OS_OS2)
public:
    RegisterQAppAssociation* associationManager();

private:
    RegisterQAppAssociation* m_registerQAppAssociation;
#endif

#ifdef Q_OS_MAC
public:
    bool event(QEvent* e);
#endif
};

#endif // MAINAPPLICATION_H
