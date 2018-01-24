/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2013-2018 David Rosca <nowrep@gmail.com>
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
#include "sbi_iconsmanager.h"
#include "sbi_imagesicon.h"
#include "sbi_javascripticon.h"
#include "sbi_zoomwidget.h"
#include "sbi_networkicon.h"
#include "sbi_networkmanager.h"
#include "browserwindow.h"
#include "statusbar.h"

#include <QSettings>

SBI_IconsManager::SBI_IconsManager(const QString &settingsPath, QObject* parent)
    : QObject(parent)
    , m_settingsPath(settingsPath)
    , m_showImagesIcon(false)
    , m_showJavaScriptIcon(false)
    , m_showNetworkIcon(false)
    , m_showZoomWidget(false)
    , m_networkManager(0)
{
    loadSettings();
}

void SBI_IconsManager::loadSettings()
{
    QSettings settings(m_settingsPath + QL1S("/extensions.ini"), QSettings::IniFormat);
    settings.beginGroup("StatusBarIcons");
    m_showImagesIcon = settings.value("showImagesIcon", true).toBool();
    m_showJavaScriptIcon = settings.value("showJavaScriptIcon", true).toBool();
    m_showNetworkIcon = settings.value("showNetworkIcon", true).toBool();
    m_showZoomWidget = settings.value("showZoomWidget", true).toBool();
    settings.endGroup();
}

bool SBI_IconsManager::showImagesIcon() const
{
    return m_showImagesIcon;
}

void SBI_IconsManager::setShowImagesIcon(bool show)
{
    QSettings settings(m_settingsPath + QL1S("/extensions.ini"), QSettings::IniFormat);
    settings.setValue("StatusBarIcons/showImagesIcon", show);

    m_showImagesIcon = show;
}

bool SBI_IconsManager::showJavaScriptIcon() const
{
    return m_showJavaScriptIcon;
}

void SBI_IconsManager::setShowJavaScriptIcon(bool show)
{
    QSettings settings(m_settingsPath + QL1S("/extensions.ini"), QSettings::IniFormat);
    settings.setValue("StatusBarIcons/showJavaScriptIcon", show);

    m_showJavaScriptIcon = show;
}

bool SBI_IconsManager::showNetworkIcon() const
{
    return m_showNetworkIcon;
}

void SBI_IconsManager::setShowNetworkIcon(bool show)
{
    QSettings settings(m_settingsPath + QL1S("/extensions.ini"), QSettings::IniFormat);
    settings.setValue("StatusBarIcons/showNetworkIcon", show);

    m_showNetworkIcon = show;
}

bool SBI_IconsManager::showZoomWidget() const
{
    return m_showZoomWidget;
}

void SBI_IconsManager::setShowZoomWidget(bool show)
{
    QSettings settings(m_settingsPath + QL1S("/extensions.ini"), QSettings::IniFormat);
    settings.setValue("StatusBarIcons/showZoomWidget", show);

    m_showZoomWidget = show;
}

void SBI_IconsManager::reloadIcons()
{
    QHashIterator<BrowserWindow*, QWidgetList> it(m_windows);

    while (it.hasNext()) {
        it.next();
        mainWindowDeleted(it.key());
        mainWindowCreated(it.key());
    }
}

void SBI_IconsManager::destroyIcons()
{
    QHashIterator<BrowserWindow*, QWidgetList> it(m_windows);

    while (it.hasNext()) {
        it.next();
        mainWindowDeleted(it.key());
    }
}

void SBI_IconsManager::mainWindowCreated(BrowserWindow* window)
{
    if (m_showImagesIcon) {
        SBI_ImagesIcon* w = new SBI_ImagesIcon(window, m_settingsPath);
        window->statusBar()->addPermanentWidget(w);
        m_windows[window].append(w);
    }

    if (m_showJavaScriptIcon) {
        SBI_JavaScriptIcon* w = new SBI_JavaScriptIcon(window);
        window->statusBar()->addPermanentWidget(w);
        m_windows[window].append(w);
    }

    if (m_showNetworkIcon) {
        if (!m_networkManager) {
            m_networkManager = new SBI_NetworkManager(m_settingsPath, this);
        }

        SBI_NetworkIcon* w = new SBI_NetworkIcon(window);
        window->statusBar()->addPermanentWidget(w);
        m_windows[window].append(w);
    }

    if (m_showZoomWidget) {
        SBI_ZoomWidget* w = new SBI_ZoomWidget(window);
        window->statusBar()->addPermanentWidget(w);
        m_windows[window].append(w);
    }
}

void SBI_IconsManager::mainWindowDeleted(BrowserWindow* window)
{
    foreach (QWidget* w, m_windows[window]) {
        window->statusBar()->removeWidget(w);
        delete w;
    }

    m_windows[window].clear();
}

SBI_IconsManager::~SBI_IconsManager()
{
    delete m_networkManager;
}
