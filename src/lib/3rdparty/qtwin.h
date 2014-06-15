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
/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#ifndef QTWIN_H
#define QTWIN_H

#include "qzcommon.h"

#include <QColor>
#include <QWidget>
#include <QSysInfo>
/**
  * This is a helper class for using the Desktop Window Manager
  * functionality on Windows 7 and Windows Vista. On other platforms
  * these functions will simply not do anything.
  */
#ifdef Q_OS_WIN
// Qt5 compile issue: http://comments.gmane.org/gmane.comp.lib.qt.user/4711
#ifndef __MINGW32__
#define NOMINMAX
#endif
#ifdef W7API
#include <ShlObj.h>
#include <shlwapi.h>
#include <Propvarutil.h>
#include "msvc2008.h"

DEFINE_PROPERTYKEY(PKEY_Title, 0xF29F85E0, 0x4FF9, 0x1068, 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9, 2);
DEFINE_PROPERTYKEY(PKEY_AppUserModel_IsDestListSeparator, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 6);
#endif
#endif
class WindowNotifier;
class QUPZILLA_EXPORT QtWin : public QObject
{
    Q_OBJECT
public:
    static bool isRunningWindows7();
    static bool enableBlurBehindWindow(QWidget* widget, bool enable = true);
    static bool extendFrameIntoClientArea(QWidget* widget,
                                          int left = -1, int top = -1,
                                          int right = -1, int bottom = -1);
    static bool isCompositionEnabled();
    static QColor colorizationColor();

    static void createJumpList();
#ifdef Q_OS_WIN
    static HWND hwndOfWidget(const QWidget* widget);
#endif

private:
    static WindowNotifier* windowNotifier();
#ifdef W7API
    static void populateFrequentSites(IObjectCollection* collection, const QString &appPath);
    static void AddTasksToList(ICustomDestinationList* destinationList);
    static IShellLink* CreateShellLink(const QString &title, const QString &description, const QString &app_path, const QString &app_args, const QString &icon_path, int app_index);
#endif
};

#endif // QTWIN_H
