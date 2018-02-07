/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2014 Franz Fellner <alpine.art.de@googlemail.com>
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
#include "restoremanager.h"
#include "recoveryjsobject.h"
#include "datapaths.h"

#include <QFile>

static const int restoreDataVersion = 2;

bool RestoreData::isValid() const
{
    for (const BrowserWindow::SavedWindow &window : qAsConst(windows)) {
        if (!window.isValid()) {
            return false;
        }
    }
    return !windows.isEmpty();
}

void RestoreData::clear()
{
    windows.clear();
    crashedSession.clear();
    closedWindows.clear();
}

QDataStream &operator<<(QDataStream &stream, const RestoreData &data)
{
    stream << data.windows.count();
    for (const BrowserWindow::SavedWindow &window : qAsConst(data.windows)) {
        stream << window;
    }

    stream << restoreDataVersion;
    stream << data.crashedSession;
    stream << data.closedWindows;

    return stream;
}

QDataStream &operator>>(QDataStream &stream, RestoreData &data)
{
    int windowCount;
    stream >> windowCount;
    data.windows.reserve(windowCount);

    for (int i = 0; i < windowCount; ++i) {
        BrowserWindow::SavedWindow window;
        stream >> window;
        data.windows.append(window);
    }

    int version;
    stream >> version;

    if (version >= 1) {
        stream >> data.crashedSession;
    }

    if (version >= 2) {
        stream >> data.closedWindows;
    }

    return stream;
}

RestoreManager::RestoreManager(const QString &file)
    : m_recoveryObject(new RecoveryJsObject(this))
{
    createFromFile(file);
}

RestoreManager::~RestoreManager()
{
    delete m_recoveryObject;
}

RestoreData RestoreManager::restoreData() const
{
    return m_data;
}

void RestoreManager::clearRestoreData()
{
    m_data.clear();

    QDataStream stream(&m_data.crashedSession, QIODevice::ReadOnly);
    stream >> m_data;
}

bool RestoreManager::isValid() const
{
    return m_data.isValid();
}

QObject *RestoreManager::recoveryObject(WebPage *page)
{
    m_recoveryObject->setPage(page);
    return m_recoveryObject;
}

static void loadCurrentVersion(QDataStream &stream, RestoreData &data)
{
    stream >> data;
}

static void loadVersion3(QDataStream &stream, RestoreData &data)
{
    int windowCount;
    stream >> windowCount;
    data.windows.reserve(windowCount);

    for (int i = 0; i < windowCount; ++i) {
        QByteArray tabsState;
        QByteArray windowState;

        stream >> tabsState;
        stream >> windowState;

        BrowserWindow::SavedWindow window;

#ifdef QZ_WS_X11
        QDataStream stream1(&windowState, QIODevice::ReadOnly);
        stream1 >> window.windowState;
        stream1 >> window.virtualDesktop;
#else
        window.windowState = windowState;
#endif

        int tabsCount = -1;
        QDataStream stream2(&tabsState, QIODevice::ReadOnly);
        stream2 >> tabsCount;
        window.tabs.reserve(tabsCount);
        for (int i = 0; i < tabsCount; ++i) {
            WebTab::SavedTab tab;
            stream2 >> tab;
            window.tabs.append(tab);
        }
        stream2 >> window.currentTab;

        data.windows.append(window);
    }
}

// static
bool RestoreManager::validateFile(const QString &file)
{
    RestoreData data;
    createFromFile(file, data);
    return data.isValid();
}

// static
void RestoreManager::createFromFile(const QString &file, RestoreData &data)
{
    if (!QFile::exists(file)) {
        return;
    }

    QFile recoveryFile(file);
    if (!recoveryFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QDataStream stream(&recoveryFile);

    int version;
    stream >> version;

    if (version == Qz::sessionVersion) {
        loadCurrentVersion(stream, data);
    } else if (version == 0x0003 || version == (0x0003 | 0x050000)) {
        loadVersion3(stream, data);
    } else {
        qWarning() << "Unsupported session file version" << version;
    }
}

void RestoreManager::createFromFile(const QString &file)
{
    createFromFile(file, m_data);
}
