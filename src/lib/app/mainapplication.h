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
#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#define mApp MainApplication::getInstance()

#include <QList>
#include <QUrl>
#include <QPointer>

#include "restoremanager.h"
#include "qtsingleapplication.h"
#include "qz_namespace.h"

class QWebSettings;
class QNetworkDiskCache;

class QupZilla;
class CookieManager;
class BrowsingLibrary;
class History;
class NetworkManager;
class CookieJar;
class RSSManager;
class Updater;
class PluginProxy;
class BookmarksModel;
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
class Speller;
#ifdef Q_OS_MAC
class MacMenuReceiver;
#endif

class QT_QUPZILLA_EXPORT MainApplication : public QtSingleApplication
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
    void loadSettings();
    void reloadSettings();
    bool restoreStateSlot(QupZilla* window, RestoreData recoveryData);
    QupZilla* makeNewWindow(Qz::BrowserWindow type, const QUrl &startUrl = QUrl());
    void aboutToCloseWindow(QupZilla* window);
    bool isStateChanged();

    QList<QupZilla*> mainWindows();

    static MainApplication* getInstance() { return static_cast<MainApplication*>(QCoreApplication::instance()); }

    bool isClosing() const;
    bool isRestoring() const;
    bool isPrivateSession() const;
    bool isStartingAfterCrash() const;
    int windowCount() const;
    QString currentLanguageFile() const;
    QString currentLanguage() const;
    QString currentProfilePath() const;

    bool checkSettingsDir();
    void destroyRestoreManager();
    void clearTempPath();

    void setProxyStyle(ProxyStyle* style);
    QString currentStyle() const;
    QString tempPath() const;

    QupZilla* getWindow();
    CookieManager* cookieManager();
    BrowsingLibrary* browsingLibrary();
    History* history();
    QWebSettings* webSettings();
    NetworkManager* networkManager();
    CookieJar* cookieJar();
    RSSManager* rssManager();
    PluginProxy* plugins();
    BookmarksModel* bookmarksModel();
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

#ifdef Q_OS_WIN
    RegisterQAppAssociation* associationManager();
#endif

#ifdef Q_OS_MAC
    MacMenuReceiver* macMenuReceiver();
    bool event(QEvent* e);
#endif

public slots:
    bool saveStateSlot();
    void quitApplication();
    void sendMessages(Qz::AppMessageType mes, bool state);
    void receiveAppMessage(QString message);
    void setStateChanged();
    void addNewTab(const QUrl &url = QUrl());

    void startPrivateBrowsing();
    void reloadUserStyleSheet();
    bool checkDefaultWebBrowser();

signals:
    void message(Qz::AppMessageType mes, bool state);

private slots:
    void postLaunch();
    void setupJumpList();
    void restoreCursor();

    void saveSettings();

private:
    enum PostLaunchAction { OpenDownloadManager, OpenNewTab };

    void translateApp();
    void restoreOtherWindows();

    QUrl userStyleSheet(const QString &filePath) const;

    CookieManager* m_cookiemanager;
    BrowsingLibrary* m_browsingLibrary;
    History* m_historymodel;
    QWebSettings* m_websettings;
    NetworkManager* m_networkmanager;
    CookieJar* m_cookiejar;
    RSSManager* m_rssmanager;
    PluginProxy* m_plugins;
    BookmarksModel* m_bookmarksModel;
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
    QList<QPointer<QupZilla> > m_mainWindows;

    QString m_activeProfil;
    QString m_activeLanguage;
    QString m_activeThemePath;

    bool m_isPrivateSession;
    bool m_isClosing;
    bool m_isStateChanged;
    bool m_isRestoring;
    bool m_startingAfterCrash;

    bool m_databaseConnected;
    QList<PostLaunchAction> m_postLaunchActions;

#ifdef Q_OS_WIN
    RegisterQAppAssociation* m_registerQAppAssociation;
#endif
#ifdef Q_OS_MAC
    MacMenuReceiver* m_macMenuReceiver;
#endif
};

#endif // MAINAPPLICATION_H
