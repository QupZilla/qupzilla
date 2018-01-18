/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#ifndef TESTPLUGIN_SIDEBAR_H
#define TESTPLUGIN_SIDEBAR_H

#include "sidebarinterface.h"

class TestPlugin_Sidebar : public SideBarInterface
{
    Q_OBJECT
public:
    explicit TestPlugin_Sidebar(QObject* parent = 0);

    QString title() const;
    QAction* createMenuAction();

    QWidget* createSideBarWidget(BrowserWindow* mainWindow);
};

#endif // TESTPLUGIN_SIDEBAR_H
