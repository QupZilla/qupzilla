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
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QPointer>

#include "qz_namespace.h"

namespace Ui
{
class Preferences;
}

class QAbstractButton;
class QListWidgetItem;

class AutoFillManager;
class QupZilla;
class PluginsManager;
class DesktopNotification;
class ThemeManager;

class QT_QUPZILLA_EXPORT Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QupZilla* mainClass, QWidget* parent = 0);
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
    void openSslManager();
    void showAcceptLanguage();
    void chooseUserStyleClicked();
    void deleteHtml5storage();
    void chooseExternalDownloadManager();
    void openUserAgentManager();
    void openJsOptions();

    void saveHistoryChanged(bool state);
    void allowHtml5storageChanged(bool state);
    void downLocChanged(bool state);
    void allowCacheChanged(bool state);
    void showPassManager(bool state);
    void setManualProxyConfigurationEnabled(bool state);
    void useExternalDownManagerChanged(bool state);
    void useDifferentProxyForHttpsChanged(bool state);
    void showTabPreviewsChanged(bool state);
    void changeCachePathClicked();

    void newTabChanged(int value);
    void afterLaunchChanged(int value);
    void cacheValueChanged(int value);
    void pageCacheValueChanged(int value);

    void createProfile();
    void deleteProfile();
    void startProfileIndexChanged(QString index);

    void setProgressBarColorIcon(QColor col = QColor());
    void selectCustomProgressBarColor();

    void setNotificationPreviewVisible(bool state);

    void makeQupZillaDefault();

private:
    void closeEvent(QCloseEvent* event);

    Ui::Preferences* ui;
    QupZilla* p_QupZilla;
    AutoFillManager* m_autoFillManager;
    PluginsManager* m_pluginsList;
    ThemeManager* m_themesManager;
    QPointer<DesktopNotification> m_notification;

    QString m_homepage;
    QString m_newTabUrl;
    QString m_actProfileName;
    int m_afterLaunch;
    int m_onNewTab;
    QPoint m_notifPosition;
};

#endif // PREFERENCES_H
