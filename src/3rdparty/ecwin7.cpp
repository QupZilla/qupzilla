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

#include "ecwin7.h"

// Windows only definitions
#ifdef W7API
DEFINE_GUID(CLSID_TaskbarList,0x56fdf344,0xfd6d,0x11d0,0x95,0x8a,0x0,0x60,0x97,0xc9,0xa0,0x90);
DEFINE_GUID(IID_ITaskbarList3,0xea1afb91,0x9e28,0x4b86,0x90,0xE9,0x9e,0x9f,0x8a,0x5e,0xef,0xaf);

// Constructor: variabiles initialization
EcWin7::EcWin7()
{
    mOverlayIcon = NULL;
}

// Init taskbar communication
void EcWin7::init(WId wid)
{
    mWindowId = wid;
    mTaskbarMessageId = RegisterWindowMessage(L"TaskbarButtonCreated");
}

// Windows event handler callback function
// (handles taskbar communication initial message)
bool EcWin7::winEvent(MSG * message, long * result)
{
    if (message->message == mTaskbarMessageId)
    {
        HRESULT hr = CoCreateInstance(CLSID_TaskbarList,
                                      0,
                                      CLSCTX_INPROC_SERVER,
                                      IID_ITaskbarList3,
                                      reinterpret_cast<void**> (&(mTaskbar)));
        *result = hr;
        return true;
    }
    return false;
}

// Set progress bar current value
void EcWin7::setProgressValue(int value, int max)
{
    mTaskbar->SetProgressValue(mWindowId, value, max);

}

// Set progress bar current state (active, error, pause, ecc...)
void EcWin7::setProgressState(ToolBarProgressState state)
{
    mTaskbar->SetProgressState(mWindowId, (TBPFLAG)state);
}

// Set new overlay icon and corresponding description (for accessibility)
// (call with iconName == "" and description == "" to remove any previous overlay icon)
void EcWin7::setOverlayIcon(QString iconName, QString description)
{
    HICON oldIcon = NULL;
    if (mOverlayIcon != NULL) oldIcon = mOverlayIcon;
    if (iconName == "")
    {
        mTaskbar->SetOverlayIcon(mWindowId, NULL, NULL);
        mOverlayIcon = NULL;
    }
    else
    {
        mOverlayIcon = (HICON) LoadImage(GetModuleHandle(NULL),
                                 iconName.toStdWString().c_str(),
                                 IMAGE_ICON,
                                 0,
                                 0,
                                 NULL);
        mTaskbar->SetOverlayIcon(mWindowId, mOverlayIcon, description.toStdWString().c_str());
    }
    if ((oldIcon != NULL) && (oldIcon != mOverlayIcon))
    {
        DestroyIcon(oldIcon);
    }
}
#endif
