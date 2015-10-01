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
#include "sslmanager.h"
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
#include "pac/pacmanager.h"
#include "searchenginesdialog.h"

#include <QSettings>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QColorDialog>
#include <QDesktopWidget>
#include <QNetworkDiskCache>

static QString createLanguageItem(const QString &lang)
{
    QLocale locale(lang);

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

Preferences::Preferences(BrowserWindow* window, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Preferences)
    , m_window(window)
    , m_autoFillManager(0)
    , m_pluginsList(0)
    , m_autoFillEnabled(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->languages->setLayoutDirection(Qt::LeftToRight);

    m_themesManager = new ThemeManager(ui->themesWidget, this);
    m_pluginsList = new PluginsManager(this);
    ui->pluginsFrame->addWidget(m_pluginsList);

#ifdef DISABLE_CHECK_UPDATES
    ui->checkUpdates->setVisible(false);
#endif

    if (QIcon::themeName().toLower() == QLatin1String("oxygen")) {
        ui->listWidget->item(0)->setIcon(QIcon::fromTheme("preferences-desktop", QIcon(":/icons/preferences/preferences-desktop.png")));
        ui->listWidget->item(1)->setIcon(QIcon::fromTheme("format-stroke-color", QIcon(":/icons/preferences/application-x-theme.png")));
        ui->listWidget->item(2)->setIcon(QIcon::fromTheme("tab-new-background", QIcon(":/icons/preferences/applications-internet.png")));
        ui->listWidget->item(3)->setIcon(QIcon::fromTheme("preferences-system-network", QIcon(":/icons/preferences/applications-webbrowsers.png")));
        ui->listWidget->item(4)->setIcon(QIcon::fromTheme("preferences-desktop-font", QIcon(":/icons/preferences/applications-fonts.png")));
        ui->listWidget->item(5)->setIcon(QIcon::fromTheme("configure-shortcuts", QIcon(":/icons/preferences/preferences-desktop-keyboard-shortcuts.png")));
        ui->listWidget->item(6)->setIcon(QIcon::fromTheme("download", QIcon(":/icons/preferences/mail-inbox.png")));
        ui->listWidget->item(7)->setIcon(QIcon::fromTheme("user-identity", QIcon(":/icons/preferences/dialog-password.png")));
        ui->listWidget->item(8)->setIcon(QIcon::fromTheme("preferences-system-firewall", QIcon(":/icons/preferences/preferences-system-firewall.png")));
        ui->listWidget->item(9)->setIcon(QIcon::fromTheme("preferences-desktop-notification", QIcon(":/icons/preferences/dialog-question.png")));
        ui->listWidget->item(10)->setIcon(QIcon::fromTheme("preferences-plugin", QIcon(":/icons/preferences/extension.png")));
        ui->listWidget->item(11)->setIcon(QIcon::fromTheme("applications-system", QIcon(":/icons/preferences/applications-system.png")));
    }
    else {
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
        ui->listWidget->item(11)->setIcon(QIcon::fromTheme("applications-system", QIcon(":/icons/preferences/applications-system.png")));
    }

    Settings settings;
    //GENERAL URLs
    settings.beginGroup("Web-URL-Settings");
    m_homepage = settings.value("homepage", QUrl(QSL("qupzilla:start"))).toUrl();
    m_newTabUrl = settings.value("newTabUrl", QUrl(QSL("qupzilla:speeddial"))).toUrl();
    ui->homepage->setText(m_homepage.toEncoded());
    ui->newTabUrl->setText(m_newTabUrl.toEncoded());
    int afterLaunch = settings.value("afterLaunch", 3).toInt();
    settings.endGroup();
    ui->afterLaunch->setCurrentIndex(afterLaunch);
    ui->checkUpdates->setChecked(settings.value("Web-Browser-Settings/CheckUpdates", DEFAULT_CHECK_UPDATES).toBool());
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
    ProfileManager profileManager;
    QString startingProfile = profileManager.startingProfile();
    ui->activeProfile->setText("<b>" + profileManager.currentProfile() + "</b>");
    ui->startProfile->addItem(startingProfile);

    foreach (const QString &name, profileManager.availableProfiles()) {
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
    ui->showStatusbar->setChecked(settings.value("showStatusBar", true).toBool());
    ui->showBookmarksToolbar->setChecked(settings.value("showBookmarksToolbar", true).toBool());
    ui->showNavigationToolbar->setChecked(settings.value("showNavigationToolbar", true).toBool());
    ui->showHome->setChecked(settings.value("showHomeButton", true).toBool());
    ui->showBackForward->setChecked(settings.value("showBackForwardButtons", true).toBool());
    ui->showAddTabButton->setChecked(settings.value("showAddTabButton", false).toBool());
    ui->showReloadStopButtons->setChecked(settings.value("showReloadButton", true).toBool());
    ui->showWebSearchBar->setChecked(settings.value("showWebSearchBar", true).toBool());
    ui->useTransparentBg->setChecked(settings.value("useTransparentBackground", false).toBool());
    int currentSettingsPage = settings.value("settingsDialogPage", 0).toInt(0);
    settings.endGroup();
#ifdef Q_OS_WIN
    ui->useTransparentBg->setEnabled(QtWin::isCompositionEnabled());
#endif

    //TABS
    settings.beginGroup("Browser-Tabs-Settings");
    ui->hideTabsOnTab->setChecked(settings.value("hideTabsWithOneTab", false).toBool());
    ui->activateLastTab->setChecked(settings.value("ActivateLastTabWhenClosingActual", false).toBool());
    ui->openNewTabAfterActive->setChecked(settings.value("newTabAfterActive", true).toBool());
    ui->openNewEmptyTabAfterActive->setChecked(settings.value("newEmptyTabAfterActive", false).toBool());
    ui->alwaysSwitchTabsWithWheel->setChecked(settings.value("AlwaysSwitchTabsWithWheel", false).toBool());
    ui->switchToNewTabs->setChecked(settings.value("OpenNewTabsSelected", false).toBool());
    ui->dontCloseOnLastTab->setChecked(settings.value("dontCloseWithOneTab", false).toBool());
    ui->askWhenClosingMultipleTabs->setChecked(settings.value("AskOnClosing", false).toBool());
    ui->showClosedTabsButton->setChecked(settings.value("showClosedTabsButton", false).toBool());
    ui->showTabPreviews->setChecked(settings.value("showTabPreviews", false).toBool());
    ui->animatedTabPreviews->setChecked(settings.value("tabPreviewAnimationsEnabled", true).toBool());
    ui->showCloseOnInactive->setCurrentIndex(settings.value("showCloseOnInactiveTabs", 0).toInt());
    settings.endGroup();

    connect(ui->showTabPreviews, SIGNAL(toggled(bool)), this, SLOT(showTabPreviewsChanged(bool)));
    showTabPreviewsChanged(ui->showTabPreviews->isChecked());

    //AddressBar
    settings.beginGroup("AddressBar");
    ui->addressbarCompletion->setCurrentIndex(settings.value("showSuggestions", 0).toInt());
    ui->useInlineCompletion->setChecked(settings.value("useInlineCompletion", true).toBool());
    ui->completionShowSwitchTab->setChecked(settings.value("showSwitchTab", true).toBool());
    ui->alwaysShowGoIcon->setChecked(settings.value("alwaysShowGoIcon", false).toBool());
    ui->selectAllOnFocus->setChecked(settings.value("SelectAllTextOnDoubleClick", true).toBool());
    ui->selectAllOnClick->setChecked(settings.value("SelectAllTextOnClick", false).toBool());
    ui->addCountryWithAlt->setChecked(settings.value("AddCountryDomainWithAltKey", true).toBool());
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
    ui->searchWithDefaultEngine->setChecked(settings.value("SearchWithDefaultEngine", false).toBool());
    settings.endGroup();

    // BROWSING
    settings.beginGroup("Web-Browser-Settings");
    ui->allowPlugins->setChecked(settings.value("allowFlash", true).toBool());
    ui->allowJavaScript->setChecked(settings.value("allowJavaScript", true).toBool());
    ui->allowJava->setChecked(settings.value("allowJava", true).toBool());
    ui->allowDNSPrefetch->setChecked(settings.value("DNS-Prefetch", false).toBool());
    ui->linksInFocusChain->setChecked(settings.value("IncludeLinkInFocusChain", false).toBool());
    ui->zoomTextOnly->setChecked(settings.value("zoomTextOnly", false).toBool());
    ui->spatialNavigation->setChecked(settings.value("SpatialNavigation", false).toBool());
    ui->caretBrowsing->setChecked(settings.value("CaretBrowsing", false).toBool());
    ui->animateScrolling->setChecked(settings.value("AnimateScrolling", true).toBool());
    ui->printEBackground->setChecked(settings.value("PrintElementBackground", true).toBool());
    ui->wheelScroll->setValue(settings.value("wheelScrollLines", qApp->wheelScrollLines()).toInt());
    ui->xssAuditing->setChecked(settings.value("XSSAuditing", false).toBool());
    ui->formsUndoRedo->setChecked(settings.value("enableFormsUndoRedo", false).toBool());

    foreach (int level, WebView::zoomLevels()) {
        ui->defaultZoomLevel->addItem(QString("%1%").arg(level));
    }
    ui->defaultZoomLevel->setCurrentIndex(settings.value("DefaultZoomLevel", WebView::zoomLevels().indexOf(100)).toInt());
    ui->closeAppWithCtrlQ->setChecked(settings.value("closeAppWithCtrlQ", true).toBool());

    //Cache
    ui->pagesInCache->setValue(settings.value("maximumCachedPages", 3).toInt());
    connect(ui->pagesInCache, SIGNAL(valueChanged(int)), this, SLOT(pageCacheValueChanged(int)));
    ui->pageCacheLabel->setText(QString::number(ui->pagesInCache->value()));

    ui->allowCache->setChecked(settings.value("AllowLocalCache", true).toBool());
    ui->cacheMB->setValue(settings.value("LocalCacheSize", 50).toInt());
    ui->MBlabel->setText(settings.value("LocalCacheSize", 50).toString() + " MB");
    ui->cachePath->setText(settings.value("CachePath", mApp->networkCache()->cacheDirectory()).toString());
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
    ui->sendReferer->setChecked(settings.value("SendReferer", true).toBool());

    //CSS Style
    ui->userStyleSheet->setText(settings.value("userStyleSheet", "").toString());
    connect(ui->chooseUserStylesheet, SIGNAL(clicked()), this, SLOT(chooseUserStyleClicked()));
    settings.endGroup();

    //DOWNLOADS
    settings.beginGroup("DownloadManager");
    ui->downLoc->setText(settings.value("defaultDownloadPath", "").toString());
    ui->closeDownManOnFinish->setChecked(settings.value("CloseManagerOnFinish", false).toBool());
    ui->downlaodNativeSystemDialog->setChecked(settings.value("useNativeDialog", DEFAULT_DOWNLOAD_USE_NATIVE_DIALOG).toBool());
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
    QWebSettings* webSettings = QWebSettings::globalSettings();
    ui->fontStandard->setCurrentFont(QFont(settings.value("StandardFont", webSettings->fontFamily(QWebSettings::StandardFont)).toString()));
    ui->fontCursive->setCurrentFont(QFont(settings.value("CursiveFont", webSettings->fontFamily(QWebSettings::CursiveFont)).toString()));
    ui->fontFantasy->setCurrentFont(QFont(settings.value("FantasyFont", webSettings->fontFamily(QWebSettings::FantasyFont)).toString()));
    ui->fontFixed->setCurrentFont(QFont(settings.value("FixedFont", webSettings->fontFamily(QWebSettings::FixedFont)).toString()));
    ui->fontSansSerif->setCurrentFont(QFont(settings.value("SansSerifFont", webSettings->fontFamily(QWebSettings::SansSerifFont)).toString()));
    ui->fontSerif->setCurrentFont(QFont(settings.value("SerifFont", webSettings->fontFamily(QWebSettings::SerifFont)).toString()));

    ui->sizeDefault->setValue(settings.value("DefaultFontSize", webSettings->fontSize(QWebSettings::DefaultFontSize)).toInt());
    ui->sizeFixed->setValue(settings.value("FixedFontSize", webSettings->fontSize(QWebSettings::DefaultFixedFontSize)).toInt());
    ui->sizeMinimum->setValue(settings.value("MinimumFontSize", webSettings->fontSize(QWebSettings::MinimumFontSize)).toInt());
    ui->sizeMinimumLogical->setValue(settings.value("MinimumLogicalFontSize", webSettings->fontSize(QWebSettings::MinimumLogicalFontSize)).toInt());
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
    NetworkProxyFactory::ProxyPreference proxyPreference = NetworkProxyFactory::ProxyPreference(settings.value("UseProxy", NetworkProxyFactory::SystemProxy).toInt());
    QNetworkProxy::ProxyType proxyType = QNetworkProxy::ProxyType(settings.value("ProxyType", QNetworkProxy::HttpProxy).toInt());

    ui->systemProxy->setChecked(proxyPreference == NetworkProxyFactory::SystemProxy);
    ui->noProxy->setChecked(proxyPreference == NetworkProxyFactory::NoProxy);
    ui->manualProxy->setChecked(proxyPreference == NetworkProxyFactory::DefinedProxy);
    ui->pacProxy->setChecked(proxyPreference == NetworkProxyFactory::ProxyAutoConfig);
    if (proxyType == QNetworkProxy::HttpProxy) {
        ui->proxyType->setCurrentIndex(0);
    }
    else {
        ui->proxyType->setCurrentIndex(1);
    }

    ui->proxyServer->setText(settings.value("HostName", "").toString());
    ui->proxyPort->setText(settings.value("Port", 8080).toString());
    ui->proxyUsername->setText(settings.value("Username", "").toString());
    ui->proxyPassword->setText(settings.value("Password", "").toString());

    ui->useHttpsProxy->setChecked(settings.value("UseDifferentProxyForHttps", false).toBool());
    ui->httpsProxyServer->setText(settings.value("HttpsHostName", "").toString());
    ui->httpsProxyPort->setText(settings.value("HttpsPort", 8080).toString());
    ui->httpsProxyUsername->setText(settings.value("HttpsUsername", "").toString());
    ui->httpsProxyPassword->setText(settings.value("HttpsPassword", "").toString());

    ui->pacUrl->setText(settings.value("PacUrl", QUrl()).toUrl().toString());
    ui->proxyExceptions->setText(settings.value("ProxyExceptions", QStringList() << "localhost" << "127.0.0.1").toStringList().join(","));
    settings.endGroup();

    useDifferentProxyForHttpsChanged(ui->useHttpsProxy->isChecked());
    setManualProxyConfigurationEnabled(proxyPreference == NetworkProxyFactory::DefinedProxy);
    setProxyAutoConfigEnabled(proxyPreference == NetworkProxyFactory::ProxyAutoConfig);

    connect(ui->manualProxy, SIGNAL(toggled(bool)), this, SLOT(setManualProxyConfigurationEnabled(bool)));
    connect(ui->pacProxy, SIGNAL(toggled(bool)), this, SLOT(setProxyAutoConfigEnabled(bool)));
    connect(ui->useHttpsProxy, SIGNAL(toggled(bool)), this, SLOT(useDifferentProxyForHttpsChanged(bool)));
    connect(ui->reloadPac, SIGNAL(clicked()), this, SLOT(reloadPacFileClicked()));

    //CONNECTS
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
    connect(ui->cookieManagerBut, SIGNAL(clicked()), this, SLOT(showCookieManager()));
    connect(ui->html5permissions, SIGNAL(clicked()), this, SLOT(showHtml5Permissions()));
    connect(ui->sslManagerButton, SIGNAL(clicked()), this, SLOT(openSslManager()));
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

#if QTWEBKIT_TO_2_3
    ui->caretBrowsing->setHidden(true);
    ui->animateScrolling->setHidden(true);
#endif

#if QTWEBKIT_TO_2_2
    ui->html5permissions->setDisabled(true);
#endif

    settings.beginGroup(QSL("Preferences"));
    restoreGeometry(settings.value(QSL("Geometry")).toByteArray());
    settings.endGroup();

    QzTools::setWmClass("Preferences", this);
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

void Preferences::chooseExternalDownloadManager()
{
    QString path = QzTools::getOpenFileName("Preferences-ExternalDownloadManager", this, tr("Choose executable location..."), QDir::homePath());
    if (path.isEmpty()) {
        return;
    }

    ui->externalDownExecutable->setText(path);
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

    useDifferentProxyForHttpsChanged(state ? ui->useHttpsProxy->isChecked() : false);

    ui->useHttpsProxy->setEnabled(state);
}

void Preferences::setProxyAutoConfigEnabled(bool state)
{
    ui->pacUrl->setEnabled(state);
    ui->reloadPac->setEnabled(state);
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
    CookieManager* dialog = new CookieManager(this);
    dialog->open();
}

void Preferences::showHtml5Permissions()
{
    HTML5PermissionsDialog* dialog = new HTML5PermissionsDialog(this);
    dialog->open();
}

void Preferences::openSslManager()
{
    SSLManager* m = new SSLManager(this);
    m->open();
}

void Preferences::openJsOptions()
{
    JsOptions* dialog = new JsOptions(this);
    dialog->open();
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

void Preferences::pageCacheValueChanged(int value)
{
    ui->pageCacheLabel->setText(QString::number(value));
}

void Preferences::useExternalDownManagerChanged(bool state)
{
    ui->externalDownExecutable->setEnabled(state);
    ui->externalDownArguments->setEnabled(state);
    ui->chooseExternalDown->setEnabled(state);
}

void Preferences::useDifferentProxyForHttpsChanged(bool state)
{
    ui->httpsProxyServer->setEnabled(state);
    ui->httpsProxyPort->setEnabled(state);
    ui->httpsProxyUsername->setEnabled(state);
    ui->httpsProxyPassword->setEnabled(state);
}

void Preferences::showTabPreviewsChanged(bool state)
{
    ui->animatedTabPreviews->setEnabled(state);
}

void Preferences::changeCachePathClicked()
{
    QString path = QzTools::getExistingDirectory("Preferences-CachePath", this, tr("Choose cache path..."), ui->cachePath->text());
    if (path.isEmpty()) {
        return;
    }

    ui->cachePath->setText(path);
}

void Preferences::reloadPacFileClicked()
{
    mApp->networkManager()->proxyFactory()->pacManager()->downloadPacFile();
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

    ProfileManager profileManager;
    int res = profileManager.createProfile(name);

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
                                         tr("Are you sure to permanently delete \"%1\" profile? This action cannot be undone!").arg(name), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) {
        return;
    }

    ProfileManager profileManager;
    profileManager.removeProfile(name);

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
    settings.setValue("showBookmarksToolbar", ui->showBookmarksToolbar->isChecked());
    settings.setValue("showNavigationToolbar", ui->showNavigationToolbar->isChecked());
    settings.setValue("showHomeButton", ui->showHome->isChecked());
    settings.setValue("showBackForwardButtons", ui->showBackForward->isChecked());
    settings.setValue("showWebSearchBar", ui->showWebSearchBar->isChecked());
    settings.setValue("useTransparentBackground", ui->useTransparentBg->isChecked());
    settings.setValue("showAddTabButton", ui->showAddTabButton->isChecked());
    settings.setValue("showReloadButton", ui->showReloadStopButtons->isChecked());
    settings.endGroup();

    //TABS
    settings.beginGroup("Browser-Tabs-Settings");
    settings.setValue("hideTabsWithOneTab", ui->hideTabsOnTab->isChecked());
    settings.setValue("ActivateLastTabWhenClosingActual", ui->activateLastTab->isChecked());
    settings.setValue("newTabAfterActive", ui->openNewTabAfterActive->isChecked());
    settings.setValue("newEmptyTabAfterActive", ui->openNewEmptyTabAfterActive->isChecked());
    settings.setValue("AlwaysSwitchTabsWithWheel", ui->alwaysSwitchTabsWithWheel->isChecked());
    settings.setValue("OpenNewTabsSelected", ui->switchToNewTabs->isChecked());
    settings.setValue("dontCloseWithOneTab", ui->dontCloseOnLastTab->isChecked());
    settings.setValue("AskOnClosing", ui->askWhenClosingMultipleTabs->isChecked());
    settings.setValue("showClosedTabsButton", ui->showClosedTabsButton->isChecked());
    settings.setValue("showTabPreviews", ui->showTabPreviews->isChecked());
    settings.setValue("tabPreviewAnimationsEnabled", ui->animatedTabPreviews->isChecked());
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
    settings.setValue("useNativeDialog", ui->downlaodNativeSystemDialog->isChecked());
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
    settings.setValue("allowFlash", ui->allowPlugins->isChecked());
    settings.setValue("allowJavaScript", ui->allowJavaScript->isChecked());
    settings.setValue("allowJava", ui->allowJava->isChecked());
    settings.setValue("DNS-Prefetch", ui->allowDNSPrefetch->isChecked());
    settings.setValue("IncludeLinkInFocusChain", ui->linksInFocusChain->isChecked());
    settings.setValue("zoomTextOnly", ui->zoomTextOnly->isChecked());
    settings.setValue("SpatialNavigation", ui->spatialNavigation->isChecked());
    settings.setValue("CaretBrowsing", ui->caretBrowsing->isChecked());
    settings.setValue("AnimateScrolling", ui->animateScrolling->isChecked());
    settings.setValue("PrintElementBackground", ui->printEBackground->isChecked());
    settings.setValue("wheelScrollLines", ui->wheelScroll->value());
    settings.setValue("DoNotTrack", ui->doNotTrack->isChecked());
    settings.setValue("CheckUpdates", ui->checkUpdates->isChecked());
    settings.setValue("LoadTabsOnActivation", ui->dontLoadTabsUntilSelected->isChecked());
    settings.setValue("DefaultZoomLevel", ui->defaultZoomLevel->currentIndex());
    settings.setValue("XSSAuditing", ui->xssAuditing->isChecked());
    settings.setValue("enableFormsUndoRedo", ui->formsUndoRedo->isChecked());
    settings.setValue("closeAppWithCtrlQ", ui->closeAppWithCtrlQ->isChecked());
#ifdef Q_OS_WIN
    settings.setValue("CheckDefaultBrowser", ui->checkDefaultBrowser->isChecked());
#endif
    //Cache
    settings.setValue("maximumCachedPages", ui->pagesInCache->value());
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
    settings.setValue("SendReferer", ui->sendReferer->isChecked());
    settings.endGroup();

    //NOTIFICATIONS
    settings.beginGroup("Notifications");
    settings.setValue("Timeout", ui->notificationTimeout->value() * 1000);
    settings.setValue("Enabled", !ui->doNotUseNotifications->isChecked());
    settings.setValue("UseNativeDesktop", ui->useNativeSystemNotifications->isChecked());
    settings.setValue("Position", m_notification.data() ? m_notification.data()->pos() : m_notifPosition);
    settings.endGroup();

    //OTHER
    //AddressBar
    settings.beginGroup("AddressBar");
    settings.setValue("showSuggestions", ui->addressbarCompletion->currentIndex());
    settings.setValue("useInlineCompletion", ui->useInlineCompletion->isChecked());
    settings.setValue("alwaysShowGoIcon", ui->alwaysShowGoIcon->isChecked());
    settings.setValue("showSwitchTab", ui->completionShowSwitchTab->isChecked());
    settings.setValue("SelectAllTextOnDoubleClick", ui->selectAllOnFocus->isChecked());
    settings.setValue("SelectAllTextOnClick", ui->selectAllOnClick->isChecked());
    settings.setValue("AddCountryDomainWithAltKey", ui->addCountryWithAlt->isChecked());
    settings.setValue("ShowLoadingProgress", ui->showLoadingInAddressBar->isChecked());
    settings.setValue("ProgressStyle", ui->progressStyleSelector->currentIndex());
    settings.setValue("UseCustomProgressColor", ui->checkBoxCustomProgressColor->isChecked());
    settings.setValue("CustomProgressColor", ui->customColorToolButton->property("ProgressColor").value<QColor>());
    settings.endGroup();

    settings.beginGroup("SearchEngines");
    settings.setValue("SearchWithDefaultEngine", ui->searchWithDefaultEngine->isChecked());
    settings.endGroup();

    //Languages
    settings.beginGroup("Language");
    settings.setValue("language", ui->languages->itemData(ui->languages->currentIndex()).toString());
    settings.endGroup();

    //Proxy Configuration
    NetworkProxyFactory::ProxyPreference proxyPreference;
    if (ui->systemProxy->isChecked()) {
        proxyPreference = NetworkProxyFactory::SystemProxy;
    }
    else if (ui->noProxy->isChecked()) {
        proxyPreference = NetworkProxyFactory::NoProxy;
    }
    else if (ui->pacProxy->isChecked()) {
        proxyPreference = NetworkProxyFactory::ProxyAutoConfig;
    }
    else {
        proxyPreference = NetworkProxyFactory::DefinedProxy;
    }

    QNetworkProxy::ProxyType proxyType;
    if (ui->proxyType->currentIndex() == 0) {
        proxyType = QNetworkProxy::HttpProxy;
    }
    else {
        proxyType = QNetworkProxy::Socks5Proxy;
    }

    settings.beginGroup("Web-Proxy");
    settings.setValue("ProxyType", proxyType);
    settings.setValue("UseProxy", proxyPreference);
    settings.setValue("HostName", ui->proxyServer->text());
    settings.setValue("Port", ui->proxyPort->text().toInt());
    settings.setValue("Username", ui->proxyUsername->text());
    settings.setValue("Password", ui->proxyPassword->text());

    settings.setValue("UseDifferentProxyForHttps", ui->useHttpsProxy->isChecked());
    settings.setValue("HttpsHostName", ui->httpsProxyServer->text());
    settings.setValue("HttpsPort", ui->httpsProxyPort->text());
    settings.setValue("HttpsUsername", ui->httpsProxyUsername->text());
    settings.setValue("HttpsPassword", ui->httpsProxyPassword->text());

    settings.setValue("PacUrl", ui->pacUrl->text());
    settings.setValue("ProxyExceptions", ui->proxyExceptions->text().split(QLatin1Char(','), QString::SkipEmptyParts));
    settings.endGroup();

    ProfileManager profileManager;
    profileManager.setStartingProfile(ui->startProfile->currentText());

    m_pluginsList->save();
    m_themesManager->save();
    mApp->cookieJar()->loadSettings();
    mApp->history()->loadSettings();
    mApp->reloadSettings();
    mApp->plugins()->c2f_saveSettings();
    mApp->networkManager()->loadSettings();
    mApp->desktopNotifications()->loadSettings();
    mApp->autoFill()->loadSettings();
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
