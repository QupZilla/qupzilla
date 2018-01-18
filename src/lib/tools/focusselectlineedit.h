/* ============================================================
* QupZilla - Qt web browser
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
#ifndef FOCUSSELECTLINEEDIT_H
#define FOCUSSELECTLINEEDIT_H

#include <QLineEdit>

#include "qzcommon.h"

class QUPZILLA_EXPORT FocusSelectLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit FocusSelectLineEdit(QWidget* parent = 0);

public slots:
    void setFocus();

protected:
    void focusInEvent(QFocusEvent* event);
    void mousePressEvent(QMouseEvent* event);

    bool m_mouseFocusReason;

};

#endif // FOCUSSELECTLINEEDIT_H
