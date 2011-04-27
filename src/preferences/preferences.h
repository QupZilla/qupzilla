/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include <QFileDialog>
#include <QListWidgetItem>
#include <QColorDialog>
#include <QAbstractButton>
#include <QPointer>

namespace Ui {
    class Preferences;
}

class AutoFillManager;
class QupZilla;
class PluginsList;
class DesktopNotification;

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QupZilla* mainClass, QWidget* parent = 0);
    ~Preferences();

private slots:
    void saveSettings();
    void buttonClicked(QAbstractButton* button);

    void showStackedPage(QListWidgetItem* item);
    void newTabChanged();
    void chooseDownPath();
    void showCookieManager();
    void chooseBackgroundPath();
    void useActualHomepage();
    void useActualNewTab();
    void resetBackground();
    void chooseColor();
    void openSslManager();

    void allowJavaScriptChanged(bool stat);
    void saveHistoryChanged(bool stat);
    void saveCookiesChanged(bool stat);
    void downLocChanged(bool state);
    void allowCacheChanged(bool state);
    void showPassManager(bool state);
    void useBgImageChanged(bool state);
    void setManualProxyConfigurationEnabled(bool state);
    void cacheValueChanged(int value);
    void pageCacheValueChanged(int value);

    void createProfile();
    void deleteProfile();
    void startProfileIndexChanged(QString index);

private:
    void updateBgLabel();
    Ui::Preferences* ui;
    QupZilla* p_QupZilla;
    AutoFillManager* m_autoFillManager;
    PluginsList* m_pluginsList;
    QPointer<DesktopNotification> m_notification;

    QColor m_menuTextColor;
    QString m_homepage;
    QString m_newTabUrl;
    QString m_actProfileName;
    int m_afterLaunch;
    int m_onNewTab;
    QSize m_bgLabelSize;
    QPoint m_notifPosition;
};

#endif // PREFERENCES_H
