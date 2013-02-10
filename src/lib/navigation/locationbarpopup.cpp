/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  Franz Fellner <alpine.art.de@googlemail.com>
*                          David Rosca <nowrep@gmail.com>
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
#include "locationbarpopup.h"

#include <QLayout>

LocationBarPopup::LocationBarPopup(QWidget* parent)
    : QFrame(parent, Qt::Popup)
    , m_alignment(Qt::AlignRight)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setLineWidth(1);
    setMidLineWidth(2);
}

void LocationBarPopup::showAt(QWidget* parent)
{
    if (!parent) {
        return;
    }

    // Calculate sizes before showing
    layout()->invalidate();
    layout()->activate();

    QPoint p = parent->mapToGlobal(QPoint(0, 0));

    if (m_alignment == Qt::AlignRight) {
        p.setX(p.x() + parent->width() - width());
    }

    p.setY(p.y() + parent->height());
    move(p);

    QFrame::show();
}

void LocationBarPopup::setPopupAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
}

Qt::Alignment LocationBarPopup::popupAlignment() const
{
    return m_alignment;
}
