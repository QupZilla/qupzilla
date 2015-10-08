/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QString>

#include "qzcommon.h"

class ProfileManager
{
public:
    explicit ProfileManager();

    // Make sure the config dir exists and have correct structure
    void initConfigDir() const;
    // Set current profile name (from profiles.ini) and ensure dir exists with correct structure
    void initCurrentProfile(const QString &profileName);

    // Return 0 on success, -1 profile already exists, -2 cannot create directory
    int createProfile(const QString &profileName);
    // Return false on error (profile does not exists)
    bool removeProfile(const QString &profileName);

    // Name of current profile
    QString currentProfile() const;

    // Name of starting profile
    QString startingProfile() const;
    void setStartingProfile(const QString &profileName);

    // Names of available profiles
    QStringList availableProfiles() const;

private:
    void updateCurrentProfile();
    void updateProfile(const QString &current, const QString &profile);
    void copyDataToProfile();

    void connectDatabase();

    void update100();
    void update118();
    void update120();
    void update130();
    void update140();
    void update160();
    void update186();

    bool m_databaseConnected;
};

#endif // PROFILEMANAGER_H
