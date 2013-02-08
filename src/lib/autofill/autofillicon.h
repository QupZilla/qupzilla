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
#ifndef AUTOFILLICON_H
#define AUTOFILLICON_H

#include "qz_namespace.h"
#include "clickablelabel.h"
#include "autofill.h"

struct AutoFillData;

class QT_QUPZILLA_EXPORT AutoFillIcon : public ClickableLabel
{
public:
    explicit AutoFillIcon(QWidget* parent = 0);

    void setFormData(const QList<AutoFillData> &data);
    QList<AutoFillData> formData() const;

private:
    QList<AutoFillData> m_data;

};

#endif // AUTOFILLICON_H
