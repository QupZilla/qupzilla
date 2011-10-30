/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include "historymodel.h"
#include "tabwidget.h"
#include "cookiejar.h"
#include "locationbar.h"
#include "autofillmanager.h"
#include "mainapplication.h"
#include "cookiemanager.h"
#include "pluginslist.h"
#include "qtwin.h"
#include "pluginproxy.h"
#include "sslmanager.h"
#include "networkproxyfactory.h"
#include "networkmanager.h"
#include "desktopnotificationsfactory.h"
#include "desktopnotification.h"
#include "navigationbar.h"
#include "thememanager.h"
#include "acceptlanguage.h"
#include "globalfunctions.h"

bool removeFile(const QString &fullFileName)
{
    QFile f(fullFileName);
    if (f.exists())
        return f.remove();
    else return false;
}

void removeDir(const QString &d)
{
    QDir dir(d);
    if (dir.exists())
    {
        const QFileInfoList list = dir.entryInfoList();
        QFileInfo fi;
        for (int l = 0; l < list.size(); l++)
        {
            fi = list.at(l);
            if (fi.isDir() && fi.fileName() != "." && fi.fileName() != "..")
                removeDir(fi.absoluteFilePath());
            else if (fi.isFile())
                removeFile(fi.absoluteFilePath());

        }
        dir.rmdir(d);
    }
}

Preferences::Preferences(QupZilla* mainClass, QWidget* parent) :
    QDialog(parent)
    ,ui(new Ui::Preferences)
    ,p_QupZilla(mainClass)
    ,m_pluginsList(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    m_bgLabelSize = this->sizeHint();

    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    //GENERAL URLs
    settings.beginGroup("Web-URL-Settings");
    m_homepage = settings.value("homepage","qupzilla:start").toString();
    m_newTabUrl = settings.value("newTabUrl","").toString();
    ui->homepage->setText(m_homepage);
    ui->newTabUrl->setText(m_newTabUrl);
    int afterLaunch = settings.value("afterLaunch",1).toInt();
    settings.endGroup();
    ui->afterLaunch->setCurrentIndex(afterLaunch);
    ui->checkUpdates->setChecked( settings.value("Web-Browser-Settings/CheckUpdates", true).toBool() );

    ui->newTabFrame->setVisible(false);
    if (m_newTabUrl.isEmpty())
        ui->newTab->setCurrentIndex(0);
    else if (m_newTabUrl == m_homepage)
        ui->newTab->setCurrentIndex(1);
    else{
        ui->newTab->setCurrentIndex(2);
        ui->newTabFrame->setVisible(true);
    }
    connect(ui->newTab, SIGNAL(currentIndexChanged(int)), this, SLOT(newTabChanged()));
    connect(ui->useActualBut, SIGNAL(clicked()), this, SLOT(useActualHomepage()));
    connect(ui->newTabUseActual, SIGNAL(clicked()), this, SLOT(useActualNewTab()));

    //PROFILES
    QString homePath = QDir::homePath();
    homePath += "/.qupzilla/";
    QSettings profileSettings(homePath + "profiles/profiles.ini", QSettings::IniFormat);
    m_actProfileName = profileSettings.value("Profiles/startProfile", "default").toString();

    ui->startProfile->addItem(m_actProfileName);
    QDir profilesDir(QDir::homePath() + "/.qupzilla/profiles/");
    QStringList list_ = profilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (QString name, list_) {
        if (m_actProfileName == name)
            continue;
        ui->startProfile->addItem(name);
    }
    connect(ui->createProfile, SIGNAL(clicked()), this, SLOT(createProfile()));
    connect(ui->deleteProfile, SIGNAL(clicked()), this, SLOT(deleteProfile()));
    connect(ui->startProfile, SIGNAL(currentIndexChanged(QString)), this, SLOT(startProfileIndexChanged(QString)));

    //APPEREANCE
    m_themesManager = new ThemeManager(ui->themesWidget);
    settings.beginGroup("Browser-View-Settings");
    ui->showStatusbar->setChecked( settings.value("showStatusBar",true).toBool() );
    ui->showBookmarksToolbar->setChecked( p_QupZilla->bookmarksToolbar()->isVisible() );
    ui->showNavigationToolbar->setChecked( p_QupZilla->navigationBar()->isVisible() );
    ui->showHome->setChecked( settings.value("showHomeButton",true).toBool() );
    ui->showBackForward->setChecked( settings.value("showBackForwardButtons",true).toBool() );
    ui->showAddTabButton->setChecked( settings.value("showAddTabButton", false).toBool() );
    ui->useTransparentBg->setChecked( settings.value("useTransparentBackground",false).toBool() );
    settings.endGroup();
#ifdef Q_WS_WIN
    ui->useTransparentBg->setEnabled(QtWin::isCompositionEnabled());
#endif

    //TABS
    settings.beginGroup("Browser-Tabs-Settings");
    ui->makeMovable->setChecked( settings.value("makeTabsMovable",true).toBool() );
    ui->hideCloseOnTab->setChecked( settings.value("hideCloseButtonWithOneTab",false).toBool() );
    ui->hideTabsOnTab->setChecked( settings.value("hideTabsWithOneTab",false).toBool() );
    ui->activateLastTab->setChecked( settings.value("ActivateLastTabWhenClosingActual", false).toBool() );
    ui->askWhenClosingMultipleTabs->setChecked( settings.value("AskOnClosing", false).toBool() );
    settings.endGroup();
    //AddressBar
    settings.beginGroup("AddressBar");
    ui->selectAllOnFocus->setChecked( settings.value("SelectAllTextOnDoubleClick",true).toBool() );
    ui->addComWithCtrl->setChecked( settings.value("AddComDomainWithCtrlKey",false).toBool() );
    ui->addCountryWithAlt->setChecked( settings.value("AddCountryDomainWithAltKey",true).toBool() );
    settings.endGroup();

    //BROWSING
    settings.beginGroup("Web-Browser-Settings");
    ui->allowPlugins->setChecked( settings.value("allowFlash",true).toBool() );
    ui->allowJavaScript->setChecked( settings.value("allowJavaScript",true).toBool() );
    ui->blockPopup->setChecked( !settings.value("allowJavaScriptOpenWindow", false).toBool() );
    ui->allowJava->setChecked( settings.value("allowJava",true).toBool() );
    ui->loadImages->setChecked( settings.value("autoLoadImages",true).toBool() );
    ui->allowDNSPrefetch->setChecked( settings.value("DNS-Prefetch", false).toBool() );
    ui->jscanAccessClipboard->setChecked( settings.value("JavaScriptCanAccessClipboard", true).toBool() );
    ui->linksInFocusChain->setChecked( settings.value("IncludeLinkInFocusChain", false).toBool() );
    ui->zoomTextOnly->setChecked( settings.value("zoomTextOnly", false).toBool() );
    ui->printEBackground->setChecked( settings.value("PrintElementBackground", true).toBool() );
    ui->wheelScroll->setValue( settings.value("wheelScrollLines", qApp->wheelScrollLines()).toInt() );
    ui->doNotTrack->setChecked( settings.value("DoNotTrack", false).toBool() );
    ui->defaultZoom->setValue( settings.value("DefaultZoom", 100).toInt() );

    if (!ui->allowJavaScript->isChecked())
        ui->blockPopup->setEnabled(false);
    connect(ui->allowJavaScript, SIGNAL(toggled(bool)), this, SLOT(allowJavaScriptChanged(bool)));
    //Cache
    ui->pagesInCache->setValue( settings.value("maximumCachedPages",3).toInt() );
    connect(ui->pagesInCache, SIGNAL(valueChanged(int)), this, SLOT(pageCacheValueChanged(int)));
    ui->pageCacheLabel->setText(QString::number(ui->pagesInCache->value()));

    ui->allowCache->setChecked( settings.value("AllowLocalCache",true).toBool() );
    ui->cacheMB->setValue( settings.value("LocalCacheSize", 50).toInt() );
    ui->MBlabel->setText( settings.value("LocalCacheSize", 50).toString() + " MB");
    connect(ui->allowCache, SIGNAL(clicked(bool)), this, SLOT(allowCacheChanged(bool)));
    connect(ui->cacheMB, SIGNAL(valueChanged(int)), this, SLOT(cacheValueChanged(int)) );
    allowCacheChanged(ui->allowCache->isChecked());

    //PASSWORD MANAGER
    ui->allowPassManager->setChecked(settings.value("AutoFillForms",true).toBool());
    connect(ui->allowPassManager, SIGNAL(toggled(bool)), this, SLOT(showPassManager(bool)));

    m_autoFillManager = new AutoFillManager(this);
    ui->autoFillFrame->addWidget(m_autoFillManager);

    //PRIVACY
    //Web storage
    ui->storeIcons->setChecked( settings.value("allowPersistentStorage",true).toBool() );
    ui->saveHistory->setChecked( mApp->history()->isSaving() );
    ui->deleteHistoryOnClose->setChecked( settings.value("deleteHistoryOnClose",false).toBool() );
    if (!ui->saveHistory->isChecked())
        ui->deleteHistoryOnClose->setEnabled(false);
    connect(ui->saveHistory, SIGNAL(toggled(bool)), this, SLOT(saveHistoryChanged(bool)));
    //Cookies
    ui->saveCookies->setChecked( settings.value("allowCookies",true).toBool() );
    if (!ui->saveCookies->isChecked())
        ui->deleteCookiesOnClose->setEnabled(false);
    connect(ui->saveCookies, SIGNAL(toggled(bool)), this, SLOT(saveCookiesChanged(bool)));
    ui->deleteCookiesOnClose->setChecked( settings.value("deleteCookiesOnClose", false).toBool() );
    ui->matchExactly->setChecked( settings.value("allowCookiesFromVisitedDomainOnly",false).toBool() );
    ui->filterTracking->setChecked( settings.value("filterTrackingCookie",false).toBool() );

    //CSS Style
    ui->userStyleSheet->setText( settings.value("userStyleSheet", "").toString() );
    connect(ui->chooseUserStylesheet, SIGNAL(clicked()), this, SLOT(chooseUserStyleClicked()));
    settings.endGroup();

    //DOWNLOADS
    settings.beginGroup("DownloadManager");
    ui->downLoc->setText( settings.value("defaultDownloadPath","").toString() );
    ui->closeDownManOnFinish->setChecked( settings.value("CloseManagerOnFinish", false).toBool() );
    ui->downlaodNativeSystemDialog->setChecked( settings.value("useNativeDialog",
#ifdef Q_WS_WIN
    false
#else
    true
#endif
    ).toBool() );
    if (ui->downLoc->text().isEmpty())
        ui->askEverytime->setChecked(true);
    else
        ui->useDefined->setChecked(true);
    connect(ui->useDefined, SIGNAL(toggled(bool)), this, SLOT(downLocChanged(bool)));
    connect(ui->downButt, SIGNAL(clicked()), this, SLOT(chooseDownPath()));
    downLocChanged(ui->useDefined->isChecked());
    settings.endGroup();

    //FONTS
    settings.beginGroup("Browser-Fonts");
    ui->fontStandard->setCurrentFont(QFont( settings.value("StandardFont", mApp->webSettings()->fontFamily(QWebSettings::StandardFont)).toString() ));
    ui->fontCursive->setCurrentFont(QFont( settings.value("CursiveFont", mApp->webSettings()->fontFamily(QWebSettings::CursiveFont)).toString() ));
    ui->fontFantasy->setCurrentFont(QFont( settings.value("FantasyFont", mApp->webSettings()->fontFamily(QWebSettings::FantasyFont)).toString() ));
    ui->fontFixed->setCurrentFont(QFont( settings.value("FixedFont", mApp->webSettings()->fontFamily(QWebSettings::FixedFont)).toString() ));
    ui->fontSansSerif->setCurrentFont(QFont( settings.value("SansSerifFont", mApp->webSettings()->fontFamily(QWebSettings::SansSerifFont)).toString() ));
    ui->fontSerif->setCurrentFont(QFont( settings.value("SerifFont", mApp->webSettings()->fontFamily(QWebSettings::SerifFont)).toString() ));

    ui->sizeDefault->setValue( settings.value("DefaultFontSize", mApp->webSettings()->fontSize(QWebSettings::DefaultFontSize)).toInt() );
    ui->sizeFixed->setValue( settings.value("FixedFontSize", mApp->webSettings()->fontSize(QWebSettings::DefaultFixedFontSize)).toInt() );
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
    if (notifyType == DesktopNotificationsFactory::DesktopNative)
        ui->useNativeSystemNotifications->setChecked(true);
    else
        ui->useOSDNotifications->setChecked(true);

    ui->doNotUseNotifications->setChecked(!settings.value("Enabled", true).toBool());
    m_notifPosition = settings.value("Position", QPoint(10,10)).toPoint();
    settings.endGroup();

    //OTHER
    //Languages
    QString activeLanguage="";
    if (!p_QupZilla->activeLanguage().isEmpty()) {
        activeLanguage = p_QupZilla->activeLanguage();
        QString loc = activeLanguage;
        loc.remove(".qm");
        QLocale locale(loc);
        QString country = QLocale::countryToString(locale.country());
        QString language = QLocale::languageToString(locale.language());
        ui->languages->addItem(language+", "+country+" ("+loc+")", activeLanguage);
    }
    ui->languages->addItem("English (en_US)");

    QDir lanDir(mApp->TRANSLATIONSDIR);
    QStringList list = lanDir.entryList(QStringList("*.qm"));
    foreach(QString name, list) {
        if (name.startsWith("qt_") || name == activeLanguage)
            continue;

        QString loc = name;
        loc.remove(".qm");
        QLocale locale(loc);
        QString country = QLocale::countryToString(locale.country());
        QString language = QLocale::languageToString(locale.language());
        ui->languages->addItem(language+", "+country+" ("+loc+")", name);
    }
    //Proxy Config
    settings.beginGroup("Web-Proxy");
    NetworkProxyFactory::ProxyPreference proxyPreference = NetworkProxyFactory::ProxyPreference(settings.value("UseProxy", NetworkProxyFactory::SystemProxy).toInt());
    QNetworkProxy::ProxyType proxyType = QNetworkProxy::ProxyType(settings.value("ProxyType", QNetworkProxy::HttpProxy).toInt());

    connect(ui->manualProxy, SIGNAL(toggled(bool)), this, SLOT(setManualProxyConfigurationEnabled(bool)));
    ui->systemProxy->setChecked(proxyPreference == NetworkProxyFactory::SystemProxy);
    ui->noProxy->setChecked(proxyPreference == NetworkProxyFactory::NoProxy);
    ui->manualProxy->setChecked(proxyPreference == NetworkProxyFactory::DefinedProxy);
    setManualProxyConfigurationEnabled(proxyPreference == NetworkProxyFactory::DefinedProxy);
    if (proxyType == QNetworkProxy::HttpProxy)
        ui->proxyType->setCurrentIndex(0);
    else
        ui->proxyType->setCurrentIndex(1);

    ui->proxyServer->setText(settings.value("HostName", "").toString());
    ui->proxyPort->setText(settings.value("Port", 8080).toString());
    ui->proxyUsername->setText(settings.value("Username", "").toString());
    ui->proxyPassword->setText(settings.value("Password", "").toString());
    ui->proxyExceptions->setText(settings.value("ProxyExceptions", QStringList() << "localhost" << "127.0.0.1").toStringList().join(","));
    settings.endGroup();

    //CONNECTS
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
    connect(ui->cookieManagerBut, SIGNAL(clicked()), this, SLOT(showCookieManager()));
    connect(ui->sslManagerButton, SIGNAL(clicked()), this, SLOT(openSslManager()));
    connect(ui->preferredLanguages, SIGNAL(clicked()), this, SLOT(showAcceptLanguage()));

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(showStackedPage(QListWidgetItem*)));
    ui->listWidget->setItemSelected(ui->listWidget->itemAt(5,5), true);

    ui->version->setText(" QupZilla v"+QupZilla::VERSION);
}

void Preferences::showStackedPage(QListWidgetItem* item)
{
    if (!item)
        return;
   ui->caption->setText("<b>"+item->text()+"</b>");
   ui->stackedWidget->setCurrentIndex(item->whatsThis().toInt());

   if (ui->stackedWidget->currentIndex() == 8) {
       m_notification = new DesktopNotification(true);
       m_notification->setPixmap(QPixmap(":icons/preferences/stock_dialog-question.png"));
       m_notification->setHeading(tr("OSD Notification"));
       m_notification->setText(tr("Drag it on the screen to place it where you want."));
       m_notification->move(m_notifPosition);
       m_notification->show();
   } else if (m_notification) {
       m_notifPosition = m_notification->pos();
       delete m_notification;
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
    if (userFileName.isEmpty())
        return;
#ifdef Q_WS_WIN   //QFileDialog::getExistingDirectory returns path with \ instead of / (??)
    userFileName.replace("\\", "/");
#endif
    userFileName += "/";

    ui->downLoc->setText(userFileName);
}

void Preferences::chooseUserStyleClicked()
{
    QString file = QFileDialog::getOpenFileName(p_QupZilla, tr("Choose stylesheet location..."), QDir::homePath(), "*.css");
    if (file.isEmpty())
        return;
    ui->userStyleSheet->setText(file);
}

void Preferences::newTabChanged()
{
    if (ui->newTab->currentIndex() == 2)
        ui->newTabFrame->setVisible(true);
    else
        ui->newTabFrame->setVisible(false);
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
    ui->proxyExceptions->setEnabled(state);
}

void Preferences::allowJavaScriptChanged(bool stat)
{
    ui->blockPopup->setEnabled(stat);
}

void Preferences::saveHistoryChanged(bool stat)
{
    ui->deleteHistoryOnClose->setEnabled(stat);
}

void Preferences::saveCookiesChanged(bool stat)
{
    ui->deleteCookiesOnClose->setEnabled(stat);
}

void Preferences::showCookieManager()
{
    CookieManager* m = new CookieManager();
    m->refreshTable();
    m->show();
}

void Preferences::openSslManager()
{
    SSLManager* m = new SSLManager(this);
//    qz_centerWidgetToParent(m, this);
    m->show();
}

void Preferences::showAcceptLanguage()
{
    AcceptLanguage a(this);
    a.exec();
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
    if (name.isEmpty() || name.contains("/") || name.contains("\\"))
        return;
    QDir dir(QDir::homePath()+"/.qupzilla/profiles/");
    if (QDir(dir.absolutePath() + "/" + name).exists()) {
        QMessageBox::warning(this, tr("Error!"), tr("This profile already exists!"));
        return;
    }
    if (!dir.mkdir(name)) {
        QMessageBox::warning(this, tr("Error!"), tr("Cannot create profile directory!"));
        return;
    }
    dir.cd(name);
    QFile(mApp->DATADIR+"data/default/profiles/default/browsedata.db").copy(dir.absolutePath()+"/browsedata.db");

    ui->startProfile->insertItem(0, name);
    ui->startProfile->setCurrentIndex(0);
}

void Preferences::deleteProfile()
{
    QString name = ui->startProfile->currentText();
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                         tr("Are you sure to permanently delete \"%1\" profile? This action cannot be undone!").arg(name), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes)
        return;

    removeDir(QDir::homePath()+"/.qupzilla/profiles/"+name);
    ui->startProfile->removeItem(ui->startProfile->currentIndex());
}

void Preferences::startProfileIndexChanged(QString index)
{
    ui->deleteProfile->setEnabled(m_actProfileName != index);

    if (m_actProfileName == index)
        ui->cannotDeleteActiveProfileLabel->setText(tr("Note: You cannot delete active profile."));
    else
        ui->cannotDeleteActiveProfileLabel->setText(" ");
}

void Preferences::saveSettings()
{
    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    //GENERAL URLs
    settings.beginGroup("Web-URL-Settings");
    settings.setValue("homepage",ui->homepage->text());

    QString homepage = ui->homepage->text();
    settings.setValue("afterLaunch",ui->afterLaunch->currentIndex() );


    if (ui->newTab->currentIndex() == 0)
        settings.setValue("newTabUrl","");
    else if (ui->newTab->currentIndex() == 1)
        settings.setValue("newTabUrl",homepage);
    else
        settings.setValue("newTabUrl",ui->newTabUrl->text());

    settings.endGroup();
    //PROFILES
    /*
     *
     *
     *
     */

    //WINDOW
    settings.beginGroup("Browser-View-Settings");

    settings.setValue("showStatusbar",ui->showStatusbar->isChecked());
    settings.setValue("showBookmarksToolbar", ui->showBookmarksToolbar->isChecked());
    settings.setValue("showNavigationToolbar", ui->showNavigationToolbar->isChecked());
    settings.setValue("showHomeButton", ui->showHome->isChecked());
    settings.setValue("showBackForwardButtons",ui->showBackForward->isChecked());
    settings.setValue("useTransparentBackground", ui->useTransparentBg->isChecked());
    settings.setValue("menuTextColor", m_menuTextColor);
    settings.setValue("showAddTabButton", ui->showAddTabButton->isChecked());
    settings.endGroup();

    //TABS
    settings.beginGroup("Browser-Tabs-Settings");
    settings.setValue("makeTabsMovable",ui->makeMovable->isChecked() );
    settings.setValue("hideCloseButtonWithOneTab",ui->hideCloseOnTab->isChecked());
    settings.setValue("hideTabsWithOneTab",ui->hideTabsOnTab->isChecked() );
    settings.setValue("ActivateLastTabWhenClosingActual", ui->activateLastTab->isChecked());
    settings.setValue("AskOnClosing", ui->askWhenClosingMultipleTabs->isChecked());
    settings.endGroup();

    //DOWNLOADS
    settings.beginGroup("DownloadManager");
    if (ui->askEverytime->isChecked())
        settings.setValue("defaultDownloadPath","");
    else
        settings.setValue("defaultDownloadPath", ui->downLoc->text());
    settings.setValue("CloseManagerOnFinish", ui->closeDownManOnFinish->isChecked());
    settings.setValue("useNativeDialog", ui->downlaodNativeSystemDialog->isChecked());
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
    settings.endGroup();

    //BROWSING
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("allowFlash",ui->allowPlugins->isChecked());
    settings.setValue("allowJavaScript",ui->allowJavaScript->isChecked());
    settings.setValue("allowJavaScriptOpenWindow", !ui->blockPopup->isChecked());
    settings.setValue("allowJava",ui->allowJava->isChecked());
    settings.setValue("autoLoadImages",ui->loadImages->isChecked());
    settings.setValue("maximumCachedPages",ui->pagesInCache->value());
    settings.setValue("DNS-Prefetch", ui->allowDNSPrefetch->isChecked());
    settings.setValue("JavaScriptCanAccessClipboard", ui->jscanAccessClipboard->isChecked());
    settings.setValue("IncludeLinkInFocusChain", ui->linksInFocusChain->isChecked());
    settings.setValue("zoomTextOnly", ui->zoomTextOnly->isChecked());
    settings.setValue("PrintElementBackground", ui->printEBackground->isChecked());
    settings.setValue("wheelScrollLines", ui->wheelScroll->value());
    settings.setValue("DoNotTrack", ui->doNotTrack->isChecked());
    settings.setValue("CheckUpdates", ui->checkUpdates->isChecked());
    settings.setValue("DefaultZoom", ui->defaultZoom->value());
    //Cache
    settings.setValue("AllowLocalCache", ui->allowCache->isChecked());
    settings.setValue("LocalCacheSize", ui->cacheMB->value());
    //CSS Style
    settings.setValue("userStyleSheet", ui->userStyleSheet->text());

    //PRIVACY
    //Web storage
    settings.setValue("allowPersistentStorage", ui->storeIcons->isChecked());
    //ui->saveHistory->setChecked( p_QupZilla->history->isSaving() );
    settings.setValue("deleteHistoryOnClose",ui->deleteHistoryOnClose->isChecked());

    //Cookies
    settings.setValue("allowCookies",ui->saveCookies->isChecked());
    settings.setValue("deleteCookiesOnClose", ui->deleteCookiesOnClose->isChecked());
    settings.setValue("allowCookiesFromVisitedDomainOnly",ui->matchExactly->isChecked() );
    settings.setValue("filterTrackingCookie",ui->filterTracking->isChecked() );
    settings.endGroup();

    //NOTIFICATIONS
    settings.beginGroup("Notifications");
    settings.setValue("Timeout", ui->notificationTimeout->value() * 1000);
    settings.setValue("Enabled", !ui->doNotUseNotifications->isChecked());
    settings.setValue("UseNativeDesktop", ui->useNativeSystemNotifications->isChecked());
    settings.setValue("Position", m_notification ? m_notification->pos() : m_notifPosition);
    settings.endGroup();

    //OTHER
    //AddressBar
    settings.beginGroup("AddressBar");
    settings.setValue("SelectAllTextOnDoubleClick",ui->selectAllOnFocus->isChecked() );
    settings.setValue("AddComDomainWithCtrlKey",ui->addComWithCtrl->isChecked() );
    settings.setValue("AddCountryDomainWithAltKey", ui->addCountryWithAlt->isChecked() );
    settings.endGroup();
    //Languages
    settings.beginGroup("Language");
    settings.setValue("language",ui->languages->itemData(ui->languages->currentIndex()).toString());
    settings.endGroup();
    //Proxy Configuration
    NetworkProxyFactory::ProxyPreference proxyPreference;
    if (ui->systemProxy->isChecked())
        proxyPreference = NetworkProxyFactory::SystemProxy;
    else if (ui->noProxy->isChecked())
        proxyPreference = NetworkProxyFactory::NoProxy;
    else
        proxyPreference = NetworkProxyFactory::DefinedProxy;

    QNetworkProxy::ProxyType proxyType;
    if (ui->proxyType->currentIndex() == 0)
        proxyType = QNetworkProxy::HttpProxy;
    else
        proxyType = QNetworkProxy::Socks5Proxy;

    settings.beginGroup("Web-Proxy");
    settings.setValue("ProxyType", proxyType);
    settings.setValue("UseProxy", proxyPreference);
    settings.setValue("HostName", ui->proxyServer->text());
    settings.setValue("Port", ui->proxyPort->text().toInt());
    settings.setValue("Username", ui->proxyUsername->text());
    settings.setValue("Password", ui->proxyPassword->text());
    settings.setValue("ProxyExceptions", ui->proxyExceptions->text().split(","));
    settings.endGroup();

    //Profiles
    QString homePath = QDir::homePath();
    homePath += "/.qupzilla/";
    QSettings profileSettings(homePath + "profiles/profiles.ini", QSettings::IniFormat);
    profileSettings.setValue("Profiles/startProfile", ui->startProfile->currentText());

    m_pluginsList->save();
    m_themesManager->save();
    mApp->cookieJar()->loadSettings();
    mApp->history()->loadSettings();
    mApp->reloadSettings();
    mApp->plugins()->c2f_saveSettings();
    mApp->networkManager()->loadSettings();
    mApp->desktopNotifications()->loadSettings();
}

Preferences::~Preferences()
{
    delete ui;
    delete m_autoFillManager;
    delete m_pluginsList;
    if (m_notification)
        delete m_notification;
}
