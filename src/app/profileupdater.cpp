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
#include "profileupdater.h"
#include "qupzilla.h"
#include "updater.h"

ProfileUpdater::ProfileUpdater(const QString &profilePath, const QString &dataPath)
    : QObject()
    , m_profilePath(profilePath)
    , m_dataPath(dataPath)
{
}

void ProfileUpdater::checkProfile()
{
    QDir profileDir(m_profilePath);

    if (!profileDir.exists()) {
        QDir newDir(profileDir.path().remove(profileDir.dirName()));
        newDir.mkdir(profileDir.dirName());
    }

    QFile versionFile(m_profilePath + "version");
    if (versionFile.exists()) {
        versionFile.open(QFile::ReadOnly);
        QString profileVersion = versionFile.readAll();
        versionFile.close();
        versionFile.remove();

        updateProfile(QupZilla::VERSION, profileVersion.trimmed());
    } else
        copyDataToProfile();

    versionFile.open(QFile::WriteOnly);
    versionFile.write(QupZilla::VERSION.toAscii());
    versionFile.close();
}

void ProfileUpdater::updateProfile(const QString &current, const QString &profile)
{
    if (current == profile)
        return;

//    Updater::Version currentVersion = Updater::parseVersionFromString(current);
    Updater::Version profileVersion = Updater::parseVersionFromString(profile);

    if (profileVersion == Updater::parseVersionFromString("1.0.0-b4")) {
        update100b4();
        return;
    }

    std::cout << "incompatible profile version detected, updating profile data..." << std::endl;

    copyDataToProfile();
}

void ProfileUpdater::copyDataToProfile()
{
    QDir profileDir(m_profilePath);
    profileDir.mkdir("certificates");

    QFile(m_profilePath + "browsedata.db").remove();
    QFile(m_dataPath + "data/default/profiles/default/browsedata.db").copy(m_profilePath + "browsedata.db");
}

void ProfileUpdater::update100b4()
{
    std::cout << "upgrading profile version from 1.0.0-b4..." << std::endl;

    QSqlQuery query;
    query.exec("CREATE TABLE search_engines (id INTEGER PRIMARY KEY, name TEXT, icon TEXT,"
               "url TEXT, shortcut TEXT, suggestionsUrl TEXT, suggestionsParameters TEXT);");
}
