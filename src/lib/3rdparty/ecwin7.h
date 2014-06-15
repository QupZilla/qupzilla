/* EcWin7 - Support library for integrating Windows 7 taskbar features
 * into any Qt application
 * Copyright (C) 2010 Emanuele Colombo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef ECWIN7_H
#define ECWIN7_H

#include "qzcommon.h"

#include <QtGlobal>
#include <QWidget>

// Windows only data definitions
#ifdef W7TASKBAR
#ifndef __MINGW32__
#define NOMINMAX
#endif
#include <windows.h>
#include <initguid.h>
#define CMIC_MASK_ASYNCOK SEE_MASK_ASYNCOK

#include <ShlObj.h>
#include <shlwapi.h>
#include "msvc2008.h"

// ********************************************************************
// EcWin7 class - Windows 7 taskbar handling for Qt and MinGW

class EcWin7
{
public:

    // Initialization methods
    EcWin7();
    void init(HWND wid);
    bool winEvent(MSG* message, long* result);

    // Overlay icon handling
    void setOverlayIcon(QString iconName, QString description);

    // Progress indicator handling
    enum ToolBarProgressState {
        NoProgress = 0,
        Indeterminate = 1,
        Normal = 2,
        Error = 4,
        Paused = 8
    };
    void setProgressValue(int value, int max);
    void setProgressState(ToolBarProgressState state);

private:
    HWND mWindowId;
    UINT mTaskbarMessageId;
    ITaskbarList3* mTaskbar;
    HICON mOverlayIcon;
};
// Windows only data definitions - END
#endif // W7TASKBAR

#endif // ECWIN7_H
