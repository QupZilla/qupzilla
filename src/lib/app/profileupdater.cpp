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
#include "profileupdater.h"
#include "qupzilla.h"
#include "updater.h"
#include "globalfunctions.h"
#include "mainapplication.h"

#include <QDir>
#include <QSqlQuery>
#include <QMessageBox>
#include <iostream>

ProfileUpdater::ProfileUpdater(const QString &profilePath)
    : m_profilePath(profilePath)
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
    }
    else {
        copyDataToProfile();
    }

    versionFile.open(QFile::WriteOnly);
    versionFile.write(QupZilla::VERSION.toUtf8());
    versionFile.close();
}

void ProfileUpdater::updateProfile(const QString &current, const QString &profile)
{
    if (current == profile) {
        return;
    }

//    Updater::Version currentVersion = Updater::parseVersionFromString(current);
    Updater::Version profileVersion = Updater::parseVersionFromString(profile);

    if (profileVersion == Updater::parseVersionFromString("1.0.0-b4")) {
        update100b4();
        update100rc1();
        update100();
        update118();
        return;
    }

    if (profileVersion == Updater::parseVersionFromString("1.0.0-rc1")) {
        update100rc1();
        update100();
        update118();
        return;
    }

    if (profileVersion == Updater::parseVersionFromString("1.0.0")) {
        update100();
        update118();
        return;
    }

    if (profileVersion == Updater::parseVersionFromString("1.1.0")) {
        // Do nothing, nothing changed
        return;
    }

    if (profileVersion == Updater::parseVersionFromString("1.1.5")) {
        // Do nothing, nothing changed
        return;
    }

    if (profileVersion == Updater::parseVersionFromString("1.1.8")) {
        update118();
        return;
    }

    std::cout << "QupZilla: Incompatible profile version detected, overwriting profile data..." << std::endl;

    copyDataToProfile();
}

void ProfileUpdater::copyDataToProfile()
{
    QDir profileDir(m_profilePath);
    profileDir.mkdir("certificates");

    QFile browseData(m_profilePath + "browsedata.db");
    if (browseData.exists()) {
        const QString &browseDataBackup = qz_ensureUniqueFilename(m_profilePath + "browsedata-backup.db");
        const QString &settingsBackup = qz_ensureUniqueFilename(m_profilePath + "settings-backup.ini");
        browseData.copy(browseDataBackup);
        QFile(m_profilePath + "settings.ini").copy(settingsBackup);
        const QString &text = "Incompatible profile version has been detected. To avoid losing your profile data, they were "
                              "backed up in following directories:<br/><br/><b>" + browseDataBackup + "<br/>" + settingsBackup + "<br/></b>";
        QMessageBox::warning(0, "QupZilla: Incompatible profile version", text);
    }

    browseData.remove();
    QFile(":data/browsedata.db").copy(m_profilePath + "browsedata.db");
    QFile(m_profilePath + "browsedata.db").setPermissions(QFile::ReadUser | QFile::WriteUser);
}

void ProfileUpdater::update100b4()
{
    std::cout << "QupZilla: Upgrading profile version from 1.0.0-b4..." << std::endl;
    mApp->connectDatabase();

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS search_engines (id INTEGER PRIMARY KEY, name TEXT, icon TEXT,"
               "url TEXT, shortcut TEXT, suggestionsUrl TEXT, suggestionsParameters TEXT);");
}

void ProfileUpdater::update100rc1()
{
    std::cout << "QupZilla: Upgrading profile version from 1.0.0-rc1..." << std::endl;
    mApp->connectDatabase();

    QSqlQuery query;
    query.exec("ALTER TABLE folders ADD COLUMN subfolder TEXT");
    query.exec("UPDATE folders SET subfolder='no'");

    query.exec("ALTER TABLE bookmarks ADD COLUMN toolbar_position NUMERIC");
    query.exec("UPDATE bookmarks SET toolbar_position=0");

}

void ProfileUpdater::update100()
{
    std::cout << "QupZilla: Upgrading profile version from 1.0.0..." << std::endl;
    mApp->connectDatabase();

    QSqlQuery query;
    query.exec("ALTER TABLE autofill ADD COLUMN last_used NUMERIC");
    query.exec("UPDATE autofill SET last_used=0");
}

void ProfileUpdater::update118()
{
    std::cout << "QupZilla: Upgrading profile version from 1.1.8..." << std::endl;
    mApp->connectDatabase();

    QSqlQuery query;
    query.exec("ALTER TABLE folders ADD COLUMN parent TEXT");
}
