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
#include "autofillicon.h"

AutoFillIcon::AutoFillIcon(QWidget* parent)
    : ClickableLabel(parent)
{
    setObjectName("locationbar-autofillicon");
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("Choose username to login"));
    setFocusPolicy(Qt::ClickFocus);
}

void AutoFillIcon::setFormData(const QList<AutoFillData> &data)
{
    m_data = data;
}

QList<AutoFillData> AutoFillIcon::formData() const
{
    return m_data;
}
