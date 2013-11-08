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
#include "profileupdater.h"
#include "qupzilla.h"
#include "updater.h"
#include "qztools.h"
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

    Updater::Version prof(profile);

    if (prof == Updater::Version("1.0.0")) {
        update100();
        return;
    }

    if (prof == Updater::Version("1.1.0")
            || prof == Updater::Version("1.1.5")
            || prof == Updater::Version("1.1.8")) {
        update118();
        return;
    }

    if (prof == Updater::Version("1.2.0")) {
        update120();
        return;
    }

    if (prof == Updater::Version("1.3.0")
            || prof == Updater::Version("1.3.1")) {
        update130();
        return;
    }

    if (prof >= Updater::Version("1.4.0") && prof <= Updater::Version("1.5.0")) {
        update140();
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
        const QString &browseDataBackup = QzTools::ensureUniqueFilename(m_profilePath + "browsedata-backup.db");
        const QString &settingsBackup = QzTools::ensureUniqueFilename(m_profilePath + "settings-backup.ini");
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

void ProfileUpdater::update100()
{
    std::cout << "QupZilla: Upgrading profile version from 1.0.0..." << std::endl;
    mApp->connectDatabase();

    QSqlQuery query;
    query.exec("ALTER TABLE autofill ADD COLUMN last_used NUMERIC");
    query.exec("UPDATE autofill SET last_used=0");

    update118();
}

void ProfileUpdater::update118()
{
    std::cout << "QupZilla: Upgrading profile version from 1.1.8..." << std::endl;
    mApp->connectDatabase();

    QSqlQuery query;
    query.exec("ALTER TABLE folders ADD COLUMN parent TEXT");

    update120();
}

void ProfileUpdater::update120()
{
    std::cout << "QupZilla: Upgrading profile version from 1.2.0..." << std::endl;
    mApp->connectDatabase();

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    // This is actually just renaming bookmarks.toolbar_position to bookmarks.position
    QSqlQuery query;
    query.exec("ALTER TABLE bookmarks RENAME TO tmp_bookmarks");
    query.exec("CREATE TABLE bookmarks (icon TEXT, folder TEXT, id INTEGER PRIMARY KEY, title VARCHAR(200), url VARCHAR(200), position NUMERIC)");
    query.exec("INSERT INTO bookmarks(icon, folder, id, title, url, position)"
               "SELECT icon, folder, id, title, url, toolbar_position FROM tmp_bookmarks");
    query.exec("DROP TABLE tmp_bookmarks");
    query.exec("CREATE INDEX bookmarksTitle ON bookmarks(title ASC)");
    query.exec("CREATE INDEX bookmarksUrl ON bookmarks(url ASC)");

    db.commit();

    update130();
}

void ProfileUpdater::update130()
{
    std::cout << "QupZilla: Upgrading profile version from 1.3.0..." << std::endl;
    mApp->connectDatabase();

    QSqlQuery query;
    query.exec("ALTER TABLE bookmarks ADD COLUMN keyword TEXT");

    update140();
}

void ProfileUpdater::update140()
{
    std::cout << "QupZilla: Upgrading profile version from 1.4.0..." << std::endl;
    mApp->connectDatabase();

    QSqlQuery query;
    query.exec("ALTER TABLE search_engines ADD COLUMN method TEXT");
    query.exec("UPDATE search_engines SET method='GET'");
}
