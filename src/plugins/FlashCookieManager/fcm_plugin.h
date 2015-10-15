/* ============================================================
* FlashCookieManager plugin for QupZilla
* Copyright (C) 2014  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#ifndef FLASHCOOKIEMANAGER_H
#define FLASHCOOKIEMANAGER_H

#include "plugininterface.h"

#include <QPointer>
#include <QDateTime>

class BrowserWindow;
class FCM_Dialog;
class QTimer;

struct FlashCookie {
    QString name;
    QString origin;
    int size;
    QString path;
    QString contents;
    QDateTime lastModification;

    bool operator ==(const FlashCookie &other) {
        return (this->name == other.name && this->path == other.path);
    }
};

class FCM_Plugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)

#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "QupZilla.Browser.plugin.FlashCookieManager")
#endif

public:
    explicit FCM_Plugin();


    PluginSpec pluginSpec();

    void init(InitState state, const QString &settingsPath);
    void unload();
    bool testPlugin();

    QTranslator* getTranslator(const QString &locale);
    void showSettings(QWidget* parent = 0);
    void  populateExtensionsMenu(QMenu* menu);

    void setFlashCookies(const QList<FlashCookie> &flashCookies);
    QList<FlashCookie> flashCookies();
    QStringList newCookiesList();
    void clearNewOrigins();
    void clearCache();
    QString flashPlayerDataPath() const;
    QVariantHash readSettings() const;
    void writeSettings(const QVariantHash &hashSettings);

    void removeCookie(const FlashCookie &flashCookie);

private slots:
    void autoRefresh();
    void showFlashCookieManager();
    void mainWindowCreated(BrowserWindow* window);
    void mainWindowDeleted(BrowserWindow* window);
    void startStopTimer();

private:
    QWidget* createStatusBarIcon(BrowserWindow* mainWindow);
    void loadFlashCookies();
    void loadFlashCookies(QString path);
    void insertFlashCookie(QString path);
    QString extractOriginFrom(const QString &path);
    QString flashDataPathForOS() const;
    bool isBlacklisted(const FlashCookie &flashCookie);
    bool isWhitelisted(const FlashCookie &flashCookie);
    void removeAllButWhitelisted();
    QString sharedObjectDirName() const;

    QHash<BrowserWindow*, QWidget*> m_statusBarIcons;
    QPointer<FCM_Dialog> m_fcmDialog;

    QString m_settingsPath;
    QList<FlashCookie> m_flashCookies;
    QTimer* m_timer;

    mutable QVariantHash m_settingsHash;
    bool m_autoMode;
    bool m_deleteOnClose;
    bool m_enableNotification;
    QStringList m_blaklist;
    QStringList m_whitelist;
    QStringList m_newCookiesList;
    mutable QString m_flashDataPathForOS;
};

Q_DECLARE_METATYPE(FlashCookie);
#endif // FLASHCOOKIEMANAGER_H
