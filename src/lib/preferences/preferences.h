/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QUrl>
#include <QDialog>
#include <QPointer>

#include "qzcommon.h"

namespace Ui
{
class Preferences;
}

class QAbstractButton;
class QListWidgetItem;

class AutoFillManager;
class BrowserWindow;
class PluginsManager;
class DesktopNotification;
class ThemeManager;

class QUPZILLA_EXPORT Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(BrowserWindow* window);
    ~Preferences();

private slots:
    void saveSettings();

    void buttonClicked(QAbstractButton* button);
    void showStackedPage(QListWidgetItem* item);

    void chooseDownPath();
    void showCookieManager();
    void showHtml5Permissions();
    void useActualHomepage();
    void useActualNewTab();
    void showAcceptLanguage();
    void chooseUserStyleClicked();
    void deleteHtml5storage();
    void chooseExternalDownloadManager();
    void openUserAgentManager();
    void openJsOptions();
    void openSearchEnginesManager();

    void searchFromAddressBarChanged(bool state);
    void saveHistoryChanged(bool state);
    void allowHtml5storageChanged(bool state);
    void downLocChanged(bool state);
    void allowCacheChanged(bool state);
    void setManualProxyConfigurationEnabled(bool state);
    void useExternalDownManagerChanged(bool state);
    void changeCachePathClicked();

    void newTabChanged(int value);
    void afterLaunchChanged(int value);

    void createProfile();
    void deleteProfile();
    void startProfileIndexChanged(int index);

    void setProgressBarColorIcon(QColor col = QColor());
    void selectCustomProgressBarColor();

    void showNotificationPreview();

    void makeQupZillaDefault();

private:
    void closeEvent(QCloseEvent* event);

    Ui::Preferences* ui;
    BrowserWindow* m_window;
    AutoFillManager* m_autoFillManager;
    PluginsManager* m_pluginsList;
    ThemeManager* m_themesManager;
    QPointer<DesktopNotification> m_notification;

    QUrl m_homepage;
    QUrl m_newTabUrl;
    QString m_actProfileName;
    int m_afterLaunch;
    int m_onNewTab;
    QPoint m_notifPosition;
};

#endif // PREFERENCES_H
