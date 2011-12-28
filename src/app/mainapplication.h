/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#define mStyle MainApplication::appStyle

#include <QToolBar>
#include <QWebSettings>
#include <QUrl>
#include <QWeakPointer>
#include <QNetworkDiskCache>
#include <QWebSecurityOrigin>
#include <iostream>

#include "qtsingleapplication.h"
#include "commandlineoptions.h"

class QupZilla;
class CookieManager;
class BrowsingLibrary;
class HistoryModel;
class NetworkManager;
class CookieJar;
class RSSManager;
class Updater;
class PluginProxy;
class BookmarksModel;
class DownloadManager;
class AutoFillModel;
class DesktopNotificationsFactory;
class IconProvider;
class Style;
class SearchEnginesManager;
class DatabaseWriter;
class MainApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    QString DATADIR;
    QString PROFILEDIR;
    QString PLUGINSDIR;
    QString TRANSLATIONSDIR;
    QString THEMESDIR;

    explicit MainApplication(const QList<CommandLineOptions::ActionPair> &cmdActions, int &argc, char** argv);

    enum MessageType { SetAdBlockIconEnabled, CheckPrivateBrowsing, ReloadSettings,
                       HistoryStateChanged, BookmarksChanged, StartPrivateBrowsing
                     };

    void connectDatabase();
    void loadSettings();
    void reloadSettings() { loadSettings(); emit message(ReloadSettings, true); }
    bool restoreStateSlot(QupZilla* window);
    QupZilla* makeNewWindow(bool tryRestore, const QUrl &startUrl = QUrl());
    void addNewTab(const QUrl &url = QUrl());
    void aboutToCloseWindow(QupZilla* window);
    bool isStateChanged();

    inline static MainApplication* getInstance() { return static_cast<MainApplication*>(QCoreApplication::instance()); }
    inline QString getActiveProfilPath() { return m_activeProfil; }
    inline QString getActiveLanguage() { return m_activeLanguage; }
    inline bool isClosing() { return m_isClosing; }
    inline bool isExited() { return m_isExited; }
    inline int windowCount() { return m_mainWindows.count(); }

    bool checkSettingsDir();

    int defaultZoom() { return m_defaultZoom; }

    QupZilla* getWindow();
    CookieManager* cookieManager();
    BrowsingLibrary* browsingLibrary();
    HistoryModel* history();
    QWebSettings* webSettings();
    NetworkManager* networkManager();
    CookieJar* cookieJar();
    RSSManager* rssManager();
    PluginProxy* plugins();
    BookmarksModel* bookmarksModel();
    DownloadManager* downManager();
    AutoFillModel* autoFill();
    SearchEnginesManager* searchEnginesManager();
    QNetworkDiskCache* networkCache() { return m_networkCache; }
    DesktopNotificationsFactory* desktopNotifications();
    IconProvider* iconProvider() { return m_iconProvider; }
    DatabaseWriter* dbWriter() { return m_dbWriter; }

    static Style* appStyle;

public slots:
    bool saveStateSlot();
    void quitApplication();
    void sendMessages(MainApplication::MessageType mes, bool state);
    void receiveAppMessage(QString message);
    void setStateChanged();

signals:
    void message(MainApplication::MessageType mes, bool state);

private slots:
    void setupJumpList();
    void restoreCursor();

private:
    void translateApp();
    void restoreOtherWindows();

    CookieManager* m_cookiemanager;
    BrowsingLibrary* m_browsingLibrary;
    HistoryModel* m_historymodel;
    QWebSettings* m_websettings;
    NetworkManager* m_networkmanager;
    CookieJar* m_cookiejar;
    RSSManager* m_rssmanager;
    Updater* m_updater;
    PluginProxy* m_plugins;
    BookmarksModel* m_bookmarksModel;
    DownloadManager* m_downloadManager;
    AutoFillModel* m_autofill;
    QNetworkDiskCache* m_networkCache;
    DesktopNotificationsFactory* m_desktopNotifications;
    IconProvider* m_iconProvider;
    SearchEnginesManager* m_searchEnginesManager;
    DatabaseWriter* m_dbWriter;

    QList<QWeakPointer<QupZilla> > m_mainWindows;

    QString m_activeProfil;
    QString m_activeLanguage;
    QString m_activeThemePath;
    int m_defaultZoom;

    bool m_isClosing;
    bool m_isStateChanged;
    bool m_isExited;
    bool m_isRestoring;

    bool m_databaseConnected;
};

#endif // MAINAPPLICATION_H
