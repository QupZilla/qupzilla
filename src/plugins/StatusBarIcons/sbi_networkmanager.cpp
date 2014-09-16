/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#include "sbi_networkmanager.h"
#include "sbi_networkproxy.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "datapaths.h"

#include <QSettings>

SBI_NetworkManager* SBI_NetworkManager::s_instance = 0;

SBI_NetworkManager::SBI_NetworkManager(const QString &settingsPath, QObject* parent)
    : QObject(parent)
    , m_settingsFile(settingsPath + QL1S("/networkicon.ini"))
    , m_currentProxy(0)
{
    s_instance = this;

    loadSettings();
}

SBI_NetworkManager* SBI_NetworkManager::instance()
{
    return s_instance;
}

void SBI_NetworkManager::loadSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);

    foreach (const QString &group, settings.childGroups()) {
        if (group.isEmpty()) {
            continue;
        }

        SBI_NetworkProxy* proxy = new SBI_NetworkProxy;

        settings.beginGroup(group);
        proxy->loadFromSettings(settings);
        settings.endGroup();

        m_proxies[group] = proxy;
    }

    const QString currentName = settings.value("CurrentProxy", QString()).toString();
    m_currentProxy = m_proxies.contains(currentName) ? m_proxies.value(currentName) : 0;

    applyCurrentProxy();
}

QString SBI_NetworkManager::currentProxyName() const
{
    return m_proxies.key(m_currentProxy);
}

SBI_NetworkProxy* SBI_NetworkManager::currentProxy() const
{
    return m_currentProxy;
}

void SBI_NetworkManager::setCurrentProxy(const QString &name)
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.setValue("CurrentProxy", name);

    m_currentProxy = m_proxies.contains(name) ? m_proxies.value(name) : 0;
    applyCurrentProxy();
}

void SBI_NetworkManager::saveProxy(const QString &name, SBI_NetworkProxy* proxy)
{
    if (name.isEmpty()) {
        return;
    }

    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup(name);
    proxy->saveToSettings(settings);
    settings.endGroup();

    m_proxies[name] = proxy;
}

void SBI_NetworkManager::removeProxy(const QString &name)
{
    if (name.isEmpty()) {
        return;
    }

    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup(name);
    settings.remove(QString()); // Removes all keys in current group
    settings.endGroup();

    m_proxies.remove(name);
}

QHash<QString, SBI_NetworkProxy*> SBI_NetworkManager::proxies() const
{
    return m_proxies;
}

void SBI_NetworkManager::applyCurrentProxy()
{
    if (!m_currentProxy) {
        return;
    }

    // Manually modify settings to apply proxy configuration
    QSettings settings(DataPaths::currentProfilePath() + "/settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Proxy");
    m_currentProxy->saveToSettings(settings);
    settings.endGroup();
    settings.sync();

    mApp->networkManager()->proxyFactory()->loadSettings();
}

void SBI_NetworkManager::deleteProxies()
{
    qDeleteAll(m_proxies);
    m_proxies.clear();
}

SBI_NetworkManager::~SBI_NetworkManager()
{
    deleteProxies();
}
