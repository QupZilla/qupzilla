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
#include "preferences.h"
#include "ui_preferences.h"
#include "qupzilla.h"
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
#include "tabbedwebview.h"
#include "clearprivatedata.h"
#include "useragentdialog.h"
#include "registerqappassociation.h"
#include "html5permissions/html5permissionsdialog.h"

#include <QSettings>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QColorDialog>

Preferences::Preferences(QupZilla* mainClass, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Preferences)
    , p_QupZilla(mainClass)
    , m_pluginsList(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    if (QIcon::themeName().toLower() == QLatin1String("oxygen")) {
        ui->listWidget->item(0)->setIcon(QIcon::fromTheme("preferences-desktop", QIcon(":/icons/preferences/preferences-desktop.png")));
        ui->listWidget->item(1)->setIcon(QIcon::fromTheme("format-stroke-color", QIcon(":/icons/preferences/application-x-theme.png")));
        ui->listWidget->item(2)->setIcon(QIcon::fromTheme("tab-new-background", QIcon(":/icons/preferences/applications-internet.png")));
        ui->listWidget->item(3)->setIcon(QIcon::fromTheme("preferences-system-network", QIcon(":/icons/preferences/applications-webbrowsers.png")));
        ui->listWidget->item(4)->setIcon(QIcon::fromTheme("preferences-desktop-font", QIcon(":/icons/preferences/applications-fonts.png")));
        ui->listWidget->item(5)->setIcon(QIcon::fromTheme("preferences-desktop-keyboard-shortcuts", QIcon(":/icons/preferences/preferences-desktop-keyboard-shortcuts.png")));
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
    m_homepage = settings.value("homepage", "qupzilla:start").toString();
    m_newTabUrl = settings.value("newTabUrl", "qupzilla:speeddial").toString();
    ui->homepage->setText(m_homepage);
    ui->newTabUrl->setText(m_newTabUrl);
    int afterLaunch = settings.value("afterLaunch", 1).toInt();
    settings.endGroup();
    ui->afterLaunch->setCurrentIndex(afterLaunch);
    ui->checkUpdates->setChecked(settings.value("Web-Browser-Settings/CheckUpdates", DEFAULT_CHECK_UPDATES).toBool());
    ui->dontLoadTabsUntilSelected->setChecked(settings.value("Web-Browser-Settings/LoadTabsOnActivation", false).toBool());
#ifdef Q_OS_WIN
    ui->checkDefaultBrowser->setChecked(settings.value("Web-Browser-Settings/CheckDefaultBrowser", DEFAULT_CHECK_DEFAULTBROWSER).toBool());
    if (mApp->associationManager()->isDefaultForAllCapabilities()) {
        ui->checkNowDefaultBrowser->setText(tr("QupZilla is default"));
        ui->checkNowDefaultBrowser->setEnabled(false);
    }
    else {
        ui->checkNowDefaultBrowser->setText(tr("Make QupZilla default"));
        ui->checkNowDefaultBrowser->setEnabled(true);
        connect(ui->checkNowDefaultBrowser, SIGNAL(clicked()), this, SLOT(makeQupZillaDefault()));
    }
#else // just Windows
    ui->hSpacerDefaultBrowser->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->hLayoutDefaultBrowser->invalidate();
    delete ui->hLayoutDefaultBrowser;
    delete ui->checkDefaultBrowser;
    delete ui->checkNowDefaultBrowser;
#endif
    ui->newTabFrame->setVisible(false);
    if (m_newTabUrl.isEmpty()) {
        ui->newTab->setCurrentIndex(0);
    }
    else if (m_newTabUrl == m_homepage) {
        ui->newTab->setCurrentIndex(1);
    }
    else if (m_newTabUrl == QLatin1String("qupzilla:speeddial")) {
        ui->newTab->setCurrentIndex(2);
    }
    else {
        ui->newTab->setCurrentIndex(3);
        ui->newTabFrame->setVisible(true);
    }

    afterLaunchChanged(ui->afterLaunch->currentIndex());
    connect(ui->afterLaunch, SIGNAL(currentIndexChanged(int)), this, SLOT(afterLaunchChanged(int)));
    connect(ui->newTab, SIGNAL(currentIndexChanged(int)), this, SLOT(newTabChanged(int)));
    if (p_QupZilla) {
        connect(ui->useCurrentBut, SIGNAL(clicked()), this, SLOT(useActualHomepage()));
        connect(ui->newTabUseCurrent, SIGNAL(clicked()), this, SLOT(useActualNewTab()));
    }
    else {
        ui->useCurrentBut->setEnabled(false);
        ui->newTabUseCurrent->setEnabled(false);
    }
    //PROFILES
    m_actProfileName = mApp->currentProfilePath();
    m_actProfileName = m_actProfileName.left(m_actProfileName.length() - 1);
    m_actProfileName = m_actProfileName.mid(m_actProfileName.lastIndexOf(QLatin1Char('/')));
    m_actProfileName.remove(QLatin1Char('/'));

    ui->activeProfile->setText("<b>" + m_actProfileName + "</b>");

    QSettings profileSettings(mApp->PROFILEDIR + "profiles/profiles.ini", QSettings::IniFormat);
    QString actProfileName = profileSettings.value("Profiles/startProfile", "default").toString();

    ui->startProfile->addItem(actProfileName);
    QDir profilesDir(mApp->PROFILEDIR + "profiles/");
    QStringList list_ = profilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(const QString & name, list_) {
        if (actProfileName == name) {
            continue;
        }

        ui->startProfile->addItem(name);
    }
    connect(ui->createProfile, SIGNAL(clicked()), this, SLOT(createProfile()));
    connect(ui->deleteProfile, SIGNAL(clicked()), this, SLOT(deleteProfile()));
    connect(ui->startProfile, SIGNAL(currentIndexChanged(QString)), this, SLOT(startProfileIndexChanged(QString)));
    startProfileIndexChanged(ui->startProfile->currentText());

    //APPEREANCE
    m_themesManager = new ThemeManager(ui->themesWidget, this);
    settings.beginGroup("Browser-View-Settings");
    ui->showStatusbar->setChecked(settings.value("showStatusBar", true).toBool());
    if (p_QupZilla) {
        ui->showBookmarksToolbar->setChecked(p_QupZilla->bookmarksToolbar()->isVisible());
        ui->showNavigationToolbar->setChecked(p_QupZilla->navigationBar()->isVisible());
    }
    else {
        ui->showBookmarksToolbar->setChecked(settings.value("showBookmarksToolbar", true).toBool());
        ui->showNavigationToolbar->setChecked(settings.value("showNavigationToolbar", true).toBool());
    }
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
    ui->switchToNewTabs->setChecked(settings.value("OpenNewTabsSelected", false).toBool());
    ui->dontQuitOnTab->setChecked(settings.value("dontQuitWithOneTab", false).toBool());
    ui->askWhenClosingMultipleTabs->setChecked(settings.value("AskOnClosing", false).toBool());
    ui->closedInsteadOpened->setChecked(settings.value("closedInsteadOpenedTabs", false).toBool());
    ui->showTabPreviews->setChecked(settings.value("showTabPreviews", true).toBool());
    ui->animatedTabPreviews->setChecked(settings.value("tabPreviewAnimationsEnabled", true).toBool());
    settings.endGroup();

    connect(ui->showTabPreviews, SIGNAL(toggled(bool)), this, SLOT(showTabPreviewsChanged(bool)));
    showTabPreviewsChanged(ui->showTabPreviews->isChecked());

    //AddressBar
    settings.beginGroup("AddressBar");
    ui->addressbarCompletion->setCurrentIndex(settings.value("showSuggestions", 0).toInt());
    ui->completionShowSwitchTab->setChecked(settings.value("showSwitchTab", true).toBool());
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
    QColor pbColor = settings.value("CustomProgressColor", p_QupZilla->palette().color(QPalette::Highlight)).value<QColor>();
    setProgressBarColorIcon(pbColor);
    connect(ui->customColorToolButton, SIGNAL(clicked(bool)), SLOT(selectCustomProgressBarColor()));
    connect(ui->resetProgressBarcolor, SIGNAL(clicked()), SLOT(setProgressBarColorIcon()));
    settings.endGroup();

    settings.beginGroup("SearchEngines");
    ui->searchWithDefaultEngine->setChecked(settings.value("SearchWithDefaultEngine", false).toBool());
    settings.endGroup();

    //BROWSING
    settings.beginGroup("Web-Browser-Settings");
    ui->allowPlugins->setChecked(settings.value("allowFlash", true).toBool());
    ui->allowJavaScript->setChecked(settings.value("allowJavaScript", true).toBool());
    ui->allowJava->setChecked(settings.value("allowJava", true).toBool());
    ui->allowDNSPrefetch->setChecked(settings.value("DNS-Prefetch", false).toBool());
    ui->linksInFocusChain->setChecked(settings.value("IncludeLinkInFocusChain", false).toBool());
    ui->zoomTextOnly->setChecked(settings.value("zoomTextOnly", false).toBool());
    ui->caretBrowsing->setChecked(settings.value("CaretBrowsing", false).toBool());
    ui->animateScrolling->setChecked(settings.value("AnimateScrolling", true).toBool());
    ui->printEBackground->setChecked(settings.value("PrintElementBackground", true).toBool());
    ui->wheelScroll->setValue(settings.value("wheelScrollLines", qApp->wheelScrollLines()).toInt());
    ui->defaultZoom->setValue(settings.value("DefaultZoom", 100).toInt());
    ui->xssAuditing->setChecked(settings.value("XSSAuditing", false).toBool());

    //Cache
    ui->pagesInCache->setValue(settings.value("maximumCachedPages", 3).toInt());
    connect(ui->pagesInCache, SIGNAL(valueChanged(int)), this, SLOT(pageCacheValueChanged(int)));
    ui->pageCacheLabel->setText(QString::number(ui->pagesInCache->value()));

    ui->allowCache->setChecked(settings.value("AllowLocalCache", true).toBool());
    ui->cacheMB->setValue(settings.value("LocalCacheSize", 50).toInt());
    ui->MBlabel->setText(settings.value("LocalCacheSize", 50).toString() + " MB");
    ui->cachePath->setText(settings.value("CachePath", QString("%1networkcache/").arg(mApp->currentProfilePath())).toString());
    connect(ui->allowCache, SIGNAL(clicked(bool)), this, SLOT(allowCacheChanged(bool)));
    connect(ui->cacheMB, SIGNAL(valueChanged(int)), this, SLOT(cacheValueChanged(int)));
    connect(ui->changeCachePath, SIGNAL(clicked()), this, SLOT(changeCachePathClicked()));
    allowCacheChanged(ui->allowCache->isChecked());

    //PASSWORD MANAGER
    ui->allowPassManager->setChecked(settings.value("SavePasswordsOnSites", true).toBool());
    connect(ui->allowPassManager, SIGNAL(toggled(bool)), this, SLOT(showPassManager(bool)));

    m_autoFillManager = new AutoFillManager(this);
    ui->autoFillFrame->addWidget(m_autoFillManager);
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
    ui->fontStandard->setCurrentFont(QFont(settings.value("StandardFont", mApp->webSettings()->fontFamily(QWebSettings::StandardFont)).toString()));
    ui->fontCursive->setCurrentFont(QFont(settings.value("CursiveFont", mApp->webSettings()->fontFamily(QWebSettings::CursiveFont)).toString()));
    ui->fontFantasy->setCurrentFont(QFont(settings.value("FantasyFont", mApp->webSettings()->fontFamily(QWebSettings::FantasyFont)).toString()));
    ui->fontFixed->setCurrentFont(QFont(settings.value("FixedFont", mApp->webSettings()->fontFamily(QWebSettings::FixedFont)).toString()));
    ui->fontSansSerif->setCurrentFont(QFont(settings.value("SansSerifFont", mApp->webSettings()->fontFamily(QWebSettings::SansSerifFont)).toString()));
    ui->fontSerif->setCurrentFont(QFont(settings.value("SerifFont", mApp->webSettings()->fontFamily(QWebSettings::SerifFont)).toString()));

    ui->sizeDefault->setValue(settings.value("DefaultFontSize", mApp->webSettings()->fontSize(QWebSettings::DefaultFontSize)).toInt());
    ui->sizeFixed->setValue(settings.value("FixedFontSize", mApp->webSettings()->fontSize(QWebSettings::DefaultFixedFontSize)).toInt());
    ui->sizeMinimum->setValue(settings.value("MinimumFontSize", mApp->webSettings()->fontSize(QWebSettings::MinimumFontSize)).toInt());
    ui->sizeMinimumLogical->setValue(settings.value("MinimumLogicalFontSize", mApp->webSettings()->fontSize(QWebSettings::MinimumLogicalFontSize)).toInt());
    settings.endGroup();

    //KEYBOARD SHORTCUTS
    settings.beginGroup("Shortcuts");
    ui->switchTabsAlt->setChecked(settings.value("useTabNumberShortcuts", true).toBool());
    ui->loadSpeedDialsCtrl->setChecked(settings.value("useSpeedDialNumberShortcuts", true).toBool());
    settings.endGroup();

    //PLUGINS
    m_pluginsList = new PluginsManager(this);
    ui->pluginsFrame->addWidget(m_pluginsList);

    //NOTIFICATIONS
    ui->useNativeSystemNotifications->setEnabled(mApp->desktopNotifications()->supportsNativeNotifications());

    DesktopNotificationsFactory::Type notifyType;
    settings.beginGroup("Notifications");
    ui->notificationTimeout->setValue(settings.value("Timeout", 6000).toInt() / 1000);
#ifdef QZ_WS_X11
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
    QString activeLanguage;
    if (!mApp->currentLanguageFile().isEmpty()) {
        activeLanguage = mApp->currentLanguageFile();
        QLocale locale(activeLanguage);
        QString country = QLocale::countryToString(locale.country());
        QString language = QLocale::languageToString(locale.language());
        ui->languages->addItem(language + ", " + country + " (" + activeLanguage + ")", activeLanguage);
    }
    ui->languages->addItem("English (en_US)");

    QDir lanDir(mApp->TRANSLATIONSDIR);
    QStringList list = lanDir.entryList(QStringList("*.qm"));
    foreach(const QString & name, list) {
        if (name.startsWith(QLatin1String("qt_"))) {
            continue;
        }

        QString loc = name;
        loc.remove(QLatin1String(".qm"));

        if (loc == activeLanguage) {
            continue;
        }

        QLocale locale(loc);
        QString country = QLocale::countryToString(locale.country());
        QString language = QLocale::languageToString(locale.language());
        ui->languages->addItem(language + ", " + country + " (" + loc + ")", loc);
    }

    // Proxy Configuration
    settings.beginGroup("Web-Proxy");
    NetworkProxyFactory::ProxyPreference proxyPreference = NetworkProxyFactory::ProxyPreference(settings.value("UseProxy", NetworkProxyFactory::SystemProxy).toInt());
    QNetworkProxy::ProxyType proxyType = QNetworkProxy::ProxyType(settings.value("ProxyType", QNetworkProxy::HttpProxy).toInt());

    ui->systemProxy->setChecked(proxyPreference == NetworkProxyFactory::SystemProxy);
    ui->noProxy->setChecked(proxyPreference == NetworkProxyFactory::NoProxy);
    ui->manualProxy->setChecked(proxyPreference == NetworkProxyFactory::DefinedProxy);
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

    ui->proxyExceptions->setText(settings.value("ProxyExceptions", QStringList() << "localhost" << "127.0.0.1").toStringList().join(","));
    settings.endGroup();

    useDifferentProxyForHttpsChanged(ui->useHttpsProxy->isChecked());
    setManualProxyConfigurationEnabled(proxyPreference == NetworkProxyFactory::DefinedProxy);

    connect(ui->manualProxy, SIGNAL(toggled(bool)), this, SLOT(setManualProxyConfigurationEnabled(bool)));
    connect(ui->useHttpsProxy, SIGNAL(toggled(bool)), this, SLOT(useDifferentProxyForHttpsChanged(bool)));

    //CONNECTS
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
    connect(ui->cookieManagerBut, SIGNAL(clicked()), this, SLOT(showCookieManager()));
    connect(ui->html5permissions, SIGNAL(clicked()), this, SLOT(showHtml5Permissions()));
    connect(ui->sslManagerButton, SIGNAL(clicked()), this, SLOT(openSslManager()));
    connect(ui->preferredLanguages, SIGNAL(clicked()), this, SLOT(showAcceptLanguage()));
    connect(ui->deleteHtml5storage, SIGNAL(clicked()), this, SLOT(deleteHtml5storage()));
    connect(ui->uaManager, SIGNAL(clicked()), this, SLOT(openUserAgentManager()));
    connect(ui->jsOptionsButton, SIGNAL(clicked()), this, SLOT(openJsOptions()));

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(showStackedPage(QListWidgetItem*)));
    ui->listWidget->setItemSelected(ui->listWidget->itemAt(5, 5), true);

    ui->version->setText(" QupZilla v" + QupZilla::VERSION);
    ui->listWidget->setCurrentRow(currentSettingsPage);

#if QTWEBKIT_TO_2_3
    ui->caretBrowsing->setHidden(true);
    ui->animateScrolling->setHidden(true);
#endif

#if QTWEBKIT_TO_2_2
    ui->html5permissions->setDisabled(true);
#endif
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
#ifdef Q_OS_WIN
    disconnect(ui->checkNowDefaultBrowser, SIGNAL(clicked()), this, SLOT(makeQupZillaDefault()));
    mApp->associationManager()->registerAllAssociation();
    ui->checkNowDefaultBrowser->setText(tr("QupZilla is default"));
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
    ui->homepage->setText(p_QupZilla->weView()->url().toString());
}

void Preferences::useActualNewTab()
{
    ui->newTabUrl->setText(p_QupZilla->weView()->url().toString());
}

void Preferences::chooseDownPath()
{
    QString userFileName = QFileDialog::getExistingDirectory(this, tr("Choose download location..."), QDir::homePath());
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
    QString file = QFileDialog::getOpenFileName(this, tr("Choose stylesheet location..."), QDir::homePath(), "*.css");
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
    QString path = QFileDialog::getOpenFileName(this, tr("Choose executable location..."), QDir::homePath());
    if (path.isEmpty()) {
        return;
    }

    ui->externalDownExecutable->setText(path);
}

void Preferences::openUserAgentManager()
{
    UserAgentDialog dialog(this);
    dialog.exec();
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
    CookieManager* m = mApp->cookieManager();
    m->refreshTable();

    m->show();
    m->raise();
}

void Preferences::showHtml5Permissions()
{
    HTML5PermissionsDialog dialog(this);
    dialog.exec();
}

void Preferences::openSslManager()
{
    SSLManager* m = new SSLManager(this);
    m->show();
}

void Preferences::openJsOptions()
{
    JsOptions options(this);
    options.exec();
}

void Preferences::showAcceptLanguage()
{
    AcceptLanguage a(this);
    a.exec();
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
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose cache path..."), ui->cachePath->text());
    if (path.isEmpty()) {
        return;
    }

    ui->cachePath->setText(path);
}

void Preferences::showPassManager(bool state)
{
    m_autoFillManager->setVisible(state);
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
    QDir dir(mApp->PROFILEDIR + "profiles/");
    if (QDir(dir.absolutePath() + "/" + name).exists()) {
        QMessageBox::warning(this, tr("Error!"), tr("This profile already exists!"));
        return;
    }
    if (!dir.mkdir(name)) {
        QMessageBox::warning(this, tr("Error!"), tr("Cannot create profile directory!"));
        return;
    }

    dir.cd(name);
    QFile(":data/browsedata.db").copy(dir.absolutePath() + "/browsedata.db");
    QFile(dir.absolutePath() + "/browsedata.db").setPermissions(QFile::ReadUser | QFile::WriteUser);

    QFile versionFile(dir.absolutePath() + "/version");
    versionFile.open(QFile::WriteOnly);
    versionFile.write(QupZilla::VERSION.toUtf8());
    versionFile.close();

    ui->startProfile->insertItem(0, name);
    ui->startProfile->setCurrentIndex(0);
}

void Preferences::deleteProfile()
{
    QString name = ui->startProfile->currentText();
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                                         tr("Are you sure to permanently delete \"%1\" profile? This action cannot be undone!").arg(name), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) {
        return;
    }

    QzTools::removeDir(mApp->PROFILEDIR + "profiles/" + name);
    ui->startProfile->removeItem(ui->startProfile->currentIndex());
}

void Preferences::startProfileIndexChanged(QString index)
{
    ui->deleteProfile->setEnabled(m_actProfileName != index);

    if (m_actProfileName == index) {
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
    settings.beginGroup("Web-URL-Settings");
    settings.setValue("homepage", ui->homepage->text());

    QString homepage = ui->homepage->text();
    settings.setValue("afterLaunch", ui->afterLaunch->currentIndex());

    switch (ui->newTab->currentIndex()) {
    case 0:
        settings.setValue("newTabUrl", "");
        break;

    case 1:
        settings.setValue("newTabUrl", homepage);
        break;

    case 2:
        settings.setValue("newTabUrl", "qupzilla:speeddial");
        break;

    case 3:
        settings.setValue("newTabUrl", ui->newTabUrl->text());
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
    settings.setValue("showStatusbar", ui->showStatusbar->isChecked());
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
    settings.setValue("OpenNewTabsSelected", ui->switchToNewTabs->isChecked());
    settings.setValue("dontQuitWithOneTab", ui->dontQuitOnTab->isChecked());
    settings.setValue("AskOnClosing", ui->askWhenClosingMultipleTabs->isChecked());
    settings.setValue("closedInsteadOpenedTabs", ui->closedInsteadOpened->isChecked());
    settings.setValue("showTabPreviews", ui->showTabPreviews->isChecked());
    settings.setValue("tabPreviewAnimationsEnabled", ui->animatedTabPreviews->isChecked());
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
    settings.endGroup();

    //BROWSING
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("allowFlash", ui->allowPlugins->isChecked());
    settings.setValue("allowJavaScript", ui->allowJavaScript->isChecked());
    settings.setValue("allowJava", ui->allowJava->isChecked());
    settings.setValue("DNS-Prefetch", ui->allowDNSPrefetch->isChecked());
    settings.setValue("IncludeLinkInFocusChain", ui->linksInFocusChain->isChecked());
    settings.setValue("zoomTextOnly", ui->zoomTextOnly->isChecked());
    settings.setValue("CaretBrowsing", ui->caretBrowsing->isChecked());
    settings.setValue("AnimateScrolling", ui->animateScrolling->isChecked());
    settings.setValue("PrintElementBackground", ui->printEBackground->isChecked());
    settings.setValue("wheelScrollLines", ui->wheelScroll->value());
    settings.setValue("DoNotTrack", ui->doNotTrack->isChecked());
    settings.setValue("CheckUpdates", ui->checkUpdates->isChecked());
    settings.setValue("LoadTabsOnActivation", ui->dontLoadTabsUntilSelected->isChecked());
    settings.setValue("DefaultZoom", ui->defaultZoom->value());
    settings.setValue("XSSAuditing", ui->xssAuditing->isChecked());
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

    settings.setValue("ProxyExceptions", ui->proxyExceptions->text().split(QLatin1Char(',')));
    settings.endGroup();

    //Profiles
    QSettings profileSettings(mApp->PROFILEDIR + "profiles/profiles.ini", QSettings::IniFormat);
    profileSettings.setValue("Profiles/startProfile", ui->startProfile->currentText());

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
        color = p_QupZilla->palette().color(QPalette::Highlight);
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
