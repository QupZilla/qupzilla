/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  Franz Fellner <alpine.art.de@googlemail.com>
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
#ifndef LOCATIONBARPOPUP_H
#define LOCATIONBARPOPUP_H

#include <QFrame>

class LocationBarPopup : public QFrame
{
public:
    LocationBarPopup(QWidget* parent);
    void showAt(QWidget* parent);
    void setPopupAlignment(Qt::Alignment alignment) {
        m_alignment = alignment;
    }

    Qt::Alignment popupAlignment() const {
        return m_alignment;
    }

private:
    Qt::Alignment m_alignment;
};

#endif // LOCATIONBARPOPUP_H
