/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef QZCOMMON_H
#define QZCOMMON_H

#include <QDebug>
#include <QFlags>
#include <QString>

#ifdef QUPZILLA_SHAREDLIBRARY
#define QUPZILLA_EXPORT Q_DECL_EXPORT
#else
#define QUPZILLA_EXPORT Q_DECL_IMPORT
#endif

#ifndef Q_UNLIKELY
#define Q_UNLIKELY(x) x
#endif

#ifndef Q_LIKELY
#define Q_LIKELY(x) x
#endif

#ifndef QSL
#if QT_VERSION >= 0x050000
#define QSL(x) QStringLiteral(x)
#else
#define QSL(x) QLatin1String(x)
#endif
#endif

#ifndef QL1S
#define QL1S(x) QLatin1String(x)
#endif

#ifndef QL1C
#define QL1C(x) QLatin1Char(x)
#endif

namespace Qz
{
// Version of session.dat file
static const int sessionVersion = 0x0003;
// Backwards compatibility (used to be different for Qt4 and Qt5)
static const int sessionVersionQt5 = 0x0003 | 0x050000;

// Version of bookmarks.json file
static const int bookmarksVersion = 1;

static const QString APPNAME = "QupZilla";
static const QString VERSION = "1.7.0";
static const QString BUILDTIME =  __DATE__" "__TIME__;
static const QString AUTHOR = "David Rosca";
static const QString COPYRIGHT = "2010-2014";
static const QString WWWADDRESS = "http://www.qupzilla.com";
static const QString WIKIADDRESS = "https://github.com/QupZilla/qupzilla/wiki";

enum BrowserWindowType {
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
    CL_ToggleFullScreen,
    CL_StartPrivateBrowsing,
    CL_StartNewInstance,
    CL_StartPortable,
    CL_ExitAction
};

enum ObjectName {
    ON_WebView,
    ON_TabBar,
    ON_BrowserWindow
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

#define ADBLOCK_EASYLIST_URL "https://easylist-downloads.adblockplus.org/easylist.txt"

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
#define DEFAULT_THEME_NAME "windows"
#elif defined(Q_OS_MAC)
#define DEFAULT_THEME_NAME "mac"
#elif defined(Q_OS_UNIX)
#define DEFAULT_THEME_NAME "linux"
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

#define QTWEBKIT_FROM_2_2 \
    (QT_VERSION >= 0x050000 || (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0)))

#define QTWEBKIT_TO_2_2 \
    (QT_VERSION < 0x050000 && (QTWEBKIT_VERSION < QTWEBKIT_VERSION_CHECK(2, 2, 0)))

#define QTWEBKIT_FROM_2_3 \
    (QT_VERSION >= 0x050000 || (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 3, 0)))

#define QTWEBKIT_TO_2_3 \
    (QT_VERSION < 0x050000 && (QTWEBKIT_VERSION < QTWEBKIT_VERSION_CHECK(2, 3, 0)))

#endif // QZCOMMON_H
