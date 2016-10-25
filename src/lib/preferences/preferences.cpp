/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2016  David Rosca <nowrep@gmail.com>
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
#include "preferences.h"
#include "ui_preferences.h"
#include "browserwindow.h"
#include "bookmarkstoolbar.h"
#include "history.h"
#include "tabwidget.h"
#include "cookiejar.h"
#include "locationbar.h"
#include "autofillmanager.h"
#include "mainapplication.h"
#include "cookiemanager.h"
#include "pluginproxy.h"
#include "pluginsmanager.h"
#include "qtwin.h"
#include "jsoptions.h"
#include "networkproxyfactory.h"
#include "networkmanager.h"
#include "desktopnotificationsfactory.h"
#include "desktopnotification.h"
#include "navigationbar.h"
#include "thememanager.h"
#include "acceptlanguage.h"
#include "qztools.h"
#include "autofill.h"
#include "settings.h"
#include "datapaths.h"
#include "tabbedwebview.h"
#include "clearprivatedata.h"
#include "useragentdialog.h"
#include "registerqappassociation.h"
#include "profilemanager.h"
#include "html5permissions/html5permissionsdialog.h"
#include "searchenginesdialog.h"

#include <QSettings>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QColorDialog>
#include <QDesktopWidget>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QLibraryInfo>

static QString createLanguageItem(const QString &lang)
{
    QLocale locale(lang);

    if (locale.language() == QLocale::C) {
        return lang;
    }

    QString country = QLocale::countryToString(locale.country());
    QString language = QLocale::languageToString(locale.language());

    if (lang == QLatin1String("es_ES")) {
        return QString::fromUtf8("Castellano");
    }
    if (lang == QLatin1String("nqo")) {
        return QString("N'ko (nqo)");
    }
    if (lang == QLatin1String("sr")) {
        return QString::fromUtf8("српски екавски");
    }
    if (lang == QLatin1String("sr@ijekavian")) {
        return QString::fromUtf8("српски ијекавски");
    }
    if (lang == QLatin1String("sr@latin")) {
        return QString::fromUtf8("srpski ekavski");
    }
    if (lang == QLatin1String("sr@ijekavianlatin")) {
        return QString::fromUtf8("srpski ijekavski");
    }
    return QString("%1, %2 (%3)").arg(language, country, lang);
}

Preferences::Preferences(BrowserWindow* window)
    : QWidget()
    , ui(new Ui::Preferences)
    , m_window(window)
    , m_autoFillManager(0)
    , m_pluginsList(0)
    , m_autoFillEnabled(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->languages->setLayoutDirection(Qt::LeftToRight);
    QzTools::centerWidgetOnScreen(this);

    m_themesManager = new ThemeManager(ui->themesWidget, this);
    m_pluginsList = new PluginsManager(this);
    ui->pluginsFrame->addWidget(m_pluginsList);

#ifdef DISABLE_CHECK_UPDATES
    ui->checkUpdates->setVisible(false);
#endif

    ui->listWidget->item(0)->setIcon(QIcon::fromTheme("preferences-desktop", QIcon(":/icons/preferences/preferences-desktop.png")));
    ui->listWidget->item(1)->setIcon(QIcon::fromTheme("application-x-theme", QIcon(":/icons/preferences/application-x-theme.png")));
    ui->listWidget->item(2)->setIcon(QIcon::fromTheme("applications-internet", QIcon(":/icons/preferences/applications-internet.png")));
    ui->listWidget->item(3)->setIcon(QIcon::fromTheme("applications-webbrowsers", QIcon(":/icons/preferences/applications-webbrowsers.png")));
    ui->listWidget->item(4)->setIcon(QIcon::fromTheme("applications-fonts", QIcon(":/icons/preferences/applications-fonts.png")));
    ui->listWidget->item(5)->setIcon(QIcon::fromTheme("preferences-desktop-keyboard-shortcuts", QIcon(":/icons/preferences/preferences-desktop-keyboard-shortcuts.png")));
    ui->listWidget->item(6)->setIcon(QIcon::fromTheme("mail-inbox", QIcon(":/icons/preferences/mail-inbox.png")));
    ui->listWidget->item(7)->setIcon(QIcon::fromTheme("dialog-password", QIcon(":/icons/preferences/dialog-password.png")));
    ui->listWidget->item(8)->setIcon(QIcon::fromTheme("preferences-system-firewall", QIcon(":/icons/preferences/preferences-system-firewall.png")));
    ui->listWidget->item(9)->setIcon(QIcon::fromTheme("dialog-question", QIcon(":/icons/preferences/dialog-question.png")));
    ui->listWidget->item(10)->setIcon(QIcon::fromTheme("extension", QIcon(":/icons/preferences/extension.png")));
    ui->listWidget->item(11)->setIcon(QIcon::fromTheme("tools-check-spelling", QIcon(":/icons/preferences/tools-check-spelling.png")));
    ui->listWidget->item(12)->setIcon(QIcon::fromTheme("applications-system", QIcon(":/icons/preferences/applications-system.png")));

    Settings settings;
    //GENERAL URLs
    settings.beginGroup("Web-URL-Settings");
    m_homepage = settings.value("homepage", QUrl(QSL("qupzilla:start"))).toUrl();
    m_newTabUrl = settings.value("newTabUrl", QUrl(QSL("qupzilla:speeddial"))).toUrl();
    ui->homepage->setText(m_homepage.toEncoded());
    ui->newTabUrl->setText(m_newTabUrl.toEncoded());
    settings.endGroup();
    ui->afterLaunch->setCurrentIndex(mApp->afterLaunch());
    ui->checkUpdates->setChecked(settings.value("Web-Browser-Settings/CheckUpdates", true).toBool());
    ui->dontLoadTabsUntilSelected->setChecked(settings.value("Web-Browser-Settings/LoadTabsOnActivation", true).toBool());

#if defined(Q_OS_WIN) && !defined(Q_OS_OS2)
    ui->checkDefaultBrowser->setChecked(settings.value("Web-Browser-Settings/CheckDefaultBrowser", DEFAULT_CHECK_DEFAULTBROWSER).toBool());
    if (mApp->associationManager()->isDefaultForAllCapabilities()) {
        ui->checkNowDefaultBrowser->setText(tr("Default"));
        ui->checkNowDefaultBrowser->setEnabled(false);
    }
    else {
        ui->checkNowDefaultBrowser->setText(tr("Set as default"));
        ui->checkNowDefaultBrowser->setEnabled(true);
        connect(ui->checkNowDefaultBrowser, SIGNAL(clicked()), this, SLOT(makeQupZillaDefault()));
    }
#else // No Default Browser settings on non-Windows platform
    ui->hSpacerDefaultBrowser->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->hLayoutDefaultBrowser->invalidate();
    delete ui->hLayoutDefaultBrowser;
    delete ui->checkDefaultBrowser;
    delete ui->checkNowDefaultBrowser;
#endif

    ui->newTabFrame->setVisible(false);
    if (m_newTabUrl.isEmpty() || m_newTabUrl.toString() == QL1S("about:blank")) {
        ui->newTab->setCurrentIndex(0);
    }
    else if (m_newTabUrl == m_homepage) {
        ui->newTab->setCurrentIndex(1);
    }
    else if (m_newTabUrl.toString() == QL1S("qupzilla:speeddial")) {
        ui->newTab->setCurrentIndex(2);
    }
    else {
        ui->newTab->setCurrentIndex(3);
        ui->newTabFrame->setVisible(true);
    }

    afterLaunchChanged(ui->afterLaunch->currentIndex());
    connect(ui->afterLaunch, SIGNAL(currentIndexChanged(int)), this, SLOT(afterLaunchChanged(int)));
    connect(ui->newTab, SIGNAL(currentIndexChanged(int)), this, SLOT(newTabChanged(int)));
    if (m_window) {
        connect(ui->useCurrentBut, SIGNAL(clicked()), this, SLOT(useActualHomepage()));
        connect(ui->newTabUseCurrent, SIGNAL(clicked()), this, SLOT(useActualNewTab()));
    }
    else {
        ui->useCurrentBut->setEnabled(false);
        ui->newTabUseCurrent->setEnabled(false);
    }

    // PROFILES
    QString startingProfile = ProfileManager::startingProfile();
    ui->activeProfile->setText("<b>" + ProfileManager::currentProfile() + "</b>");
    ui->startProfile->addItem(startingProfile);

    foreach (const QString &name, ProfileManager::availableProfiles()) {
        if (startingProfile != name) {
            ui->startProfile->addItem(name);
        }
    }

    connect(ui->createProfile, SIGNAL(clicked()), this, SLOT(createProfile()));
    connect(ui->deleteProfile, SIGNAL(clicked()), this, SLOT(deleteProfile()));
    connect(ui->startProfile, SIGNAL(currentIndexChanged(int)), this, SLOT(startProfileIndexChanged(int)));
    startProfileIndexChanged(ui->startProfile->currentIndex());

    //APPEREANCE
    settings.beginGroup("Browser-View-Settings");
    ui->showStatusbar->setChecked(settings.value("showStatusBar", false).toBool());
    // NOTE: instantBookmarksToolbar and showBookmarksToolbar cannot be both enabled at the same time
    ui->instantBookmarksToolbar->setChecked(settings.value("instantBookmarksToolbar", false).toBool());
    ui->showBookmarksToolbar->setChecked(settings.value("showBookmarksToolbar", true).toBool());
    ui->instantBookmarksToolbar->setDisabled(settings.value("showBookmarksToolbar", true).toBool());
    ui->showBookmarksToolbar->setDisabled(settings.value("instantBookmarksToolbar").toBool());
    connect(ui->instantBookmarksToolbar, SIGNAL(toggled(bool)), ui->showBookmarksToolbar, SLOT(setDisabled(bool)));
    connect(ui->showBookmarksToolbar, SIGNAL(toggled(bool)), ui->instantBookmarksToolbar, SLOT(setDisabled(bool)));
    ui->showNavigationToolbar->setChecked(settings.value("showNavigationToolbar", true).toBool());
    ui->showHome->setChecked(settings.value("showHomeButton", true).toBool());
    ui->showBackForward->setChecked(settings.value("showBackForwardButtons", true).toBool());
    ui->showAddTabButton->setChecked(settings.value("showAddTabButton", false).toBool());
    ui->showReloadStopButtons->setChecked(settings.value("showReloadButton", true).toBool());
    ui->showWebSearchBar->setChecked(settings.value("showWebSearchBar", true).toBool());
    int currentSettingsPage = settings.value("settingsDialogPage", 0).toInt(0);
    settings.endGroup();

    //TABS
    settings.beginGroup("Browser-Tabs-Settings");
    ui->hideTabsOnTab->setChecked(settings.value("hideTabsWithOneTab", false).toBool());
    ui->activateLastTab->setChecked(settings.value("ActivateLastTabWhenClosingActual", false).toBool());
    ui->openNewTabAfterActive->setChecked(settings.value("newTabAfterActive", true).toBool());
    ui->openNewEmptyTabAfterActive->setChecked(settings.value("newEmptyTabAfterActive", false).toBool());
    ui->openPopupsInTabs->setChecked(settings.value("OpenPopupsInTabs", false).toBool());
    ui->alwaysSwitchTabsWithWheel->setChecked(settings.value("AlwaysSwitchTabsWithWheel", false).toBool());
    ui->switchToNewTabs->setChecked(settings.value("OpenNewTabsSelected", false).toBool());
    ui->dontCloseOnLastTab->setChecked(settings.value("dontCloseWithOneTab", false).toBool());
    ui->askWhenClosingMultipleTabs->setChecked(settings.value("AskOnClosing", false).toBool());
    ui->showClosedTabsButton->setChecked(settings.value("showClosedTabsButton", false).toBool());
    ui->showCloseOnInactive->setCurrentIndex(settings.value("showCloseOnInactiveTabs", 0).toInt());
    settings.endGroup();

    //AddressBar
    settings.beginGroup("AddressBar");
    ui->addressbarCompletion->setCurrentIndex(settings.value("showSuggestions", 0).toInt());
    ui->useInlineCompletion->setChecked(settings.value("useInlineCompletion", true).toBool());
    ui->completionShowSwitchTab->setChecked(settings.value("showSwitchTab", true).toBool());
    ui->alwaysShowGoIcon->setChecked(settings.value("alwaysShowGoIcon", false).toBool());
    ui->selectAllOnFocus->setChecked(settings.value("SelectAllTextOnDoubleClick", true).toBool());
    ui->selectAllOnClick->setChecked(settings.value("SelectAllTextOnClick", false).toBool());
    bool showPBinAB = settings.value("ShowLoadingProgress", false).toBool();
    ui->showLoadingInAddressBar->setChecked(showPBinAB);
    ui->adressProgressSettings->setEnabled(showPBinAB);
    ui->progressStyleSelector->setCurrentIndex(settings.value("ProgressStyle", 0).toInt());
    bool pbInABuseCC = settings.value("UseCustomProgressColor", false).toBool();
    ui->checkBoxCustomProgressColor->setChecked(pbInABuseCC);
    ui->progressBarColorSelector->setEnabled(pbInABuseCC);
    QColor pbColor = settings.value("CustomProgressColor", palette().color(QPalette::Highlight)).value<QColor>();
    setProgressBarColorIcon(pbColor);
    connect(ui->customColorToolButton, SIGNAL(clicked(bool)), SLOT(selectCustomProgressBarColor()));
    connect(ui->resetProgressBarcolor, SIGNAL(clicked()), SLOT(setProgressBarColorIcon()));
    settings.endGroup();

    settings.beginGroup("SearchEngines");
    bool searchFromAB = settings.value("SearchFromAddressBar", true).toBool();
    ui->searchFromAddressBar->setChecked(searchFromAB);
    ui->searchWithDefaultEngine->setEnabled(searchFromAB);
    ui->searchWithDefaultEngine->setChecked(settings.value("SearchWithDefaultEngine", false).toBool());
    connect(ui->searchFromAddressBar, SIGNAL(toggled(bool)), this, SLOT(searchFromAddressBarChanged(bool)));
    settings.endGroup();

    // BROWSING
    settings.beginGroup("Web-Browser-Settings");
    ui->allowPlugins->setChecked(settings.value("allowPlugins", true).toBool());
    ui->allowJavaScript->setChecked(settings.value("allowJavaScript", true).toBool());
    ui->linksInFocusChain->setChecked(settings.value("IncludeLinkInFocusChain", false).toBool());
    ui->spatialNavigation->setChecked(settings.value("SpatialNavigation", false).toBool());
    ui->animateScrolling->setChecked(settings.value("AnimateScrolling", true).toBool());
    ui->wheelScroll->setValue(settings.value("wheelScrollLines", qApp->wheelScrollLines()).toInt());
    ui->xssAuditing->setChecked(settings.value("XSSAuditing", false).toBool());

    foreach (int level, WebView::zoomLevels()) {
        ui->defaultZoomLevel->addItem(QString("%1%").arg(level));
    }
    ui->defaultZoomLevel->setCurrentIndex(settings.value("DefaultZoomLevel", WebView::zoomLevels().indexOf(100)).toInt());
    ui->closeAppWithCtrlQ->setChecked(settings.value("closeAppWithCtrlQ", true).toBool());

    //Cache
    ui->allowCache->setChecked(settings.value("AllowLocalCache", true).toBool());
    ui->cacheMB->setValue(settings.value("LocalCacheSize", 50).toInt());
    ui->MBlabel->setText(settings.value("LocalCacheSize", 50).toString() + " MB");
    ui->cachePath->setText(settings.value("CachePath", QWebEngineProfile::defaultProfile()->cachePath()).toString());
    connect(ui->allowCache, SIGNAL(clicked(bool)), this, SLOT(allowCacheChanged(bool)));
    connect(ui->cacheMB, SIGNAL(valueChanged(int)), this, SLOT(cacheValueChanged(int)));
    connect(ui->changeCachePath, SIGNAL(clicked()), this, SLOT(changeCachePathClicked()));
    allowCacheChanged(ui->allowCache->isChecked());

    //PASSWORD MANAGER
    ui->allowPassManager->setChecked(settings.value("SavePasswordsOnSites", true).toBool());
    connect(ui->allowPassManager, SIGNAL(toggled(bool)), this, SLOT(showPassManager(bool)));

    showPassManager(ui->allowPassManager->isChecked());

    //PRIVACY
    //Web storage
    ui->saveHistory->setChecked(settings.value("allowHistory", true).toBool());
    ui->deleteHistoryOnClose->setChecked(settings.value("deleteHistoryOnClose", false).toBool());
    if (!ui->saveHistory->isChecked()) {
        ui->deleteHistoryOnClose->setEnabled(false);
    }
    connect(ui->saveHistory, SIGNAL(toggled(bool)), this, SLOT(saveHistoryChanged(bool)));

    // Html5Storage
    ui->html5storage->setChecked(settings.value("HTML5StorageEnabled", true).toBool());
    ui->deleteHtml5storageOnClose->setChecked(settings.value("deleteHTML5StorageOnClose", false).toBool());
    connect(ui->html5storage, SIGNAL(toggled(bool)), this, SLOT(allowHtml5storageChanged(bool)));
    // Other
    ui->doNotTrack->setChecked(settings.value("DoNotTrack", false).toBool());

    //CSS Style
    ui->userStyleSheet->setText(settings.value("userStyleSheet", "").toString());
    connect(ui->chooseUserStylesheet, SIGNAL(clicked()), this, SLOT(chooseUserStyleClicked()));
    settings.endGroup();

    //DOWNLOADS
    settings.beginGroup("DownloadManager");
    ui->downLoc->setText(settings.value("defaultDownloadPath", "").toString());
    ui->closeDownManOnFinish->setChecked(settings.value("CloseManagerOnFinish", false).toBool());
    if (ui->downLoc->text().isEmpty()) {
        ui->askEverytime->setChecked(true);
    }
    else {
        ui->useDefined->setChecked(true);
    }
    ui->useExternalDownManager->setChecked(settings.value("UseExternalManager", false).toBool());
    ui->externalDownExecutable->setText(settings.value("ExternalManagerExecutable", "").toString());
    ui->externalDownArguments->setText(settings.value("ExternalManagerArguments", "").toString());

    connect(ui->useExternalDownManager, SIGNAL(toggled(bool)), this, SLOT(useExternalDownManagerChanged(bool)));


    connect(ui->useDefined, SIGNAL(toggled(bool)), this, SLOT(downLocChanged(bool)));
    connect(ui->downButt, SIGNAL(clicked()), this, SLOT(chooseDownPath()));
    connect(ui->chooseExternalDown, SIGNAL(clicked()), this, SLOT(chooseExternalDownloadManager()));
    downLocChanged(ui->useDefined->isChecked());
    useExternalDownManagerChanged(ui->useExternalDownManager->isChecked());
    settings.endGroup();

    //FONTS
    settings.beginGroup("Browser-Fonts");
    QWebEngineSettings* webSettings = QWebEngineSettings::defaultSettings();
    auto defaultFont = [&](QWebEngineSettings::FontFamily font) -> const QString {
        const QString family = webSettings->fontFamily(font);
        if (!family.isEmpty())
            return family;
        switch (font) {
        case QWebEngineSettings::FixedFont:
            return QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
        case QWebEngineSettings::SerifFont:
            // TODO
        default:
            return QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
        }
    };
    ui->fontStandard->setCurrentFont(QFont(settings.value("StandardFont", defaultFont(QWebEngineSettings::StandardFont)).toString()));
    ui->fontCursive->setCurrentFont(QFont(settings.value("CursiveFont", defaultFont(QWebEngineSettings::CursiveFont)).toString()));
    ui->fontFantasy->setCurrentFont(QFont(settings.value("FantasyFont", defaultFont(QWebEngineSettings::FantasyFont)).toString()));
    ui->fontFixed->setCurrentFont(QFont(settings.value("FixedFont", defaultFont(QWebEngineSettings::FixedFont)).toString()));
    ui->fontSansSerif->setCurrentFont(QFont(settings.value("SansSerifFont", defaultFont(QWebEngineSettings::SansSerifFont)).toString()));
    ui->fontSerif->setCurrentFont(QFont(settings.value("SerifFont", defaultFont(QWebEngineSettings::SerifFont)).toString()));
    ui->sizeDefault->setValue(settings.value("DefaultFontSize", webSettings->fontSize(QWebEngineSettings::DefaultFontSize)).toInt());
    ui->sizeFixed->setValue(settings.value("FixedFontSize", webSettings->fontSize(QWebEngineSettings::DefaultFixedFontSize)).toInt());
    ui->sizeMinimum->setValue(settings.value("MinimumFontSize", webSettings->fontSize(QWebEngineSettings::MinimumFontSize)).toInt());
    ui->sizeMinimumLogical->setValue(settings.value("MinimumLogicalFontSize", webSettings->fontSize(QWebEngineSettings::MinimumLogicalFontSize)).toInt());
    settings.endGroup();

    //KEYBOARD SHORTCUTS
    settings.beginGroup("Shortcuts");
    ui->switchTabsAlt->setChecked(settings.value("useTabNumberShortcuts", true).toBool());
    ui->loadSpeedDialsCtrl->setChecked(settings.value("useSpeedDialNumberShortcuts", true).toBool());
    ui->singleKeyShortcuts->setChecked(settings.value("useSingleKeyShortcuts", false).toBool());
    settings.endGroup();

    //NOTIFICATIONS
    ui->useNativeSystemNotifications->setEnabled(mApp->desktopNotifications()->supportsNativeNotifications());

    DesktopNotificationsFactory::Type notifyType;
    settings.beginGroup("Notifications");
    ui->notificationTimeout->setValue(settings.value("Timeout", 6000).toInt() / 1000);
#if defined(Q_OS_UNIX) && !defined(DISABLE_DBUS)
    notifyType = settings.value("UseNativeDesktop", true).toBool() ? DesktopNotificationsFactory::DesktopNative : DesktopNotificationsFactory::PopupWidget;
#else
    notifyType = DesktopNotificationsFactory::PopupWidget;
#endif
    if (ui->useNativeSystemNotifications->isEnabled() && notifyType == DesktopNotificationsFactory::DesktopNative) {
        ui->useNativeSystemNotifications->setChecked(true);
    }
    else {
        ui->useOSDNotifications->setChecked(true);
    }

    connect(ui->useNativeSystemNotifications, SIGNAL(toggled(bool)), this, SLOT(setNotificationPreviewVisible(bool)));
    connect(ui->useOSDNotifications, SIGNAL(toggled(bool)), this, SLOT(setNotificationPreviewVisible(bool)));

    ui->doNotUseNotifications->setChecked(!settings.value("Enabled", true).toBool());
    m_notifPosition = settings.value("Position", QPoint(10, 10)).toPoint();
    settings.endGroup();

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    //SPELLCHECK
    settings.beginGroup(QSL("SpellCheck"));
    ui->spellcheckEnabled->setChecked(settings.value(QSL("Enabled"), false).toBool());
    const QString spellcheckLanguage = settings.value(QSL("Language")).toString();
    settings.endGroup();

    const QStringList dictionariesDirs = {
        QCoreApplication::applicationDirPath() + QL1S("/qtwebengine_dictionaries"),
        QLibraryInfo::location(QLibraryInfo::DataPath) + QL1S("/qtwebengine_dictionaries")
    };

    for (const QString &path : dictionariesDirs) {
        QDir dir(path);
        const QStringList files = dir.entryList({QSL("*.bdic")});
        for (const QString &file : files) {
            const QString lang = file.left(file.size() - 5);
            ui->spellcheckLanguage->addItem(createLanguageItem(lang), lang);
            if (lang == spellcheckLanguage) {
                ui->spellcheckLanguage->setCurrentIndex(ui->spellcheckLanguage->count() - 1);
            }
        }
    }
#else
    delete ui->listWidget->item(11);
    delete ui->stackedWidget->widget(11);
#endif

    //OTHER
    //Languages
    QString activeLanguage = mApp->currentLanguage();

    if (!activeLanguage.isEmpty() && activeLanguage != QLatin1String("en_US")) {
        ui->languages->addItem(createLanguageItem(activeLanguage), activeLanguage);
    }

    ui->languages->addItem("English (en_US)");

    const QStringList translationPaths = DataPaths::allPaths(DataPaths::Translations);

    foreach (const QString &path, translationPaths) {
        QDir lanDir(path);
        QStringList list = lanDir.entryList(QStringList("*.qm"));
        foreach (const QString &name, list) {
            if (name.startsWith(QLatin1String("qt_"))) {
                continue;
            }

            QString loc = name;
            loc.remove(QLatin1String(".qm"));

            if (loc == activeLanguage) {
                continue;
            }

            ui->languages->addItem(createLanguageItem(loc), loc);
        }
    }

    // Proxy Configuration
    settings.beginGroup("Web-Proxy");
    QNetworkProxy::ProxyType proxyType = QNetworkProxy::ProxyType(settings.value("ProxyType", QNetworkProxy::NoProxy).toInt());

    ui->systemProxy->setChecked(proxyType == QNetworkProxy::NoProxy);
    ui->manualProxy->setChecked(proxyType != QNetworkProxy::NoProxy);
    if (proxyType == QNetworkProxy::Socks5Proxy) {
        ui->proxyType->setCurrentIndex(1);
    }
    else {
        ui->proxyType->setCurrentIndex(0);
    }

    ui->proxyServer->setText(settings.value("HostName", "").toString());
    ui->proxyPort->setText(settings.value("Port", 8080).toString());
    ui->proxyUsername->setText(settings.value("Username", "").toString());
    ui->proxyPassword->setText(settings.value("Password", "").toString());
    settings.endGroup();

    setManualProxyConfigurationEnabled(proxyType != QNetworkProxy::NoProxy);

    connect(ui->manualProxy, SIGNAL(toggled(bool)), this, SLOT(setManualProxyConfigurationEnabled(bool)));

    //CONNECTS
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
    connect(ui->cookieManagerBut, SIGNAL(clicked()), this, SLOT(showCookieManager()));
    connect(ui->html5permissions, SIGNAL(clicked()), this, SLOT(showHtml5Permissions()));
    connect(ui->preferredLanguages, SIGNAL(clicked()), this, SLOT(showAcceptLanguage()));
    connect(ui->deleteHtml5storage, SIGNAL(clicked()), this, SLOT(deleteHtml5storage()));
    connect(ui->uaManager, SIGNAL(clicked()), this, SLOT(openUserAgentManager()));
    connect(ui->jsOptionsButton, SIGNAL(clicked()), this, SLOT(openJsOptions()));
    connect(ui->searchEngines, SIGNAL(clicked()), this, SLOT(openSearchEnginesManager()));

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(showStackedPage(QListWidgetItem*)));
    ui->listWidget->setItemSelected(ui->listWidget->itemAt(5, 5), true);

    ui->version->setText(QSL(" QupZilla v") + QL1S(Qz::VERSION));
    ui->listWidget->setCurrentRow(currentSettingsPage);

    QDesktopWidget* desktop = QApplication::desktop();
    QSize s = size();
    if (desktop->availableGeometry(this).size().width() < s.width()) {
        s.setWidth(desktop->availableGeometry(this).size().width() - 50);
    }
    if (desktop->availableGeometry(this).size().height() < s.height()) {
        s.setHeight(desktop->availableGeometry(this).size().height() - 50);
    }
    resize(s);

    settings.beginGroup(QSL("Preferences"));
    restoreGeometry(settings.value(QSL("Geometry")).toByteArray());
    settings.endGroup();

    QzTools::setWmClass("Preferences", this);
}

void Preferences::chooseExternalDownloadManager()
{
    QString path = QzTools::getOpenFileName("Preferences-ExternalDownloadManager", this, tr("Choose executable location..."), QDir::homePath());
    if (path.isEmpty()) {
        return;
    }

    ui->externalDownExecutable->setText(path);
}

void Preferences::showStackedPage(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    int index = ui->listWidget->currentRow();

    ui->caption->setText("<b>" + item->text() + "</b>");
    ui->stackedWidget->setCurrentIndex(index);

    setNotificationPreviewVisible(index == 9);

    if (index == 10) {
        m_pluginsList->load();
    }

    if (index == 7 && !m_autoFillManager) {
        m_autoFillManager = new AutoFillManager(this);
        ui->autoFillFrame->addWidget(m_autoFillManager);
        m_autoFillManager->setVisible(m_autoFillEnabled);
    }
}

void Preferences::setNotificationPreviewVisible(bool state)
{
    if (!state && m_notification) {
        m_notifPosition = m_notification.data()->pos();
        delete m_notification.data();
    }

    if (state) {
        if (ui->useOSDNotifications->isChecked()) {
            if (m_notification) {
                m_notifPosition = m_notification.data()->pos();
                delete m_notification.data();
            }

            m_notification = new DesktopNotification(true);
            m_notification.data()->setPixmap(QPixmap(":icons/preferences/stock_dialog-question.png"));
            m_notification.data()->setHeading(tr("OSD Notification"));
            m_notification.data()->setText(tr("Drag it on the screen to place it where you want."));
            m_notification.data()->move(m_notifPosition);
            m_notification.data()->show();
        }
        else if (ui->useNativeSystemNotifications->isChecked()) {
            mApp->desktopNotifications()->nativeNotificationPreview();
        }
    }
}

void Preferences::makeQupZillaDefault()
{
#if defined(Q_OS_WIN) && !defined(Q_OS_OS2)
    disconnect(ui->checkNowDefaultBrowser, SIGNAL(clicked()), this, SLOT(makeQupZillaDefault()));
    mApp->associationManager()->registerAllAssociation();
    ui->checkNowDefaultBrowser->setText(tr("Default"));
    ui->checkNowDefaultBrowser->setEnabled(false);
#endif
}

void Preferences::allowCacheChanged(bool state)
{
    ui->cacheFrame->setEnabled(state);
    ui->cacheMB->setEnabled(state);
    ui->storeCacheLabel->setEnabled(state);
    ui->cachePath->setEnabled(state);
    ui->changeCachePath->setEnabled(state);
}

void Preferences::useActualHomepage()
{
    if (!m_window)
        return;

    ui->homepage->setText(m_window->weView()->url().toString());
}

void Preferences::useActualNewTab()
{
    if (!m_window)
        return;

    ui->newTabUrl->setText(m_window->weView()->url().toString());
}

void Preferences::chooseDownPath()
{
    QString userFileName = QzTools::getExistingDirectory("Preferences-ChooseDownPath", this, tr("Choose download location..."), QDir::homePath());
    if (userFileName.isEmpty()) {
        return;
    }
#ifdef Q_OS_WIN   //QFileDialog::getExistingDirectory returns path with \ instead of / (??)
    userFileName.replace(QLatin1Char('\\'), QLatin1Char('/'));
#endif
    userFileName += QLatin1Char('/');

    ui->downLoc->setText(userFileName);
}

void Preferences::chooseUserStyleClicked()
{
    QString file = QzTools::getOpenFileName("Preferences-UserStyle", this, tr("Choose stylesheet location..."), QDir::homePath(), "*.css");
    if (file.isEmpty()) {
        return;
    }
    ui->userStyleSheet->setText(file);
}

void Preferences::deleteHtml5storage()
{
    ClearPrivateData::clearLocalStorage();

    ui->deleteHtml5storage->setText(tr("Deleted"));
    ui->deleteHtml5storage->setEnabled(false);
}

void Preferences::openUserAgentManager()
{
    UserAgentDialog* dialog = new UserAgentDialog(this);
    dialog->open();
}

void Preferences::downLocChanged(bool state)
{
    ui->downButt->setEnabled(state);
    ui->downLoc->setEnabled(state);
}

void Preferences::setManualProxyConfigurationEnabled(bool state)
{
    ui->proxyType->setEnabled(state);
    ui->proxyServer->setEnabled(state);
    ui->proxyPort->setEnabled(state);
    ui->proxyUsername->setEnabled(state);
    ui->proxyPassword->setEnabled(state);
}

void Preferences::searchFromAddressBarChanged(bool stat)
{
    ui->searchWithDefaultEngine->setEnabled(stat);
}

void Preferences::saveHistoryChanged(bool stat)
{
    ui->deleteHistoryOnClose->setEnabled(stat);
}

void Preferences::allowHtml5storageChanged(bool stat)
{
    ui->deleteHtml5storageOnClose->setEnabled(stat);
}

void Preferences::showCookieManager()
{
    CookieManager* dialog = new CookieManager();
    dialog->show();
}

void Preferences::showHtml5Permissions()
{
    HTML5PermissionsDialog* dialog = new HTML5PermissionsDialog(this);
    dialog->open();
}

void Preferences::openJsOptions()
{
    JsOptions* dialog = new JsOptions(this);
    dialog->open();
}

void Preferences::useExternalDownManagerChanged(bool state)
{
    ui->externalDownExecutable->setEnabled(state);
    ui->externalDownArguments->setEnabled(state);
    ui->chooseExternalDown->setEnabled(state);
}

void Preferences::openSearchEnginesManager()
{
    SearchEnginesDialog* dialog = new SearchEnginesDialog(this);
    dialog->open();
}

void Preferences::showAcceptLanguage()
{
    AcceptLanguage* dialog = new AcceptLanguage(this);
    dialog->open();
}

void Preferences::newTabChanged(int value)
{
    ui->newTabFrame->setVisible(value == 3);
}

void Preferences::afterLaunchChanged(int value)
{
    ui->dontLoadTabsUntilSelected->setEnabled(value == 3);
}

void Preferences::cacheValueChanged(int value)
{
    ui->MBlabel->setText(QString::number(value) + " MB");
}

void Preferences::changeCachePathClicked()
{
    QString path = QzTools::getExistingDirectory("Preferences-CachePath", this, tr("Choose cache path..."), ui->cachePath->text());
    if (path.isEmpty()) {
        return;
    }

    ui->cachePath->setText(path);
}

void Preferences::showPassManager(bool state)
{
    if (m_autoFillManager) {
        m_autoFillManager->setVisible(state);
    }
    else {
        m_autoFillEnabled = state;
    }
}

void Preferences::buttonClicked(QAbstractButton* button)
{
    switch (ui->buttonBox->buttonRole(button)) {
    case QDialogButtonBox::ApplyRole:
        saveSettings();
        break;

    case QDialogButtonBox::RejectRole:
        close();
        break;

    case QDialogButtonBox::AcceptRole:
        saveSettings();
        close();
        break;

    default:
        break;
    }
}

void Preferences::createProfile()
{
    QString name = QInputDialog::getText(this, tr("New Profile"), tr("Enter the new profile's name:"));
    name = QzTools::filterCharsFromFilename(name);

    if (name.isEmpty()) {
        return;
    }

    int res = ProfileManager::createProfile(name);

    if (res == -1) {
        QMessageBox::warning(this, tr("Error!"), tr("This profile already exists!"));
        return;
    }

    if (res != 0) {
        QMessageBox::warning(this, tr("Error!"), tr("Cannot create profile directory!"));
        return;
    }

    ui->startProfile->addItem(name);
    ui->startProfile->setCurrentIndex(ui->startProfile->count() - 1);
}

void Preferences::deleteProfile()
{
    QString name = ui->startProfile->currentText();
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                                         tr("Are you sure you want to permanently delete \"%1\" profile? This action cannot be undone!").arg(name), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) {
        return;
    }

    ProfileManager::removeProfile(name);

    ui->startProfile->removeItem(ui->startProfile->currentIndex());
}

void Preferences::startProfileIndexChanged(int index)
{
    // Index 0 is current profile

    ui->deleteProfile->setEnabled(index != 0);

    if (index == 0) {
        ui->cannotDeleteActiveProfileLabel->setText(tr("Note: You cannot delete active profile."));
    }
    else {
        ui->cannotDeleteActiveProfileLabel->setText(" ");
    }
}

void Preferences::closeEvent(QCloseEvent* event)
{
    Settings settings;
    settings.beginGroup("Browser-View-Settings");
    settings.setValue("settingsDialogPage", ui->stackedWidget->currentIndex());
    settings.endGroup();

    event->accept();
}

void Preferences::saveSettings()
{
    Settings settings;
    //GENERAL URLs
    QUrl homepage = QUrl::fromUserInput(ui->homepage->text());

    settings.beginGroup("Web-URL-Settings");
    settings.setValue("homepage", homepage);
    settings.setValue("afterLaunch", ui->afterLaunch->currentIndex());

    switch (ui->newTab->currentIndex()) {
    case 0:
        settings.setValue("newTabUrl", QUrl(QSL("about:blank")));
        break;

    case 1:
        settings.setValue("newTabUrl", homepage);
        break;

    case 2:
        settings.setValue("newTabUrl", QUrl(QSL("qupzilla:speeddial")));
        break;

    case 3:
        settings.setValue("newTabUrl", QUrl::fromUserInput(ui->newTabUrl->text()));
        break;

    default:
        break;
    }

    settings.endGroup();
    //PROFILES
    /*
     *
     *
     *
     */

    //WINDOW
    settings.beginGroup("Browser-View-Settings");
    settings.setValue("showStatusBar", ui->showStatusbar->isChecked());
    settings.setValue("instantBookmarksToolbar", ui->instantBookmarksToolbar->isChecked());
    settings.setValue("showBookmarksToolbar", ui->showBookmarksToolbar->isChecked());
    settings.setValue("showNavigationToolbar", ui->showNavigationToolbar->isChecked());
    settings.setValue("showHomeButton", ui->showHome->isChecked());
    settings.setValue("showBackForwardButtons", ui->showBackForward->isChecked());
    settings.setValue("showWebSearchBar", ui->showWebSearchBar->isChecked());
    settings.setValue("showAddTabButton", ui->showAddTabButton->isChecked());
    settings.setValue("showReloadButton", ui->showReloadStopButtons->isChecked());
    settings.endGroup();

    //TABS
    settings.beginGroup("Browser-Tabs-Settings");
    settings.setValue("hideTabsWithOneTab", ui->hideTabsOnTab->isChecked());
    settings.setValue("ActivateLastTabWhenClosingActual", ui->activateLastTab->isChecked());
    settings.setValue("newTabAfterActive", ui->openNewTabAfterActive->isChecked());
    settings.setValue("newEmptyTabAfterActive", ui->openNewEmptyTabAfterActive->isChecked());
    settings.setValue("OpenPopupsInTabs", ui->openPopupsInTabs->isChecked());
    settings.setValue("AlwaysSwitchTabsWithWheel", ui->alwaysSwitchTabsWithWheel->isChecked());
    settings.setValue("OpenNewTabsSelected", ui->switchToNewTabs->isChecked());
    settings.setValue("dontCloseWithOneTab", ui->dontCloseOnLastTab->isChecked());
    settings.setValue("AskOnClosing", ui->askWhenClosingMultipleTabs->isChecked());
    settings.setValue("showClosedTabsButton", ui->showClosedTabsButton->isChecked());
    settings.setValue("showCloseOnInactiveTabs", ui->showCloseOnInactive->currentIndex());
    settings.endGroup();

    //DOWNLOADS
    settings.beginGroup("DownloadManager");
    if (ui->askEverytime->isChecked()) {
        settings.setValue("defaultDownloadPath", "");
    }
    else {
        settings.setValue("defaultDownloadPath", ui->downLoc->text());
    }
    settings.setValue("CloseManagerOnFinish", ui->closeDownManOnFinish->isChecked());
    settings.setValue("UseExternalManager", ui->useExternalDownManager->isChecked());
    settings.setValue("ExternalManagerExecutable", ui->externalDownExecutable->text());
    settings.setValue("ExternalManagerArguments", ui->externalDownArguments->text());

    settings.endGroup();

    //FONTS
    settings.beginGroup("Browser-Fonts");
    settings.setValue("StandardFont", ui->fontStandard->currentFont().family());
    settings.setValue("CursiveFont", ui->fontCursive->currentFont().family());
    settings.setValue("FantasyFont", ui->fontFantasy->currentFont().family());
    settings.setValue("FixedFont", ui->fontFixed->currentFont().family());
    settings.setValue("SansSerifFont", ui->fontSansSerif->currentFont().family());
    settings.setValue("SerifFont", ui->fontSerif->currentFont().family());

    settings.setValue("DefaultFontSize", ui->sizeDefault->value());
    settings.setValue("FixedFontSize", ui->sizeFixed->value());
    settings.setValue("MinimumFontSize", ui->sizeMinimum->value());
    settings.setValue("MinimumLogicalFontSize", ui->sizeMinimumLogical->value());
    settings.endGroup();

    //KEYBOARD SHORTCUTS
    settings.beginGroup("Shortcuts");
    settings.setValue("useTabNumberShortcuts", ui->switchTabsAlt->isChecked());
    settings.setValue("useSpeedDialNumberShortcuts", ui->loadSpeedDialsCtrl->isChecked());
    settings.setValue("useSingleKeyShortcuts", ui->singleKeyShortcuts->isChecked());
    settings.endGroup();

    //BROWSING
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("allowPlugins", ui->allowPlugins->isChecked());
    settings.setValue("allowJavaScript", ui->allowJavaScript->isChecked());
    settings.setValue("IncludeLinkInFocusChain", ui->linksInFocusChain->isChecked());
    settings.setValue("SpatialNavigation", ui->spatialNavigation->isChecked());
    settings.setValue("AnimateScrolling", ui->animateScrolling->isChecked());
    settings.setValue("wheelScrollLines", ui->wheelScroll->value());
    settings.setValue("DoNotTrack", ui->doNotTrack->isChecked());
    settings.setValue("CheckUpdates", ui->checkUpdates->isChecked());
    settings.setValue("LoadTabsOnActivation", ui->dontLoadTabsUntilSelected->isChecked());
    settings.setValue("DefaultZoomLevel", ui->defaultZoomLevel->currentIndex());
    settings.setValue("XSSAuditing", ui->xssAuditing->isChecked());
    settings.setValue("closeAppWithCtrlQ", ui->closeAppWithCtrlQ->isChecked());
#ifdef Q_OS_WIN
    settings.setValue("CheckDefaultBrowser", ui->checkDefaultBrowser->isChecked());
#endif
    //Cache
    settings.setValue("AllowLocalCache", ui->allowCache->isChecked());
    settings.setValue("LocalCacheSize", ui->cacheMB->value());
    settings.setValue("CachePath", ui->cachePath->text());
    //CSS Style
    settings.setValue("userStyleSheet", ui->userStyleSheet->text());

    //PASSWORD MANAGER
    settings.setValue("SavePasswordsOnSites", ui->allowPassManager->isChecked());

    //PRIVACY
    //Web storage
    settings.setValue("allowHistory", ui->saveHistory->isChecked());
    settings.setValue("deleteHistoryOnClose", ui->deleteHistoryOnClose->isChecked());
    settings.setValue("HTML5StorageEnabled", ui->html5storage->isChecked());
    settings.setValue("deleteHTML5StorageOnClose", ui->deleteHtml5storageOnClose->isChecked());
    settings.endGroup();

    //NOTIFICATIONS
    settings.beginGroup("Notifications");
    settings.setValue("Timeout", ui->notificationTimeout->value() * 1000);
    settings.setValue("Enabled", !ui->doNotUseNotifications->isChecked());
    settings.setValue("UseNativeDesktop", ui->useNativeSystemNotifications->isChecked());
    settings.setValue("Position", m_notification.data() ? m_notification.data()->pos() : m_notifPosition);
    settings.endGroup();

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    //SPELLCHECK
    settings.beginGroup(QSL("SpellCheck"));
    settings.setValue("Enabled", ui->spellcheckEnabled->isChecked());
    settings.setValue("Language", ui->spellcheckLanguage->currentData().toString());
    settings.endGroup();
#endif

    //OTHER
    //AddressBar
    settings.beginGroup("AddressBar");
    settings.setValue("showSuggestions", ui->addressbarCompletion->currentIndex());
    settings.setValue("useInlineCompletion", ui->useInlineCompletion->isChecked());
    settings.setValue("alwaysShowGoIcon", ui->alwaysShowGoIcon->isChecked());
    settings.setValue("showSwitchTab", ui->completionShowSwitchTab->isChecked());
    settings.setValue("SelectAllTextOnDoubleClick", ui->selectAllOnFocus->isChecked());
    settings.setValue("SelectAllTextOnClick", ui->selectAllOnClick->isChecked());
    settings.setValue("ShowLoadingProgress", ui->showLoadingInAddressBar->isChecked());
    settings.setValue("ProgressStyle", ui->progressStyleSelector->currentIndex());
    settings.setValue("UseCustomProgressColor", ui->checkBoxCustomProgressColor->isChecked());
    settings.setValue("CustomProgressColor", ui->customColorToolButton->property("ProgressColor").value<QColor>());
    settings.endGroup();

    settings.beginGroup("SearchEngines");
    settings.setValue("SearchFromAddressBar", ui->searchFromAddressBar->isChecked());
    settings.setValue("SearchWithDefaultEngine", ui->searchWithDefaultEngine->isChecked());
    settings.endGroup();

    //Languages
    settings.beginGroup("Language");
    settings.setValue("language", ui->languages->itemData(ui->languages->currentIndex()).toString());
    settings.endGroup();

    //Proxy Configuration
    QNetworkProxy::ProxyType proxyType;
    if (ui->systemProxy->isChecked()) {
        proxyType = QNetworkProxy::NoProxy;
    }
    else if (ui->proxyType->currentIndex() == 0) {
        proxyType = QNetworkProxy::HttpProxy;
    }
    else {
        proxyType = QNetworkProxy::Socks5Proxy;
    }

    settings.beginGroup("Web-Proxy");
    settings.setValue("ProxyType", proxyType);
    settings.setValue("HostName", ui->proxyServer->text());
    settings.setValue("Port", ui->proxyPort->text().toInt());
    settings.setValue("Username", ui->proxyUsername->text());
    settings.setValue("Password", ui->proxyPassword->text());
    settings.endGroup();

    ProfileManager::setStartingProfile(ui->startProfile->currentText());

    m_pluginsList->save();
    m_themesManager->save();
    mApp->cookieJar()->loadSettings();
    mApp->history()->loadSettings();
    mApp->reloadSettings();
    mApp->plugins()->c2f_saveSettings();
    mApp->desktopNotifications()->loadSettings();
    mApp->autoFill()->loadSettings();
    mApp->networkManager()->loadSettings();
}

Preferences::~Preferences()
{
    Settings().setValue(QSL("Preferences/Geometry"), saveGeometry());

    delete ui;
    delete m_autoFillManager;
    delete m_pluginsList;
    delete m_notification.data();
}

void Preferences::setProgressBarColorIcon(QColor color)
{
    const int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QPixmap pm(QSize(size, size));
    if (!color.isValid()) {
        color = palette().color(QPalette::Highlight);
    }
    pm.fill(color);
    ui->customColorToolButton->setIcon(pm);
    ui->customColorToolButton->setProperty("ProgressColor", color);
}

void Preferences::selectCustomProgressBarColor()
{
    QColor newColor = QColorDialog::getColor(ui->customColorToolButton->property("ProgressColor").value<QColor>(), this, tr("Select Color"));
    if (newColor.isValid()) {
        setProgressBarColorIcon(newColor);
    }
}
