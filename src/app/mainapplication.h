#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QToolBar>
#include <QWebSettings>
#include <QUrl>
#include <QPointer>
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

class MainApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    QString DATADIR;
    explicit MainApplication(int &argc, char **argv);

    enum MessageType{ ShowFlashIcon, CheckPrivateBrowsing };

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

    bool checkProfileDir();

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
    BookmarksModel* bookmarks();
    DownloadManager* downManager();
    AutoFillModel* autoFill();

public slots:
    bool saveStateSlot();
    void quitApplication();
    void sendMessages(MainApplication::MessageType mes, bool state);
    void receiveAppMessage(QString message);
    inline void setChanged() { m_isChanged = true; }

signals:
    void message(MainApplication::MessageType mes, bool state);

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

    QList<QPointer<QupZilla> > m_mainWindows;

    QString m_activeProfil;
    QString m_activeLanguage;

    bool m_isClosing;
    bool m_isChanged;
    bool m_isExited;
};

#endif // MAINAPPLICATION_H
