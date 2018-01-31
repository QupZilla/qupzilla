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
#include "profilemanager.h"
#include "mainapplication.h"
#include "datapaths.h"
#include "updater.h"
#include "qztools.h"
#include "sqldatabase.h"

#include <QDir>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSettings>
#include <iostream>

ProfileManager::ProfileManager()
    : m_databaseConnected(false)
{
}

void ProfileManager::initConfigDir()
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
    QFile(dir.filePath(QLatin1String("profiles.ini"))).remove();
    QFile(QLatin1String(":data/profiles.ini")).copy(dir.filePath(QLatin1String("profiles.ini")));
    QFile(dir.filePath(QLatin1String("profiles.ini"))).setPermissions(QFile::ReadUser | QFile::WriteUser);

    dir.mkdir(QLatin1String("default"));
    dir.cd(QLatin1String("default"));

    // $Config/profiles/default
    QFile(dir.filePath(QLatin1String("browsedata.db"))).remove();
    QFile(QLatin1String(":data/browsedata.db")).copy(dir.filePath(QLatin1String("browsedata.db")));
    QFile(dir.filePath(QLatin1String("browsedata.db"))).setPermissions(QFile::ReadUser | QFile::WriteUser);

    QFile(QLatin1String(":data/bookmarks.json")).copy(dir.filePath(QLatin1String("bookmarks.json")));
    QFile(dir.filePath(QLatin1String("bookmarks.json"))).setPermissions(QFile::ReadUser | QFile::WriteUser);

    QFile versionFile(dir.filePath(QLatin1String("version")));
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

    dir.cd(profileName);
    QFile(QLatin1String(":data/browsedata.db")).copy(dir.filePath(QLatin1String("browsedata.db")));
    QFile(dir.filePath(QLatin1String("browsedata.db"))).setPermissions(QFile::ReadUser | QFile::WriteUser);

    QFile versionFile(dir.filePath(QLatin1String("version")));
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

// static
QString ProfileManager::currentProfile()
{
    QString path = DataPaths::currentProfilePath();
    return path.mid(path.lastIndexOf(QLatin1Char('/')) + 1);
}

// static
QString ProfileManager::startingProfile()
{
    QSettings settings(DataPaths::path(DataPaths::Profiles) + QLatin1String("/profiles.ini"), QSettings::IniFormat);
    return settings.value(QLatin1String("Profiles/startProfile"), QLatin1String("default")).toString();
}

// static
void ProfileManager::setStartingProfile(const QString &profileName)
{
    QSettings settings(DataPaths::path(DataPaths::Profiles) + QLatin1String("/profiles.ini"), QSettings::IniFormat);
    settings.setValue(QLatin1String("Profiles/startProfile"), profileName);
}

// static
QStringList ProfileManager::availableProfiles()
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

    if (prof > Updater::Version(Qz::VERSION)) {
        copyDataToProfile();
        return;
    }

    if (prof < Updater::Version("1.9.0")) {
        std::cout << "QupZilla: Using profile from QupZilla " << qPrintable(profile) << " is not supported!" << std::endl;
        return;
    }

    // No change in 2.0
    if (prof < Updater::Version("2.0.99")) {
        return;
    }

    // No change in 2.1
    if (prof < Updater::Version("2.1.99")) {
        return;
    }

    // No change in 2.2
    if (prof < Updater::Version("2.2.99")) {
        return;
    }

    // Nothing for now
}

void ProfileManager::copyDataToProfile()
{
    QDir profileDir(DataPaths::currentProfilePath());

    QFile browseData(profileDir.filePath(QLatin1String("browsedata.db")));

    if (browseData.exists()) {
        const QString browseDataBackup = QzTools::ensureUniqueFilename(profileDir.filePath(QLatin1String("browsedata-backup.db")));
        browseData.copy(browseDataBackup);
        browseData.remove();

        QFile settings(profileDir.filePath(QSL("settings.ini")));
        if (settings.exists()) {
            const QString settingsBackup = QzTools::ensureUniqueFilename(profileDir.filePath(QSL("settings-backup.ini")));
            settings.copy(settingsBackup);
            settings.remove();
        }

        QFile sessionFile(profileDir.filePath(QSL("session.dat")));
        if (sessionFile.exists()) {
            QString oldVersion = QzTools::readAllFileContents(profileDir.filePath(QSL("version"))).trimmed();
            if (oldVersion.isEmpty()) {
                oldVersion = QSL("unknown-version");
            }
            const QString sessionBackup = QzTools::ensureUniqueFilename(profileDir.filePath(QSL("sessions/backup-%1.dat").arg(oldVersion)));
            sessionFile.copy(sessionBackup);
            sessionFile.remove();
        }

        const QString text = "Incompatible profile version has been detected. To avoid losing your profile data, they were "
                             "backed up in following file:<br/><br/><b>" + browseDataBackup + "<br/></b>";
        QMessageBox::warning(0, "QupZilla: Incompatible profile version", text);
    }

    QFile(QLatin1String(":data/browsedata.db")).copy(profileDir.filePath(QLatin1String("browsedata.db")));
    QFile(profileDir.filePath(QLatin1String("browsedata.db"))).setPermissions(QFile::ReadUser | QFile::WriteUser);
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

    SqlDatabase::instance()->setDatabase(db);

    m_databaseConnected = true;
}
