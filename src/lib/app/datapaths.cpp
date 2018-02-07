/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2018 David Rosca <nowrep@gmail.com>
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
#include "mainapplication.h"

#include <QApplication>
#include <QDir>

#include <QStandardPaths>

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

    // Make sure the Config and Temp paths exists
    QDir dir;
    dir.mkpath(d->m_paths[Config].at(0));
    dir.mkpath(d->m_paths[Temp].at(0));
}

// static
QString DataPaths::path(DataPaths::Path path)
{
    Q_ASSERT(!qz_data_paths()->m_paths[path].isEmpty());

    return qz_data_paths()->m_paths[path].at(0);
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
    // AppData
#if defined(Q_OS_MACOS)
    m_paths[AppData].append(QApplication::applicationDirPath() + QLatin1String("/../Resources"));
#elif defined(Q_OS_UNIX) && !defined(NO_SYSTEM_DATAPATH)
    m_paths[AppData].append(USE_DATADIR);
#else
    m_paths[AppData].append(QApplication::applicationDirPath());
#endif

    m_paths[Translations].append(m_paths[AppData].at(0) + QLatin1String("/locale"));
    m_paths[Themes].append(m_paths[AppData].at(0) + QLatin1String("/themes"));
    m_paths[Plugins].append(m_paths[AppData].at(0) + QLatin1String("/plugins"));

    // Config
    if (MainApplication::isTestModeEnabled()) {
        m_paths[Config].append(QDir::tempPath() + QL1S("/QupZilla-test"));
    } else {
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
    // Use %LOCALAPPDATA%/qupzilla as Config path on Windows
    m_paths[Config].append(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
#elif defined(Q_OS_MACOS)
    m_paths[Config].append(QDir::homePath() + QLatin1String("/Library/Application Support/QupZilla"));
#else // Unix
    m_paths[Config].append(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QL1S("/qupzilla"));
#endif
    }

    // Profiles
    m_paths[Profiles].append(m_paths[Config].at(0) + QLatin1String("/profiles"));

    // Temp
#ifdef Q_OS_UNIX
    const QByteArray &user = qgetenv("USER");
    const QString &tempPath = QString(QSL("%1/qupzilla-%2")).arg(QDir::tempPath(), user.constData());
    m_paths[Temp].append(tempPath);
#else
    m_paths[Temp].append(m_paths[Config].at(0) + QLatin1String("/tmp"));
#endif

    // Cache
#ifdef Q_OS_UNIX
    const QString &cachePath = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    if (!cachePath.isEmpty())
        m_paths[Cache].append(cachePath + QLatin1String("/qupzilla"));
#endif

    // Make sure the Config and Temp paths exists
    QDir dir;
    dir.mkpath(m_paths[Config].at(0));
    dir.mkpath(m_paths[Temp].at(0));

    // We also allow to load data from Config path
    m_paths[Translations].append(m_paths[Config].at(0) + QLatin1String("/locale"));
    m_paths[Themes].append(m_paths[Config].at(0) + QLatin1String("/themes"));
    m_paths[Plugins].append(m_paths[Config].at(0) + QLatin1String("/plugins"));

#ifdef USE_LIBPATH
    m_paths[Plugins].append(QLatin1String(USE_LIBPATH "/qupzilla"));
#endif
}

void DataPaths::initCurrentProfile(const QString &profilePath)
{
    m_paths[CurrentProfile].append(profilePath);

    if (m_paths[Cache].isEmpty())
        m_paths[Cache].append(m_paths[CurrentProfile].at(0) + QLatin1String("/cache"));

    if (m_paths[Sessions].isEmpty())
        m_paths[Sessions].append(m_paths[CurrentProfile].at(0) + QLatin1String("/sessions"));

    QDir dir;
    dir.mkpath(m_paths[Cache].at(0));
    dir.mkpath(m_paths[Sessions].at(0));
}
