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
#pragma once

#include <QObject>
#include <QVector>

#include "qzcommon.h"
#include "browserwindow.h"

class QUPZILLA_EXPORT ClosedWindowsManager : public QObject
{
    Q_OBJECT

public:
    struct Window {
        QIcon icon;
        QString title;
        BrowserWindow::SavedWindow windowState;

        bool isValid() const {
            return windowState.isValid();
        }
    };

    explicit ClosedWindowsManager(QObject *parent = nullptr);

    bool isClosedWindowAvailable() const;
    QVector<Window> closedWindows() const;

    void saveWindow(BrowserWindow *window);

    // Takes window that was most recently closed
    Window takeLastClosedWindow();
    // Takes window at given index
    Window takeClosedWindowAt(int index);

    QByteArray saveState() const;
    void restoreState(const QByteArray &state);

public slots:
    void restoreClosedWindow();
    void restoreAllClosedWindows();
    void clearClosedWindows();

private:
    QVector<Window> m_closedWindows;
};

// Hint to Qt to use std::realloc on item moving
Q_DECLARE_TYPEINFO(ClosedWindowsManager::Window, Q_MOVABLE_TYPE);
