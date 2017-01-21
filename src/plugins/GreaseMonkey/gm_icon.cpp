/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2013-2017 David Rosca <nowrep@gmail.com>
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
#include "gm_icon.h"
#include "gm_manager.h"
#include "browserwindow.h"

GM_Icon::GM_Icon(GM_Manager* manager, BrowserWindow* window)
    : ClickableLabel(window)
    , m_manager(manager)
    , m_window(window)
{
    setCursor(Qt::PointingHandCursor);
    setPixmap(QIcon(":gm/data/icon.svg").pixmap(16));
    setToolTip(tr("Open GreaseMonkey settings"));

    connect(this, SIGNAL(clicked(QPoint)), this, SLOT(openSettings()));
}

void GM_Icon::openSettings()
{
    m_manager->showSettings(m_window);
}
