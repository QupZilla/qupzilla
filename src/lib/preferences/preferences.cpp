/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "pluginslist.h"
#include "qtwin.h"
#include "sslmanager.h"
#include "networkproxyfactory.h"
#include "networkmanager.h"
#include "desktopnotificationsfactory.h"
#include "desktopnotification.h"
#include "navigationbar.h"
#include "thememanager.h"
#include "acceptlanguage.h"
#include "globalfunctions.h"
#include "autofillmodel.h"
#include "settings.h"
#include "tabbedwebview.h"
#include "clearprivatedata.h"

#include <QSettings>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

Preferences::Preferences(QupZilla* mainClass, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Preferences)
    , p_QupZilla(mainClass)
    , m_pluginsList(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

#ifdef KDE
    ui->listWidget->item(0)->setIcon(QIcon::fromTheme("preferences-desktop", QIcon(":/icons/preferences/preferences-desktop.png")));
    ui->listWidget->item(1)->setIcon(QIcon::fromTheme("format-stroke-color", QIcon(":/icons/preferences/application-x-theme.png")));
    ui->listWidget->item(2)->setIcon(QIcon::fromTheme("tab-new-background", QIcon(":/icons/preferences/applications-internet.png")));
    ui->listWidget->item(3)->setIcon(QIcon::fromTheme("preferences-system-network", QIcon(":/icons/preferences/applications-webbrowsers.png")));
    ui->listWidget->item(4)->setIcon(QIcon::fromTheme("preferences-desktop-font", QIcon(":/icons/preferences/applications-fonts.png")));
    ui->listWidget->item(5)->setIcon(QIcon::fromTheme("download", QIcon(":/icons/preferences/mail-inbox.png")));
    ui->listWidget->item(6)->setIcon(QIcon::fromTheme("user-identity", QIcon(":/icons/preferences/dialog-password.png")));
    ui->listWidget->item(7)->setIcon(QIcon::fromTheme("preferences-system-firewall", QIcon(":/icons/preferences/preferences-system-firewall.png")));
    ui->listWidget->item(8)->setIcon(QIcon::fromTheme("preferences-desktop-notification", QIcon(":/icons/preferences/dialog-question.png")));
    ui->listWidget->item(9)->setIcon(QIcon::fromTheme("preferences-plugin", QIcon(":/icons/preferences/extension.png")));
    ui->listWidget->item(10)->setIcon(QIcon::fromTheme("applications-system", QIcon(":/icons/preferences/applications-system.png")));
#else
    ui->listWidget->item(0)->setIcon(QIcon::fromTheme("preferences-desktop", QIcon(":/icons/preferences/preferences-desktop.png")));
    ui->listWidget->item(1)->setIcon(QIcon::fromTheme("application-x-theme", QIcon(":/icons/preferences/application-x-theme.png")));
    ui->listWidget->item(2)->setIcon(QIcon::fromTheme("applications-internet", QIcon(":/icons/preferences/applications-internet.png")));
    ui->listWidget->item(3)->setIcon(QIcon::fromTheme("applications-webbrowsers", QIcon(":/icons/preferences/applications-webbrowsers.png")));
    ui->listWidget->item(4)->setIcon(QIcon::fromTheme("applications-fonts", QIcon(":/icons/preferences/applications-fonts.png")));
    ui->listWidget->item(5)->setIcon(QIcon::fromTheme("mail-inbox", QIcon(":/icons/preferences/mail-inbox.png")));
    ui->listWidget->item(6)->setIcon(QIcon::fromTheme("dialog-password", QIcon(":/icons/preferences/dialog-password.png")));
    ui->listWidget->item(7)->setIcon(QIcon::fromTheme("preferences-system-firewall", QIcon(":/icons/preferences/preferences-system-firewall.png")));
    ui->listWidget->item(8)->setIcon(QIcon::fromTheme("dialog-question", QIcon(":/icons/preferences/dialog-question.png")));
    ui->listWidget->item(9)->setIcon(QIcon::fromTheme("extension", QIcon(":/icons/preferences/extension.png")));
    ui->listWidget->item(10)->setIcon(QIcon::fromTheme("applications-system", QIcon(":/icons/preferences/applications-system.png")));
#endif

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

    ui->newTabFrame->setVisible(false);
    if (m_newTabUrl.isEmpty()) {
        ui->newTab->setCurrentIndex(0);
    }
    else if (m_newTabUrl == m_homepage) {
        ui->newTab->setCurrentIndex(1);
    }
    else if (m_newTabUrl == "qupzilla:speeddial") {
        ui->newTab->setCurrentIndex(2);
    }
    else {
        ui->newTab->setCurrentIndex(3);
        ui->newTabFrame->setVisible(true);
    }

    afterLaunchChanged(ui->afterLaunch->currentIndex());
    connect(ui->afterLaunch, SIGNAL(currentIndexChanged(int)), this, SLOT(afterLaunchChanged(int)));
    connect(ui->newTab, SIGNAL(currentIndexChanged(int)), this, SLOT(newTabChanged(int)));
    connect(ui->useCurrentBut, SIGNAL(clicked()), this, SLOT(useActualHomepage()));
    connect(ui->newTabUseCurrent, SIGNAL(clicked()), this, SLOT(useActualNewTab()));

    //PROFILES
    m_actProfileName = mApp->currentProfilePath();
    m_actProfileName = m_actProfileName.left(m_actProfileName.length() - 1);
    m_actProfileName = m_actProfileName.mid(m_actProfileName.lastIndexOf("/"));
    m_actProfileName.remove("/");

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
    ui->showBookmarksToolbar->setChecked(p_QupZilla->bookmarksToolbar()->isVisible());
    ui->showNavigationToolbar->setChecked(p_QupZilla->navigationBar()->isVisible());
    ui->showHome->setChecked(settings.value("showHomeButton", true).toBool());
    ui->showBackForward->setChecked(settings.value("showBackForwardButtons", true).toBool());
    ui->showAddTabButton->setChecked(settings.value("showAddTabButton", false).toBool());
    ui->useTransparentBg->setChecked(settings.value("useTransparentBackground", false).toBool());
    settings.endGroup();
#ifdef Q_WS_WIN
    ui->useTransparentBg->setEnabled(QtWin::isCompositionEnabled());
#endif

    //TABS
    settings.beginGroup("Browser-Tabs-Settings");
    ui->hideTabsOnTab->setChecked(settings.value("hideTabsWithOneTab", false).toBool());
    ui->activateLastTab->setChecked(settings.value("ActivateLastTabWhenClosingActual", false).toBool());
    ui->openNewTabAfterActive->setChecked(settings.value("newTabAfterActive", true).toBool());
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
    ui->selectAllOnFocus->setChecked(settings.value("SelectAllTextOnDoubleClick", true).toBool());
    ui->selectAllOnClick->setChecked(settings.value("SelectAllTextOnClick", false).toBool());
    ui->addCountryWithAlt->setChecked(settings.value("AddCountryDomainWithAltKey", true).toBool());
    settings.endGroup();

    //BROWSING
    settings.beginGroup("Web-Browser-Settings");
    ui->allowPlugins->setChecked(settings.value("allowFlash", true).toBool());
    ui->allowJavaScript->setChecked(settings.value("allowJavaScript", true).toBool());
    ui->allowJava->setChecked(settings.value("allowJava", true).toBool());
    ui->allowDNSPrefetch->setChecked(settings.value("DNS-Prefetch", false).toBool());
    ui->linksInFocusChain->setChecked(settings.value("IncludeLinkInFocusChain", false).toBool());
    ui->zoomTextOnly->setChecked(settings.value("zoomTextOnly", false).toBool());
    ui->printEBackground->setChecked(settings.value("PrintElementBackground", true).toBool());
    ui->wheelScroll->setValue(settings.value("wheelScrollLines", qApp->wheelScrollLines()).toInt());
    ui->defaultZoom->setValue(settings.value("DefaultZoom", 100).toInt());
    ui->xssAuditing->setChecked(settings.value("XSSAuditing", false).toBool());

    if (!ui->allowJavaScript->isChecked()) {
        ui->blockPopup->setEnabled(false);
    }
    connect(ui->allowJavaScript, SIGNAL(toggled(bool)), this, SLOT(allowJavaScriptChanged(bool)));
    //Cache
    ui->pagesInCache->setValue(settings.value("maximumCachedPages", 3).toInt());
    connect(ui->pagesInCache, SIGNAL(valueChanged(int)), this, SLOT(pageCacheValueChanged(int)));
    ui->pageCacheLabel->setText(QString::number(ui->pagesInCache->value()));

    ui->allowCache->setChecked(settings.value("AllowLocalCache", true).toBool());
    ui->cacheMB->setValue(settings.value("LocalCacheSize", 50).toInt());
    ui->MBlabel->setText(settings.value("LocalCacheSize", 50).toString() + " MB");
    connect(ui->allowCache, SIGNAL(clicked(bool)), this, SLOT(allowCacheChanged(bool)));
    connect(ui->cacheMB, SIGNAL(valueChanged(int)), this, SLOT(cacheValueChanged(int)));
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
    ui->blockPopup->setChecked(!settings.value("allowJavaScriptOpenWindow", false).toBool());
    ui->jscanAccessClipboard->setChecked(settings.value("JavaScriptCanAccessClipboard", true).toBool());
    ui->doNotTrack->setChecked(settings.value("DoNotTrack", false).toBool());
    ui->sendReferer->setChecked(settings.value("SendReferer", true).toBool());

    // UserAgent
    const QString &os = qz_buildSystem();
    QStringList items;
    items << QString("Opera/9.80 (%1) Presto/2.10.229 Version/11.61").arg(os)
          << QString("Mozilla/5.0 (%1) AppleWebKit/535.7 (KHTML, like Gecko) Chrome/16.0.912.77 Safari/535.7").arg(os)
          << QString("Mozilla/5.0 (%1) AppleWebKit/533.21.1 (KHTML, like Gecko) Version/5.0.5 Safari/533.21.1").arg(os)
          << QString("Mozilla/5.0 (%1; rv:9.0.1) Gecko/20100101 Firefox/9.0.1").arg(os);

    const QString &savedUserAgent = settings.value("UserAgent", "").toString();
    ui->changeUserAgent->setChecked(!savedUserAgent.isEmpty());
    ui->userAgentCombo->addItems(items);

    int index = ui->userAgentCombo->findText(savedUserAgent);
    if (index >= 0) {
        ui->userAgentCombo->setCurrentIndex(index);
    }
    else {
        ui->userAgentCombo->lineEdit()->setText(savedUserAgent);
    }

    ui->userAgentCombo->lineEdit()->setCursorPosition(0);
    connect(ui->changeUserAgent, SIGNAL(toggled(bool)), this, SLOT(changeUserAgentChanged(bool)));
    changeUserAgentChanged(ui->changeUserAgent->isChecked());

    //CSS Style
    ui->userStyleSheet->setText(settings.value("userStyleSheet", "").toString());
    connect(ui->chooseUserStylesheet, SIGNAL(clicked()), this, SLOT(chooseUserStyleClicked()));
    settings.endGroup();

    //Cookies
    settings.beginGroup("Cookie-Settings");
    ui->saveCookies->setChecked(settings.value("allowCookies", true).toBool());
    if (!ui->saveCookies->isChecked()) {
        ui->deleteCookiesOnClose->setEnabled(false);
    }
    connect(ui->saveCookies, SIGNAL(toggled(bool)), this, SLOT(saveCookiesChanged(bool)));
    ui->deleteCookiesOnClose->setChecked(settings.value("deleteCookiesOnClose", false).toBool());
    ui->matchExactly->setChecked(settings.value("allowCookiesFromVisitedDomainOnly", false).toBool());
    ui->filterTracking->setChecked(settings.value("filterTrackingCookie", false).toBool());
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

    //PLUGINS
    m_pluginsList = new PluginsList(this);
    ui->pluginsFrame->addWidget(m_pluginsList);

    //NOTIFICATIONS
#ifdef Q_WS_X11
    ui->useNativeSystemNotifications->setEnabled(true);
#endif
    DesktopNotificationsFactory::Type notifyType;
    settings.beginGroup("Notifications");
    ui->notificationTimeout->setValue(settings.value("Timeout", 6000).toInt() / 1000);
#ifdef Q_WS_X11
    notifyType = settings.value("UseNativeDesktop", true).toBool() ? DesktopNotificationsFactory::DesktopNative : DesktopNotificationsFactory::PopupWidget;
#else
    notifyType = DesktopNotificationsFactory::PopupWidget;
#endif
    if (notifyType == DesktopNotificationsFactory::DesktopNative) {
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
    QString activeLanguage = "";
    if (!mApp->currentLanguage().isEmpty()) {
        activeLanguage = mApp->currentLanguage();
        QLocale locale(activeLanguage);
        QString country = QLocale::countryToString(locale.country());
        QString language = QLocale::languageToString(locale.language());
        ui->languages->addItem(language + ", " + country + " (" + activeLanguage + ")", activeLanguage);
    }
    ui->languages->addItem("English (en_US)");

    QDir lanDir(mApp->TRANSLATIONSDIR);
    QStringList list = lanDir.entryList(QStringList("*.qm"));
    foreach(const QString & name, list) {
        if (name.startsWith("qt_") || name == activeLanguage) {
            continue;
        }

        QString loc = name;
        loc.remove(".qm");
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
    connect(ui->sslManagerButton, SIGNAL(clicked()), this, SLOT(openSslManager()));
    connect(ui->preferredLanguages, SIGNAL(clicked()), this, SLOT(showAcceptLanguage()));
    connect(ui->deleteHtml5storage, SIGNAL(clicked()), this, SLOT(deleteHtml5storage()));

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(showStackedPage(QListWidgetItem*)));
    ui->listWidget->setItemSelected(ui->listWidget->itemAt(5, 5), true);

    ui->version->setText(" QupZilla v" + QupZilla::VERSION);
    showStackedPage(ui->listWidget->item(0));
}

void Preferences::showStackedPage(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    ui->caption->setText("<b>" + item->text() + "</b>");
    ui->stackedWidget->setCurrentIndex(ui->listWidget->currentRow());

    setNotificationPreviewVisible(ui->stackedWidget->currentIndex() == 8);
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

void Preferences::allowCacheChanged(bool state)
{
    ui->cacheFrame->setEnabled(state);
    ui->cacheMB->setEnabled(state);
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
    QString userFileName = QFileDialog::getExistingDirectory(p_QupZilla, tr("Choose download location..."), QDir::homePath());
    if (userFileName.isEmpty()) {
        return;
    }
#ifdef Q_WS_WIN   //QFileDialog::getExistingDirectory returns path with \ instead of / (??)
    userFileName.replace("\\", "/");
#endif
    userFileName += "/";

    ui->downLoc->setText(userFileName);
}

void Preferences::chooseUserStyleClicked()
{
    QString file = QFileDialog::getOpenFileName(p_QupZilla, tr("Choose stylesheet location..."), QDir::homePath(), "*.css");
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
    QString path = QFileDialog::getOpenFileName(p_QupZilla, tr("Choose executable location..."), QDir::homePath());
    if (path.isEmpty()) {
        return;
    }

    ui->externalDownExecutable->setText(path);
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

void Preferences::allowJavaScriptChanged(bool state)
{
    ui->blockPopup->setEnabled(state);
}

void Preferences::saveHistoryChanged(bool stat)
{
    ui->deleteHistoryOnClose->setEnabled(stat);
}

void Preferences::saveCookiesChanged(bool state)
{
    ui->deleteCookiesOnClose->setEnabled(state);
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

void Preferences::openSslManager()
{
    SSLManager* m = new SSLManager(this);
    m->show();
}

void Preferences::showAcceptLanguage()
{
    AcceptLanguage* a = new AcceptLanguage(this);
    a->exec();

    delete a;
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
    if (value == 0) {
        ui->allowCache->setChecked(false);
        allowCacheChanged(false);
    }
    else if (!ui->allowCache->isChecked()) {
        ui->allowCache->setChecked(true);
        allowCacheChanged(true);
    }
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

void Preferences::changeUserAgentChanged(bool state)
{
    ui->userAgentCombo->setEnabled(state);
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
    name = qz_filterCharsFromFilename(name);
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

    qz_removeDir(mApp->PROFILEDIR + "profiles/" + name);
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
    settings.setValue("useTransparentBackground", ui->useTransparentBg->isChecked());
    settings.setValue("showAddTabButton", ui->showAddTabButton->isChecked());
    settings.endGroup();

    //TABS
    settings.beginGroup("Browser-Tabs-Settings");
    settings.setValue("hideTabsWithOneTab", ui->hideTabsOnTab->isChecked());
    settings.setValue("ActivateLastTabWhenClosingActual", ui->activateLastTab->isChecked());
    settings.setValue("newTabAfterActive", ui->openNewTabAfterActive->isChecked());
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

    //BROWSING
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("allowFlash", ui->allowPlugins->isChecked());
    settings.setValue("allowJavaScript", ui->allowJavaScript->isChecked());
    settings.setValue("allowJavaScriptOpenWindow", !ui->blockPopup->isChecked());
    settings.setValue("allowJava", ui->allowJava->isChecked());
    settings.setValue("DNS-Prefetch", ui->allowDNSPrefetch->isChecked());
    settings.setValue("JavaScriptCanAccessClipboard", ui->jscanAccessClipboard->isChecked());
    settings.setValue("IncludeLinkInFocusChain", ui->linksInFocusChain->isChecked());
    settings.setValue("zoomTextOnly", ui->zoomTextOnly->isChecked());
    settings.setValue("PrintElementBackground", ui->printEBackground->isChecked());
    settings.setValue("wheelScrollLines", ui->wheelScroll->value());
    settings.setValue("DoNotTrack", ui->doNotTrack->isChecked());
    settings.setValue("CheckUpdates", ui->checkUpdates->isChecked());
    settings.setValue("LoadTabsOnActivation", ui->dontLoadTabsUntilSelected->isChecked());
    settings.setValue("DefaultZoom", ui->defaultZoom->value());
    settings.setValue("XSSAuditing", ui->xssAuditing->isChecked());
    settings.setValue("UserAgent", ui->changeUserAgent->isChecked() ? ui->userAgentCombo->currentText() : "");

    //Cache
    settings.setValue("maximumCachedPages", ui->pagesInCache->value());
    settings.setValue("AllowLocalCache", ui->allowCache->isChecked());
    settings.setValue("LocalCacheSize", ui->cacheMB->value());
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

    //Cookies
    settings.beginGroup("Cookie-Settings");
    settings.setValue("allowCookies", ui->saveCookies->isChecked());
    settings.setValue("deleteCookiesOnClose", ui->deleteCookiesOnClose->isChecked());
    settings.setValue("allowCookiesFromVisitedDomainOnly", ui->matchExactly->isChecked());
    settings.setValue("filterTrackingCookie", ui->filterTracking->isChecked());
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
    settings.setValue("SelectAllTextOnDoubleClick", ui->selectAllOnFocus->isChecked());
    settings.setValue("SelectAllTextOnClick", ui->selectAllOnClick->isChecked());
    settings.setValue("AddCountryDomainWithAltKey", ui->addCountryWithAlt->isChecked());
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

    settings.setValue("ProxyExceptions", ui->proxyExceptions->text().split(","));
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
    if (m_notification) {
        delete m_notification.data();
    }
}
