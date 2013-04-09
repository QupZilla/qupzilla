/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#ifndef QZ_NAMESPACE_H
#define QZ_NAMESPACE_H

#ifdef QUPZILLA_SHAREDLIBRARY
#define QT_QUPZILLA_EXPORT Q_DECL_EXPORT
#else
#define QT_QUPZILLA_EXPORT Q_DECL_IMPORT
#endif

#include <QFlags>

namespace Qz
{
// Version of session.dat file
#if QT_VERSION >= 0x050000
static const int sessionVersion = 0x0003 | 0x050000;
#else
static const int sessionVersion = 0x0003;
#endif

enum AppMessageType {
    AM_SetAdBlockIconEnabled,
    AM_CheckPrivateBrowsing,
    AM_ReloadSettings,
    AM_HistoryStateChanged,
    AM_BookmarksChanged
};

enum BrowserWindow {
    BW_FirstAppWindow,
    BW_OtherRestoredWindow,
    BW_NewWindow,
    BW_MacFirstWindow
};

enum CommandLineAction {
    CL_NoAction,
    CL_OpenUrl,
    CL_OpenUrlInCurrentTab,
    CL_OpenUrlInNewWindow,
    CL_StartWithProfile,
    CL_StartWithoutAddons,
    CL_NewTab,
    CL_NewWindow,
    CL_ShowDownloadManager,
    CL_StartPrivateBrowsing,
    CL_StartNewInstance,
    CL_ExitAction
};

enum ObjectName {
    ON_WebView,
    ON_TabBar,
    ON_QupZilla
};

enum NewTabPositionFlag {
    NT_SelectedTab = 1,
    NT_NotSelectedTab = 2,
    NT_CleanTab = 4,
    NT_TabAtTheEnd = 8,
    NT_NewEmptyTab = 16,

    NT_SelectedNewEmptyTab = NT_SelectedTab | NT_TabAtTheEnd | NT_NewEmptyTab,
    NT_SelectedTabAtTheEnd = NT_SelectedTab | NT_TabAtTheEnd,
    NT_NotSelectedTabAtTheEnd = NT_NotSelectedTab | NT_TabAtTheEnd,
    NT_CleanSelectedTabAtTheEnd = NT_SelectedTab | NT_TabAtTheEnd | NT_CleanTab,
    NT_CleanSelectedTab = NT_CleanTab | NT_SelectedTab,
    NT_CleanNotSelectedTab = NT_CleanTab | NT_NotSelectedTab
};

Q_DECLARE_FLAGS(NewTabPositionFlags, NewTabPositionFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qz::NewTabPositionFlags)

}

#ifdef Q_OS_WIN
#define DEFAULT_THEME_NAME "windows"
#elif defined(QZ_WS_X11)
#define DEFAULT_THEME_NAME "linux"
#elif defined(Q_OS_MAC)
#define DEFAULT_THEME_NAME "mac"
#elif defined(Q_OS_OS2)
#define DEFAULT_THEME_NAME "windows"
#else
#define DEFAULT_THEME_NAME "default"
#endif

#ifdef Q_OS_WIN
#define DEFAULT_CHECK_UPDATES true
#else
#define DEFAULT_CHECK_UPDATES false
#endif

#define DEFAULT_CHECK_DEFAULTBROWSER false

#ifdef Q_OS_WIN
#define DEFAULT_DOWNLOAD_USE_NATIVE_DIALOG false
#else
#define DEFAULT_DOWNLOAD_USE_NATIVE_DIALOG true
#endif

#ifdef PORTABLE_BUILD
#define DEFAULT_ENABLE_PLUGINS false
#else
#define DEFAULT_ENABLE_PLUGINS true
#endif

#define QTWEBKIT_FROM_2_2 \
    (QT_VERSION >= 0x050000 || (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0)))

#define QTWEBKIT_TO_2_2 \
    (QT_VERSION < 0x050000 && (QTWEBKIT_VERSION < QTWEBKIT_VERSION_CHECK(2, 2, 0)))

#define QTWEBKIT_FROM_2_3 \
    (QT_VERSION >= 0x050000 || (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 3, 0)))

#define QTWEBKIT_TO_2_3 \
    (QT_VERSION < 0x050000 && (QTWEBKIT_VERSION < QTWEBKIT_VERSION_CHECK(2, 3, 0)))

#endif // QZ_NAMESPACE_H
