/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2017 David Rosca <nowrep@gmail.com>
* Copyright (C) 2017 S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#import <AppKit/AppKit.h>

// code taken from: https://www.mail-archive.com/interest@qt-project.org/msg23593.html
// Disables auto window tabbing where supported, otherwise a no-op.
void disableWindowTabbing()
{
    if ([NSWindow respondsToSelector:@selector(allowsAutomaticWindowTabbing)]) {
        NSWindow.allowsAutomaticWindowTabbing = NO;
    }
}
