/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "navigationbartoolbutton.h"
#include "abstractbuttoninterface.h"

#include <QApplication>

NavigationBarToolButton::NavigationBarToolButton(AbstractButtonInterface *button, QWidget *parent)
    : ToolButton(parent)
    , m_button(button)
{
    setAutoRaise(true);
    setToolbarButtonLook(true);
    setFocusPolicy(Qt::NoFocus);

    setToolTip(button->toolTip());
    setIcon(button->icon());

    connect(button, &AbstractButtonInterface::iconChanged, this, &ToolButton::setIcon);
    connect(button, &AbstractButtonInterface::toolTipChanged, this, &ToolButton::setToolTip);
    connect(this, &ToolButton::clicked, this, &NavigationBarToolButton::clicked);
}

void NavigationBarToolButton::clicked()
{
    AbstractButtonInterface::ClickController c;
    c.visualParent = this;
    c.popupPosition = [this](const QSize &size) {
        QPoint pos = mapToGlobal(rect().bottomRight());
        if (QApplication::isRightToLeft()) {
            pos.setX(pos.x() - rect().width());
        } else {
            pos.setX(pos.x() - size.width());
        }
        return pos;
    };
    setDown(true);
    emit m_button->clicked(&c);
    setDown(false);
}
