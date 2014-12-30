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
#include "profilemanager.h"
#include "mainapplication.h"
#include "datapaths.h"
#include "updater.h"
#include "qztools.h"

#include <QDir>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSettings>
#include <iostream>

ProfileManager::ProfileManager()
    : m_databaseConnected(false)
{
}

void ProfileManager::initConfigDir() const
{
    QDir dir(DataPaths::path(DataPaths::Config));

    if (dir.exists() && QFile(dir.filePath(QLatin1String("profiles/profiles.ini"))).exists()) {
        return;
    }

    std::cout << "QupZilla: Creating new profile directory" << std::endl;

    if (!dir.exists()) {
        dir.mkpath(dir.absolutePath());
    }

    dir.mkdir(QLatin1String("profiles"));
    dir.cd(QLatin1String("profiles"));

    // $Config/profiles
    QFile(dir.filePath(QLatin1String("profiles/profiles.ini"))).remove();
    QFile(QLatin1String(":data/profiles.ini")).copy(dir.filePath(QLatin1String("profiles/profiles.ini")));
    QFile(dir.filePath(QLatin1String("profiles/profiles.ini"))).setPermissions(QFile::ReadUser | QFile::WriteUser);

    dir.mkdir(QLatin1String("default"));
    dir.cd(QLatin1String("default"));

    // $Config/profiles/default
    QFile(dir.filePath(QLatin1String("profiles/default/browsedata.db"))).remove();
    QFile(QLatin1String(":data/browsedata.db")).copy(dir.filePath(QLatin1String("profiles/default/browsedata.db")));
    QFile(dir.filePath(QLatin1String("profiles/default/browsedata.db"))).setPermissions(QFile::ReadUser | QFile::WriteUser);

    QFile(QLatin1String(":data/bookmarks.json")).copy(dir.filePath(QLatin1String("profiles/default/bookmarks.json")));
    QFile(dir.filePath(QLatin1String("profiles/default/bookmarks.json"))).setPermissions(QFile::ReadUser | QFile::WriteUser);

    QFile versionFile(dir.filePath(QLatin1String("profiles/default/version")));
    versionFile.open(QFile::WriteOnly);
    versionFile.write(Qz::VERSION);
    versionFile.close();
}

void ProfileManager::initCurrentProfile(const QString &profileName)
{
    QString profilePath = DataPaths::path(DataPaths::Profiles) + QLatin1Char('/');

    if (profileName.isEmpty()) {
        profilePath.append(startingProfile());
    }
    else {
        profilePath.append(profileName);
    }

    DataPaths::setCurrentProfilePath(profilePath);

    updateCurrentProfile();
    connectDatabase();
}

int ProfileManager::createProfile(const QString &profileName)
{
    QDir dir(DataPaths::path(DataPaths::Profiles));

    if (QDir(dir.absolutePath() + QLatin1Char('/') + profileName).exists()) {
        return -1;
    }
    if (!dir.mkdir(profileName)) {
        return -2;
    }

    dir.mkdir(profileName);
    dir.cd(profileName);
    QFile(QLatin1String(":data/browsedata.db")).copy(dir.filePath(QLatin1String("/browsedata.db")));
    QFile(dir.filePath(QLatin1String("/browsedata.db"))).setPermissions(QFile::ReadUser | QFile::WriteUser);

    QFile versionFile(dir.filePath(QLatin1String("/version")));
    versionFile.open(QFile::WriteOnly);
    versionFile.write(Qz::VERSION);
    versionFile.close();

    return 0;
}

bool ProfileManager::removeProfile(const QString &profileName)
{
    QDir dir(DataPaths::path(DataPaths::Profiles) + QLatin1Char('/') + profileName);

    if (!dir.exists()) {
        return false;
    }

    QzTools::removeDir(dir.absolutePath());
    return true;
}

QString ProfileManager::currentProfile() const
{
    QString path = DataPaths::currentProfilePath();
    return path.mid(path.lastIndexOf(QLatin1Char('/')) + 1);
}

QString ProfileManager::startingProfile() const
{
    QSettings settings(DataPaths::path(DataPaths::Profiles) + QLatin1String("/profiles.ini"), QSettings::IniFormat);
    return settings.value(QLatin1String("Profiles/startProfile"), QLatin1String("default")).toString();
}

void ProfileManager::setStartingProfile(const QString &profileName)
{
    QSettings settings(DataPaths::path(DataPaths::Profiles) + QLatin1String("/profiles.ini"), QSettings::IniFormat);
    settings.setValue(QLatin1String("Profiles/startProfile"), profileName);
}

QStringList ProfileManager::availableProfiles() const
{
    QDir dir(DataPaths::path(DataPaths::Profiles));
    return dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}

void ProfileManager::updateCurrentProfile()
{
    QDir profileDir(DataPaths::currentProfilePath());

    if (!profileDir.exists()) {
        QDir newDir(profileDir.path().remove(profileDir.dirName()));
        newDir.mkdir(profileDir.dirName());
    }

    QFile versionFile(profileDir.filePath(QLatin1String("version")));

    // If file exists, just update the profile to current version
    if (versionFile.exists()) {
        versionFile.open(QFile::ReadOnly);
        QString profileVersion = versionFile.readAll();
        versionFile.close();

        updateProfile(Qz::VERSION, profileVersion.trimmed());
    }
    else {
        copyDataToProfile();
    }

    versionFile.open(QFile::WriteOnly);
    versionFile.write(Qz::VERSION);
    versionFile.close();
}

void ProfileManager::updateProfile(const QString &current, const QString &profile)
{
    if (current == profile) {
        return;
    }

    Updater::Version prof(profile);

    if (prof == Updater::Version("1.0.0")) {
        update100();
        return;
    }

    if (prof == Updater::Version("1.1.0") || prof == Updater::Version("1.1.5") || prof == Updater::Version("1.1.8")) {
        update118();
        return;
    }

    if (prof == Updater::Version("1.2.0")) {
        update120();
        return;
    }

    if (prof == Updater::Version("1.3.0") || prof == Updater::Version("1.3.1")) {
        update130();
        return;
    }

    if (prof >= Updater::Version("1.4.0") && prof <= Updater::Version("1.5.0")) {
        update140();
        return;
    }

    if (prof >= Updater::Version("1.6.0") && prof < Updater::Version("1.8.0")) {
        update160();
        return;
    }

    // 1.8.x bugfix releases
    if (prof >= Updater::Version("1.8.0") && prof < Updater::Version("1.9.0")) {
        return;
    }

    std::cout << "QupZilla: Incompatible profile version detected (" << qPrintable(profile) << "), overwriting profile data..." << std::endl;

    copyDataToProfile();
}

void ProfileManager::copyDataToProfile()
{
    QDir profileDir(DataPaths::currentProfilePath());

    QFile browseData(profileDir.filePath(QLatin1String("browsedata.db")));

    if (browseData.exists()) {
        const QString browseDataBackup = QzTools::ensureUniqueFilename(profileDir.filePath(QLatin1String("browsedata-backup.db")));
        browseData.copy(browseDataBackup);

        const QString text = "Incompatible profile version has been detected. To avoid losing your profile data, they were "
                             "backed up in following file:<br/><br/><b>" + browseDataBackup + "<br/></b>";
        QMessageBox::warning(0, "QupZilla: Incompatible profile version", text);
    }

    browseData.remove();

    QFile(QLatin1String(":data/browsedata.db")).copy(profileDir.filePath(QLatin1String("browsedata.db")));
    QFile(profileDir.filePath(QLatin1String("browsedata.db"))).setPermissions(QFile::ReadUser | QFile::WriteUser);

    // Reconnect database
    connectDatabase();
}

void ProfileManager::connectDatabase()
{
    const QString dbFile = DataPaths::currentProfilePath() + QLatin1String("/browsedata.db");

    // Reconnect
    if (m_databaseConnected) {
        QSqlDatabase::removeDatabase(QSqlDatabase::database().connectionName());
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"));
    db.setDatabaseName(dbFile);

    if (!QFile::exists(dbFile)) {
        qWarning("Cannot find SQLite database file! Copying and using the defaults!");

        QFile(":data/browsedata.db").copy(dbFile);
        QFile(dbFile).setPermissions(QFile::ReadUser | QFile::WriteUser);
        db.setDatabaseName(dbFile);
    }

    if (mApp->isPrivate()) {
        db.setConnectOptions("QSQLITE_OPEN_READONLY");
    }

    if (!db.open()) {
        qWarning("Cannot open SQLite database! Continuing without database....");
    }

    m_databaseConnected = true;
}

void ProfileManager::update100()
{
    std::cout << "QupZilla: Upgrading profile version from 1.0.0..." << std::endl;

    connectDatabase();

    QSqlQuery query;
    query.exec("ALTER TABLE autofill ADD COLUMN last_used NUMERIC");
    query.exec("UPDATE autofill SET last_used=0");

    update118();
}

void ProfileManager::update118()
{
    std::cout << "QupZilla: Upgrading profile version from 1.1.8..." << std::endl;

    connectDatabase();

    QSqlQuery query;
    query.exec("ALTER TABLE folders ADD COLUMN parent TEXT");

    update120();
}

void ProfileManager::update120()
{
    std::cout << "QupZilla: Upgrading profile version from 1.2.0..." << std::endl;

    connectDatabase();

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

void ProfileManager::update130()
{
    std::cout << "QupZilla: Upgrading profile version from 1.3.0..." << std::endl;

    connectDatabase();

    QSqlQuery query;
    query.exec("ALTER TABLE bookmarks ADD COLUMN keyword TEXT");

    update140();
}

void ProfileManager::update140()
{
    std::cout << "QupZilla: Upgrading profile version from 1.4.0..." << std::endl;

    connectDatabase();

    QSqlQuery query;
    query.exec("ALTER TABLE search_engines ADD COLUMN postData TEXT");
}

void ProfileManager::update160()
{
    // Nothing to upgrade
}
