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
#include "buttonbox.h"

ButtonBox::ButtonBox(QWidget* parent)
    : QDialogButtonBox(parent)
    , m_clickedButton(QDialogButtonBox::RejectRole)
{
    connect(this, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}

void ButtonBox::buttonClicked(QAbstractButton* button)
{
    m_clickedButton = buttonRole(button);
}

QDialogButtonBox::ButtonRole ButtonBox::clickedButtonRole()
{
    return m_clickedButton;
}
