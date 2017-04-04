/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2017  David Rosca <nowrep@gmail.com>
* Copyright (C) 2017  Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#ifndef CHECKBOXDIALOG_H
#define CHECKBOXDIALOG_H

#include <QMessageBox>

#include "qzcommon.h"

class QUPZILLA_EXPORT CheckBoxDialog : public QMessageBox
{
    Q_OBJECT
public:
    explicit CheckBoxDialog(const QMessageBox::StandardButtons &buttons, QWidget* parent = 0);

    void setCheckBoxText(const QString &text);

    bool isChecked() const;
    void setDefaultCheckState(Qt::CheckState state);

private:
    QCheckBox* m_checkBox;

};

#endif // CHECKBOXDIALOG_H
