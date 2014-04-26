/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#include "datapaths.h"
#include "qztools.h"

#include <QApplication>
#include <QDir>

#if QT_VERSION < 0x050000
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif

Q_GLOBAL_STATIC(DataPaths, qz_data_paths)

DataPaths::DataPaths()
{
    init();
}

// static
void DataPaths::setCurrentProfilePath(const QString &profilePath)
{
    qz_data_paths()->initCurrentProfile(profilePath);
}

// static
void DataPaths::setPortableVersion()
{
    DataPaths* d = qz_data_paths();
    d->m_paths[Config] = d->m_paths[AppData];

    d->m_paths[Profiles] = d->m_paths[Config];
    d->m_paths[Profiles].first().append(QLatin1String("/profiles"));

    d->m_paths[Temp] = d->m_paths[Config];
    d->m_paths[Temp].first().append(QLatin1String("/tmp"));
}

// static
QString DataPaths::path(DataPaths::Path path)
{
    Q_ASSERT(!qz_data_paths()->m_paths[path].isEmpty());

    return qz_data_paths()->m_paths[path].first();
}

// static
QStringList DataPaths::allPaths(DataPaths::Path type)
{
    Q_ASSERT(!qz_data_paths()->m_paths[type].isEmpty());

    return qz_data_paths()->m_paths[type];
}

// static
QString DataPaths::currentProfilePath()
{
    return path(CurrentProfile);
}

// static
void DataPaths::clearTempData()
{
    QzTools::removeDir(path(Temp));
}

void DataPaths::init()
{
    m_paths.reserve(5);

    // AppData
#if defined(Q_OS_MAC)
    m_paths[AppData].append(QApplication::applicationDirPath() + QLatin1String("/../Resources"));
#elif defined(Q_OS_UNIX) && !defined(NO_SYSTEM_DATAPATH)
    m_paths[AppData].append(USE_DATADIR);
#else
    m_paths[AppData].append(QApplication::applicationDirPath());
#endif

    m_paths[Translations].append(m_paths[AppData].first() + QLatin1String("/locale"));
    m_paths[Themes].append(m_paths[AppData].first() + QLatin1String("/themes"));
    m_paths[Plugins].append(m_paths[AppData].first() + QLatin1String("/plugins"));

    // Config
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
    // Use %LOCALAPPDATA%/qupzilla as Config path on Windows
#if QT_VERSION < 0x050000
    QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
    // Backwards compatibility
    if (dataLocation.isEmpty()) {
        dataLocation = QDir::homePath() + QLatin1String("/.config/qupzilla");
    }

    QDir confPath = QDir(dataLocation);
    QDir oldConfPath = QDir(QDir::homePath() + QLatin1String("/.qupzilla"));

    if (!oldConfPath.exists()) {
        oldConfPath = QDir::homePath() + QLatin1String("/.config/qupzilla");
    }
#elif defined(Q_OS_MAC)
    QDir confPath = QDir(QDir::homePath() + QLatin1String("/Library/Application Support/QupZilla"));
    QDir oldConfPath = QDir(QDir::homePath() + QLatin1String("/.config/qupzilla"));
#else // Unix
    QDir confPath = QDir(QDir::homePath() + QLatin1String("/.config/qupzilla"));
    QDir oldConfPath = QDir(QDir::homePath() + QLatin1String("/.qupzilla"));
#endif

    if (oldConfPath.exists() && !confPath.exists()) {
        m_paths[Config].append(oldConfPath.absolutePath());

        qWarning() << "WARNING: Using deprecated configuration path" << oldConfPath.absolutePath();
        qWarning() << "WARNING: This path may not be supported in future versions!";
        qWarning() << "WARNING: Please move your configuration into" << confPath.absolutePath();
    }
    else {
        m_paths[Config].append(confPath.absolutePath());
    }

    // Make sure the Config path exists
    QDir dir;
    dir.mkpath(m_paths[Config].first());

    // Profiles
    m_paths[Profiles].append(m_paths[Config].first() + QLatin1String("/profiles"));

    // Temp
#ifdef Q_OS_UNIX
    dir.mkpath(QDir::tempPath() + QLatin1String("/qupzilla/tmp"));
    m_paths[Temp].append(QDir::tempPath() + QLatin1String("/qupzilla/tmp"));
#else
    m_paths[Temp].append(m_paths[Config].first() + QLatin1String("/tmp"));
#endif

    // We also allow to load data from Config path
    m_paths[Translations].append(m_paths[Config].first() + QLatin1String("/locale"));
    m_paths[Themes].append(m_paths[Config].first() + QLatin1String("/themes"));
    m_paths[Plugins].append(m_paths[Config].first() + QLatin1String("/plugins"));

#ifdef USE_LIBPATH
    m_paths[Plugins].append(QLatin1String(USE_LIBPATH "/qupzilla"));
#endif
}

void DataPaths::initCurrentProfile(const QString &profilePath)
{
    m_paths[CurrentProfile].append(profilePath);
}
