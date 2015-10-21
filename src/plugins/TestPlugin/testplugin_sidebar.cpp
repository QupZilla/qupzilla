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
#include "testplugin_sidebar.h"

#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

TestPlugin_Sidebar::TestPlugin_Sidebar(QObject* parent)
    : SideBarInterface(parent)
{
}

QString TestPlugin_Sidebar::title() const
{
    return tr("Testing Sidebar");
}

QAction* TestPlugin_Sidebar::createMenuAction()
{
    // The action must be parented to some object from plugin, otherwise
    // there may be a crash when unloading the plugin.

    QAction* act = new QAction(tr("Testing Sidebar"), this);
    act->setCheckable(true);

    return act;
}

QWidget* TestPlugin_Sidebar::createSideBarWidget(BrowserWindow* mainWindow)
{
    Q_UNUSED(mainWindow)

    QWidget* w = new QWidget;
    QPushButton* b = new QPushButton("Example Plugin v0.0.1");
    QLabel* label = new QLabel();
    label->setPixmap(QPixmap(":icons/other/about.png"));

    QVBoxLayout* l = new QVBoxLayout(w);
    l->addWidget(label);
    l->addWidget(b);
    w->setLayout(l);

    return w;
}
