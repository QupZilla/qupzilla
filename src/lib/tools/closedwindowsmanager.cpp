/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "closedwindowsmanager.h"
#include "mainapplication.h"
#include "tabbedwebview.h"
#include "qztools.h"

#include <QAction>

ClosedWindowsManager::ClosedWindowsManager(QObject *parent)
    : QObject(parent)
{
}

bool ClosedWindowsManager::isClosedWindowAvailable() const
{
    return !m_closedWindows.isEmpty();
}

QVector<ClosedWindowsManager::Window> ClosedWindowsManager::closedWindows() const
{
    return m_closedWindows;
}

void ClosedWindowsManager::saveWindow(BrowserWindow *window)
{
    if (mApp->isPrivate() || mApp->windowCount() == 1 || !window->weView()) {
        return;
    }

    Window closedWindow;
    closedWindow.icon = window->weView()->icon();
    closedWindow.title = window->weView()->title();
    closedWindow.windowState = BrowserWindow::SavedWindow(window);
    m_closedWindows.prepend(closedWindow);
}

ClosedWindowsManager::Window ClosedWindowsManager::takeLastClosedWindow()
{
    Window window;
    if (!m_closedWindows.isEmpty()) {
        window = m_closedWindows.takeFirst();
    }
    return window;
}

ClosedWindowsManager::Window ClosedWindowsManager::takeClosedWindowAt(int index)
{
    Window window;
    if (QzTools::containsIndex(m_closedWindows, index)) {
        window = m_closedWindows.takeAt(index);
    }
    return window;
}

void ClosedWindowsManager::restoreClosedWindow()
{
    Window window;
    QAction *act = qobject_cast<QAction*>(sender());
    if (act) {
        window = takeClosedWindowAt(act->data().toInt());
    } else {
        window = takeLastClosedWindow();
    }

    if (!window.isValid()) {
        return;
    }

    mApp->createWindow(Qz::BW_OtherRestoredWindow)->restoreWindow(window.windowState);
}

void ClosedWindowsManager::restoreAllClosedWindows()
{
    const int count = m_closedWindows.count();
    for (int i = 0; i < count; ++i) {
        restoreClosedWindow();
    }
}

void ClosedWindowsManager::clearClosedWindows()
{
    m_closedWindows.clear();
}

static const int closedWindowsVersion = 1;

QByteArray ClosedWindowsManager::saveState() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << closedWindowsVersion;

    // Only save last 3 windows
    const int windowCount = qBound(0, m_closedWindows.count(), 3);
    stream << windowCount;

    for (int i = 0; i < windowCount; ++i) {
        stream << m_closedWindows.at(i).windowState;
    }

    return data;
}

void ClosedWindowsManager::restoreState(const QByteArray &state)
{
    QDataStream stream(state);

    int version;
    stream >> version;

    if (version < 1) {
        return;
    }

    m_closedWindows.clear();

    int windowCount;
    stream >> windowCount;
    m_closedWindows.reserve(windowCount);

    for (int i = 0; i < windowCount; ++i) {
        Window window;
        stream >> window.windowState;
        if (!window.isValid()) {
            continue;
        }
        window.icon = window.windowState.tabs.at(0).icon;
        window.title = window.windowState.tabs.at(0).title;
        m_closedWindows.append(window);
    }
}
