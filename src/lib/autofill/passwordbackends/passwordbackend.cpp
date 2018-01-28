/* ============================================================
* QupZilla - Qt web browser
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
#include "passwordbackend.h"

PasswordBackend::PasswordBackend()
    : m_active(false)
{
}

QStringList PasswordBackend::getUsernames(const QUrl &url)
{
    QStringList out;
    const auto entries = getEntries(url);
    for (const PasswordEntry &entry : entries) {
        out.append(entry.username);
    }
    return out;
}

void PasswordBackend::setActive(bool active)
{
    m_active = active;
}

bool PasswordBackend::isActive() const
{
    return m_active;
}

bool PasswordBackend::hasSettings() const
{
    return false;
}

void PasswordBackend::showSettings(QWidget* parent)
{
    Q_UNUSED(parent)
}
