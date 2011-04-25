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
#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H
#define mApp MainApplication::getInstance()

#include <QToolBar>
#include <QWebSettings>
#include <QUrl>
#include <QPointer>
#include <QNetworkDiskCache>
#include <iostream>

#include "qtsingleapplication.h"

class QupZilla;
class BookmarksManager;
class CookieManager;
class HistoryManager;
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

class MainApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    QString DATADIR;
    explicit MainApplication(int &argc, char **argv);

    enum MessageType{ SetAdBlockIconEnabled, CheckPrivateBrowsing };

    void loadSettings();
    bool restoreStateSlot(QupZilla* window);
    void makeNewWindow(bool tryRestore, const QUrl &startUrl=QUrl());
    void addNewTab(QUrl url);
    void aboutToCloseWindow(QupZilla* window);
    bool isChanged();

    inline static MainApplication* getInstance() { return static_cast<MainApplication*>(QCoreApplication::instance()); }
    inline QString getActiveProfil() { return m_activeProfil; }
    inline QString getActiveLanguage() { return m_activeLanguage; }
    inline bool isClosing() { return m_isClosing; }
    inline bool isExited() { return m_isExited; }
    bool checkSettingsDir();
    void checkProfile(QString path);
    inline int windowCount() { return m_mainWindows.count(); }

    QupZilla* getWindow();
    BookmarksManager* bookmarksManager();
    CookieManager* cookieManager();
    HistoryManager* historyManager();
    HistoryModel* history();
    QWebSettings* webSettings();
    NetworkManager* networkManager();
    CookieJar* cookieJar();
    RSSManager* rssManager();
    PluginProxy* plugins();
    BookmarksModel* bookmarksModel();
    DownloadManager* downManager();
    AutoFillModel* autoFill();
    QNetworkDiskCache* networkCache() { return m_networkCache; }
    DesktopNotificationsFactory* desktopNotifications();
    IconProvider* iconProvider() { return m_iconProvider; }

public slots:
    bool saveStateSlot();
    void quitApplication();
    void sendMessages(MainApplication::MessageType mes, bool state);
    void receiveAppMessage(QString message);
    inline void setChanged() { m_isChanged = true; }

signals:
    void message(MainApplication::MessageType mes, bool state);

private slots:
    void restoreCursor() { QApplication::restoreOverrideCursor(); }

private:
    void connectDatabase();
    void translateApp();
    void restoreOtherWindows();

    BookmarksManager* m_bookmarksmanager;
    CookieManager* m_cookiemanager;
    HistoryManager* m_historymanager;
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

    QList<QPointer<QupZilla> > m_mainWindows;

    QString m_activeProfil;
    QString m_activeLanguage;

    bool m_isClosing;
    bool m_isChanged;
    bool m_isExited;
    bool m_isRestoring;
};

#endif // MAINAPPLICATION_H
