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
#include "settings.h"

#include <QSettings>

QSettings* Settings::m_settings = 0;

Settings::Settings()
{
#ifdef QT_DEBUG
    if (!m_settings->group().isEmpty()) {
        qWarning("Settings: Creating object with opened group!");
        m_settings->endGroup();
    }
#endif
}

void Settings::createSettings(const QString &fileName)
{
    m_settings = new QSettings(fileName, QSettings::IniFormat);
}

void Settings::syncSettings()
{
    m_settings->sync();
}

void Settings::setValue(const QString &key, const QVariant &defaultValue)
{
    m_settings->setValue(key, defaultValue);
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue)
{
    return m_settings->value(key, defaultValue);
}

void Settings::beginGroup(const QString &prefix)
{
    m_settings->beginGroup(prefix);
}

void Settings::endGroup()
{
    m_settings->endGroup();
}

QSettings* Settings::globalSettings()
{
    return m_settings;
}

Settings::~Settings()
{
#ifdef QT_DEBUG
    if (!m_settings->group().isEmpty()) {
        qWarning("Settings: Deleting object with opened group!");
        m_settings->endGroup();
    }
#endif
}
