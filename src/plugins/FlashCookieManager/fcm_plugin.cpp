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
#include "fcm_plugin.h"
#include "browserwindow.h"
#include "pluginproxy.h"
#include "mainapplication.h"
#include "fcm_dialog.h"
#include "clickablelabel.h"
#include "tabbedwebview.h"
#include "fcm_notification.h"

#include <QStatusBar>
#include <QTimer>
#include <QSettings>
#include <QTranslator>
#include <QDir>

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
#include <QProcessEnvironment>
#endif

const int refreshInterval = 60 * 1000;

FCM_Plugin::FCM_Plugin()
    : QObject()
{
}

PluginSpec FCM_Plugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "Flash Cookie Manager";
    spec.info = "A plugin to manage flash cookies.";
    spec.description = "You can easily view/delete flash cookies stored on your computer. This is a solution for having more privacy.";
    spec.version = "0.3.0";
    spec.author = "Razi Alavizadeh <s.r.alavizadeh@gmail.com>";
    spec.icon = QPixmap(":/flashcookiemanager/data/flash-cookie-manager.png");
    spec.hasSettings = true;

    return spec;
}

void FCM_Plugin::init(InitState state, const QString &settingsPath)
{
    m_settingsPath = settingsPath;

    connect(mApp->plugins(), SIGNAL(mainWindowCreated(BrowserWindow*)), this, SLOT(mainWindowCreated(BrowserWindow*)));
    connect(mApp->plugins(), SIGNAL(mainWindowDeleted(BrowserWindow*)), this, SLOT(mainWindowDeleted(BrowserWindow*)));

    m_timer = new QTimer(this);
    m_timer->setInterval(refreshInterval);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(autoRefresh()));

    // start timer if needed
    startStopTimer();

    if (state == StartupInitState && readSettings().value(QL1S("deleteAllOnStartExit")).toBool()) {
        loadFlashCookies();

        removeAllButWhitelisted();
    }

    if (state == LateInitState) {
        foreach (BrowserWindow* window, mApp->windows()) {
            mainWindowCreated(window);
        }
    }
}

void FCM_Plugin::unload()
{
    if (m_fcmDialog) {
        m_fcmDialog->close();
    }

    if (mApp->isClosing() && readSettings().value(QL1S("deleteAllOnStartExit")).toBool()) {
        removeAllButWhitelisted();
    }

    foreach (BrowserWindow* window, mApp->windows()) {
        window->statusBar()->removeWidget(m_statusBarIcons.value(window));
        delete m_statusBarIcons.value(window);
        m_statusBarIcons.remove(window);
    }

    delete m_fcmDialog;
}

bool FCM_Plugin::testPlugin()
{
    return (Qz::VERSION == QLatin1String(QUPZILLA_VERSION));
}

QTranslator* FCM_Plugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator(this);
    translator->load(locale, ":/flashcookiemanager/locale/");
    return translator;
}

void FCM_Plugin::showSettings(QWidget* parent)
{
    Q_UNUSED(parent)

    showFlashCookieManager();
    m_fcmDialog->showPage(2);
}

void FCM_Plugin::setFlashCookies(const QList<FlashCookie> &flashCookies)
{
    m_flashCookies = flashCookies;
}

QList<FlashCookie> FCM_Plugin::flashCookies()
{
    if (m_flashCookies.isEmpty()) {
        loadFlashCookies();
    }
    return m_flashCookies;
}

QStringList FCM_Plugin::newCookiesList()
{
    return m_newCookiesList;
}

void FCM_Plugin::clearNewOrigins()
{
    m_newCookiesList.clear();
}

void FCM_Plugin::clearCache()
{
    m_flashCookies.clear();
}

QString FCM_Plugin::flashDataPathForOS() const
{
    /* On Microsoft Windows NT 5.x and 6.x, they are stored in:
     *  %APPDATA%\Macromedia\Flash Player\#SharedObjects\
     *  %APPDATA%\Macromedia\Flash Player\macromedia.com\support\flashplayer\sys\
     * On Mac OS X, they are stored in:
     * ~/Library/Preferences/Macromedia/Flash Player/#SharedObjects/
     * ~/Library/Preferences/Macromedia/Flash Player/macromedia.com/support/flashplayer/sys/
     * On Linux or Unix, they are stored in:
     * ~/.macromedia/Flash_Player/#SharedObjects/
     * ~/.macromedia/Flash_Player/macromedia.com/support/flashplayer/sys/
     * For Linux and Unix systems, if the open-source Gnash plugin is being used
     *  instead of the official Adobe Flash, they will instead be found at:
     * ~/.gnash/SharedObjects/
     */

    if (m_flashDataPathForOS.isEmpty()) {
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
        QString appData = QProcessEnvironment::systemEnvironment().value(QL1S("APPDATA"));
        appData.replace(QL1C('\\'), QL1C('/'));

        m_flashDataPathForOS = (appData + QL1S("/Macromedia/Flash Player"));
#elif defined(Q_OS_MAC)
        m_flashDataPathForOS = QDir::homePath() + QL1S("/Library/Preferences/Macromedia/Flash Player");
#else
        if (QDir::home().cd(QL1S(".macromedia"))) {
            m_flashDataPathForOS = QDir::homePath() +  QL1S("/.macromedia/Flash_Player");
        }
        else {
            m_flashDataPathForOS = QDir::homePath() +  QL1S("/.gnash");
        }
#endif
    }

    return m_flashDataPathForOS;
}

bool FCM_Plugin::isBlacklisted(const FlashCookie &flashCookie)
{
    return readSettings().value(QL1S("flashCookiesBlacklist")).toStringList().contains(flashCookie.origin);
}

bool FCM_Plugin::isWhitelisted(const FlashCookie &flashCookie)
{
    return readSettings().value(QL1S("flashCookiesWhitelist")).toStringList().contains(flashCookie.origin);
}

void FCM_Plugin::removeAllButWhitelisted()
{
    foreach (const FlashCookie &flashCookie, m_flashCookies) {
        if (isWhitelisted(flashCookie)) {
            continue;
        }

        removeCookie(flashCookie);
    }
}

QString FCM_Plugin::sharedObjectDirName() const
{
    if (flashPlayerDataPath().contains(QL1S("macromedia"), Qt::CaseInsensitive) ||
            !flashPlayerDataPath().contains(QL1S("/.gnash"), Qt::CaseInsensitive)) {
        return QLatin1String(QL1S("/#SharedObjects/"));
    }
    else {
        return QLatin1String(QL1S("/SharedObjects/"));
    }
}

QString FCM_Plugin::flashPlayerDataPath() const
{
    return readSettings().value(QL1S("flashDataPath")).toString();
}

QVariantHash FCM_Plugin::readSettings() const
{
    if (m_settingsHash.isEmpty()) {
        m_settingsHash.insert(QL1S("autoMode"), QVariant(false));
        m_settingsHash.insert(QL1S("deleteAllOnStartExit"), QVariant(false));
        m_settingsHash.insert(QL1S("notification"), QVariant(false));
        m_settingsHash.insert(QL1S("flashCookiesWhitelist"), QVariant());
        m_settingsHash.insert(QL1S("flashCookiesBlacklist"), QVariant());
        m_settingsHash.insert(QL1S("flashDataPath"), flashDataPathForOS());

        QSettings settings(m_settingsPath + QL1S("/extensions.ini"), QSettings::IniFormat);
        settings.beginGroup(QL1S("FlashCookieManager"));
        QVariantHash::iterator i = m_settingsHash.begin();
        while (i != m_settingsHash.end()) {
            *i = settings.value(i.key(), i.value());
            ++i;
        }

        settings.endGroup();
    }

    return m_settingsHash;
}

void FCM_Plugin::writeSettings(const QVariantHash &hashSettings)
{
    m_settingsHash = hashSettings;

    QSettings settings(m_settingsPath + QL1S(QL1S("/extensions.ini")), QSettings::IniFormat);
    settings.beginGroup(QL1S("FlashCookieManager"));
    QVariantHash::const_iterator i = m_settingsHash.constBegin();
    while (i != m_settingsHash.constEnd()) {
        settings.setValue(i.key(), i.value());
        ++i;
    }

    settings.endGroup();

    startStopTimer();
}

void FCM_Plugin::removeCookie(const FlashCookie &flashCookie)
{
    if (m_flashCookies.contains(flashCookie)) {
        m_flashCookies.removeOne(flashCookie);
        if (QFile(flashCookie.path + QL1C('/') + flashCookie.name).remove()) {
            QDir dir(flashCookie.path);
            dir.rmpath(flashCookie.path);
        }
    }
}

void FCM_Plugin::autoRefresh()
{
    if (m_fcmDialog && m_fcmDialog->isVisible()) {
        return;
    }

    QList<FlashCookie> oldflashCookies = m_flashCookies;
    loadFlashCookies();
    QStringList newCookieList;

    foreach (const FlashCookie &flashCookie, m_flashCookies) {
        if (isBlacklisted(flashCookie)) {
            removeCookie(flashCookie);
            continue;
        }

        if (isWhitelisted(flashCookie)) {
            continue;
        }

        bool newCookie = true;
        foreach (const FlashCookie &oldFlashCookie, oldflashCookies) {
            if (QString(oldFlashCookie.path + oldFlashCookie.name) ==
                    QString(flashCookie.path + flashCookie.name)) {
                newCookie = false;
                break;
            }
        }

        if (newCookie) {
            newCookieList << flashCookie.path + QL1C('/') + flashCookie.name;
        }
    }

    if (!newCookieList.isEmpty() && readSettings().value(QL1S("notification")).toBool()) {
        m_newCookiesList << newCookieList;
        BrowserWindow* window = mApp->getWindow();
        if (!window) {
            return;
        }
        TabbedWebView* weView = window->weView();
        if (!weView) {
            return;
        }

        FCM_Notification* notif = new FCM_Notification(this, newCookieList.size());
        weView->addNotification(notif);
    }
}

void FCM_Plugin::showFlashCookieManager()
{
    if (!m_fcmDialog) {
        m_fcmDialog = new FCM_Dialog(this);
    }

    m_fcmDialog->refreshView();
    m_fcmDialog->showPage(0);
    m_fcmDialog->show();
    m_fcmDialog->raise();
}

void FCM_Plugin::mainWindowCreated(BrowserWindow *window)
{
    window->statusBar()->addPermanentWidget(createStatusBarIcon(window));
}

void FCM_Plugin::mainWindowDeleted(BrowserWindow *window)
{
    if (!window) {
        return;
    }

    if (m_fcmDialog && m_fcmDialog->parent() == window) {
        m_fcmDialog->setParent(0);
    }

    window->statusBar()->removeWidget(m_statusBarIcons.value(window));
    delete m_statusBarIcons.value(window);
    m_statusBarIcons.remove(window);
}

void FCM_Plugin::startStopTimer()
{
    if (readSettings().value(QL1S("autoMode")).toBool()) {
        if (!m_timer->isActive()) {
            if (m_flashCookies.isEmpty()) {
                loadFlashCookies();
            }

            m_timer->start();
        }
    }
    else {
        m_timer->stop();
    }
}

QWidget* FCM_Plugin::createStatusBarIcon(BrowserWindow* mainWindow)
{
    if (m_statusBarIcons.contains(mainWindow)) {
        return m_statusBarIcons.value(mainWindow);
    }

    ClickableLabel* icon = new ClickableLabel(mainWindow);
    icon->setCursor(Qt::PointingHandCursor);
    QPixmap p(":/flashcookiemanager/data/flash-cookie-manager.png");
    icon->setPixmap(p.scaledToHeight(16));
    icon->setToolTip(tr("Show/Hide Flash Cookie Manager"));

    connect(icon, SIGNAL(clicked(QPoint)), this, SLOT(showFlashCookieManager()));

    m_statusBarIcons.insert(mainWindow, icon);

    return icon;
}

void FCM_Plugin::loadFlashCookies()
{
    m_flashCookies.clear();
    loadFlashCookies(flashPlayerDataPath());
}

void FCM_Plugin::loadFlashCookies(QString path)
{
    QDir solDir(path);
    QStringList entryList = solDir.entryList();
    entryList.removeAll(QL1S("."));
    entryList.removeAll(QL1S(".."));

    foreach(QString entry, entryList) {
        if (path.endsWith(QL1S("#SharedObjects")) && entry == QL1S("#AppContainer")) {
            // specific to IE and Windows
            continue;
        }

        path.replace(QL1C('\\'), QL1C('/'));
        QFileInfo entryInfo(path + QL1C('/') + entry);
        if (entryInfo.isDir()) {
            loadFlashCookies(entryInfo.filePath());
        }
        else if (entryInfo.isFile() && entryInfo.suffix() == QL1S("sol")) {
            insertFlashCookie(entryInfo.filePath());
        }
    }
}

void FCM_Plugin::insertFlashCookie(QString path)
{
    QFile solFile(path);
    if (!solFile.open(QFile::ReadOnly)) {
        return;
    }

    QByteArray file = solFile.readAll();
    for (int i = 0; i < file.size(); ++i) {
        if (!((file.at(i) >= 'a' && file.at(i) <= 'z') || (file.at(i) >= 'A' && file.at(i) <= 'Z') ||
              (file.at(i) >= '0' && file.at(i) <= '9'))) {
            file[i] = ' ';
        }
    }

    QString fileStr = QString(file);
    fileStr = fileStr.split(QL1C('.'), QString::SkipEmptyParts).join(QL1S("\n"));

    QFileInfo solFileInfo(solFile);

    FlashCookie flashCookie;
    flashCookie.contents = fileStr;
    flashCookie.name = solFileInfo.fileName();
    flashCookie.path = solFileInfo.canonicalPath();
    flashCookie.size = (int)solFile.size();
    flashCookie.lastModification = solFileInfo.lastModified();
    flashCookie.origin = extractOriginFrom(path);

    m_flashCookies << flashCookie;
}

QString FCM_Plugin::extractOriginFrom(const QString &path)
{
    QString origin = path;
    if (path.startsWith(flashPlayerDataPath() + sharedObjectDirName())) {
        origin.remove(flashPlayerDataPath() + sharedObjectDirName());
        if (origin.indexOf(QL1C('/')) != -1) {
            origin.remove(0, origin.indexOf(QL1C('/')) + 1);
        }
    }
    else if (path.startsWith(flashPlayerDataPath() + QL1S("/macromedia.com/support/flashplayer/sys/"))) {
        origin.remove(flashPlayerDataPath() + QL1S("/macromedia.com/support/flashplayer/sys/"));
        if (origin == QL1S("settings.sol")) {
            return tr("!default");
        }
        else if (origin.startsWith(QL1C('#'))) {
            origin.remove(0, 1);
        }
    }
    else {
        origin.clear();
    }

    int index = origin.indexOf(QL1C('/'));
    if (index == -1) {
        return tr("!other");
    }
    origin = origin.remove(index, origin.size());
    if (origin == QL1S("localhost") || origin == QL1S("local")) {
        origin = QL1S("!localhost");
    }

    return origin;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(FlashCookieManagerPlugin, FCM_Plugin)
#endif
