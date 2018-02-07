/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#ifndef POPUPSTATUSBARMESSAGE_H
#define POPUPSTATUSBARMESSAGE_H

#include "qzcommon.h"
#include "statusbar.h"

class PopupWindow;
class TipLabel;

class QUPZILLA_EXPORT PopupStatusBarMessage
{
public:
    explicit PopupStatusBarMessage(PopupWindow* window);

    void showMessage(const QString &message);
    void clearMessage();

private:
    PopupWindow* m_popupWindow;
    TipLabel* m_statusBarText;
};

#endif // POPUPSTATUSBARMESSAGE_H
