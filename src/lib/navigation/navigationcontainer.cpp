/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#include "navigationcontainer.h"
#include "qzsettings.h"

#include <QPainter>
#include <QStyleOptionFrameV3>

NavigationContainer::NavigationContainer(QWidget* parent)
    : QWidget(parent)
{
}

void NavigationContainer::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    if (qzSettings->tabsOnTop) {
        // Draw line at the bottom of navigation bar if tabs are on top
        // To visually distinguish navigation bar from the page
        QStyleOptionFrameV3 option;
        option.initFrom(this);

        QPainter p(this);
        QRect lineRect(0, height() - 1, width(), 1);
        p.fillRect(lineRect, option.palette.window().color().darker(150));
    }
}
