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

        updateProfile(QupZilla::VERSION, profileVersion);
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

    if (profileVersion >= Updater::parseVersionFromString("1.0.0-b3"))
        //Data not changed from 1.0.0-b3
        return;

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
