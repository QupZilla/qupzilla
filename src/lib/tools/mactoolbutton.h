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
#ifndef MACTOOLBUTTON_H
#define MACTOOLBUTTON_H

#include "qzcommon.h"

#ifdef Q_OS_MACOS
#include <QPushButton>

class QUPZILLA_EXPORT MacToolButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool autoRaise READ autoRaise WRITE setAutoRaise)

public:
    explicit MacToolButton(QWidget* parent = 0);

    void setIconSize(const QSize &size);

    void setAutoRaise(bool enable);
    bool autoRaise() const;

private:
    bool m_autoRise;
    QSize m_buttonFixedSize;
};
#else
#include <QToolButton>

class QUPZILLA_EXPORT MacToolButton : public QToolButton
{
    Q_OBJECT

public:
    explicit MacToolButton(QWidget* parent = 0);
};
#endif
#endif // MACTOOLBUTTON_H
