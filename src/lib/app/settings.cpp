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
#include "settings.h"
#include "qzsettings.h"

#include <QSettings>

QSettings* Settings::s_settings = 0;
QzSettings* Settings::s_qzSettings = 0;

Settings::Settings()
{
    // Save currently opened group
    if (!s_settings->group().isEmpty()) {
        m_openedGroup = s_settings->group();
        s_settings->endGroup();
    }
}

void Settings::createSettings(const QString &fileName)
{
    s_settings = new QSettings(fileName, QSettings::IniFormat);
    s_qzSettings = new QzSettings();
}

void Settings::syncSettings()
{
    if (!s_settings)
        return;

    s_settings->sync();
}

bool Settings::contains(const QString &key) const
{
    return s_settings->contains(key);
}

void Settings::remove(const QString &key)
{
    s_settings->remove(key);
}

void Settings::setValue(const QString &key, const QVariant &defaultValue)
{
    s_settings->setValue(key, defaultValue);
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue)
{
    return s_settings->value(key, defaultValue);
}

void Settings::beginGroup(const QString &prefix)
{
    s_settings->beginGroup(prefix);
}

void Settings::endGroup()
{
    s_settings->endGroup();
}

void Settings::sync()
{
    s_settings->sync();
}

QSettings* Settings::globalSettings()
{
    return s_settings;
}

QzSettings* Settings::staticSettings()
{
    return s_qzSettings;
}

Settings::~Settings()
{
    if (!s_settings->group().isEmpty()) {
        qDebug() << "Settings: Deleting object with opened group!";
        s_settings->endGroup();
    }

    // Restore opened group
    if (!m_openedGroup.isEmpty())
        s_settings->beginGroup(m_openedGroup);
}
