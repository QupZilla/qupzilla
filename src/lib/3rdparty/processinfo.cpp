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
#include "processinfo.h"

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#endif

ProcessInfo::ProcessInfo(const QString &name)
    : m_name(name)
{
}

bool ProcessInfo::isRunning() const
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    pid_t pid = GetPIDbyName(qPrintable(m_name));
    // -1 = process not found
    // -2 = /proc fs access error
    return (pid != -1 && pid != -2);
#else
    return false;
#endif
}

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
bool ProcessInfo::IsNumeric(const char* ccharptr_CharacterList) const
{
    for (; *ccharptr_CharacterList; ccharptr_CharacterList++) {
        if (*ccharptr_CharacterList < '0' || *ccharptr_CharacterList > '9') {
            return false;
        }
    }

    return true;
}

pid_t ProcessInfo::GetPIDbyName(const char* cchrptr_ProcessName) const
{
    char chrarry_CommandLinePath[260]  ;
    char chrarry_NameOfProcess[300]  ;
    char* chrptr_StringToCompare = NULL ;
    pid_t pid_ProcessIdentifier = (pid_t) - 1 ;
    struct dirent* de_DirEntity = NULL ;
    DIR* dir_proc = NULL ;

    dir_proc = opendir("/proc/") ;
    if (dir_proc == NULL) {
        perror("Couldn't open the /proc/ directory") ;
        return (pid_t) - 2 ;
    }

    // Loop while not NULL
    while ((de_DirEntity = readdir(dir_proc))) {
#ifndef __HAIKU__
        if (de_DirEntity->d_type == DT_DIR) {
            if (IsNumeric(de_DirEntity->d_name)) {
                strcpy(chrarry_CommandLinePath, "/proc/") ;
                strcat(chrarry_CommandLinePath, de_DirEntity->d_name) ;
                strcat(chrarry_CommandLinePath, "/cmdline") ;
                FILE* fd_CmdLineFile = fopen(chrarry_CommandLinePath, "rt") ;   // open the file for reading text
                if (fd_CmdLineFile) {
                    int r = fscanf(fd_CmdLineFile, "%20s", chrarry_NameOfProcess) ; // read from /proc/<NR>/cmdline

                    fclose(fd_CmdLineFile);  // close the file prior to exiting the routine

                    if (r < 1) {
                        continue;
                    }

                    if (strrchr(chrarry_NameOfProcess, '/')) {
                        chrptr_StringToCompare = strrchr(chrarry_NameOfProcess, '/') + 1 ;
                    }
                    else {
                        chrptr_StringToCompare = chrarry_NameOfProcess ;
                    }

                    //printf("Process name: %s\n", chrarry_NameOfProcess);
                    //printf("Pure Process name: %s\n", chrptr_StringToCompare );

                    if (!strcmp(chrptr_StringToCompare, cchrptr_ProcessName)) {
                        pid_ProcessIdentifier = (pid_t) atoi(de_DirEntity->d_name) ;
                        closedir(dir_proc) ;
                        return pid_ProcessIdentifier ;
                    }
                }
            }
        }
#endif
    }

    closedir(dir_proc) ;
    return pid_ProcessIdentifier ;
}

#endif
