/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#ifndef PROCESSINFO_H
#define PROCESSINFO_H

#include "qzcommon.h"

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
#include <sys/types.h>
#endif

/*
 * Code used from http://ubuntuforums.org/showpost.php?p=6593782&postcount=5
 * written by user WitchCraft
 */

class QUPZILLA_EXPORT ProcessInfo
{
public:
    explicit ProcessInfo(const QString &name);

    bool isRunning() const;

private:
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    bool IsNumeric(const char* ccharptr_CharacterList) const;
    pid_t GetPIDbyName(const char* cchrptr_ProcessName) const;
#endif

    QString m_name;
};

#endif // PROCESSINFO_H
