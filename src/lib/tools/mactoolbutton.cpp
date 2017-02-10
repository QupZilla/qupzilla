/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2013-2017 David Rosca <nowrep@gmail.com>
* Copyright (C) 2013-2014 S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#include "mactoolbutton.h"

#ifdef Q_OS_MACOS
MacToolButton::MacToolButton(QWidget* parent)
    : QPushButton(parent)
    , m_autoRise(false)
    , m_buttonFixedSize(18, 18)
{
}

void MacToolButton::setIconSize(const QSize &size)
{
    QPushButton::setIconSize(size);
    m_buttonFixedSize = QSize(size.width() + 2, size.height() + 2);
}

void MacToolButton::setAutoRaise(bool enable)
{
    m_autoRise = enable;
    setFlat(enable);
    if (enable) {
        setFixedSize(m_buttonFixedSize);
    }
}

bool MacToolButton::autoRaise() const
{
    return m_autoRise;
}
#else
MacToolButton::MacToolButton(QWidget* parent)
    : QToolButton(parent)
{
}
#endif
