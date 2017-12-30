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
#include "checkboxdialog.h"

#include <QCheckBox>

CheckBoxDialog::CheckBoxDialog(const QMessageBox::StandardButtons &buttons, QWidget* parent)
    : QMessageBox(parent)
    , m_checkBox(new QCheckBox)
{
    setCheckBox(m_checkBox);
    setStandardButtons(buttons);
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
}

void CheckBoxDialog::setCheckBoxText(const QString &text)
{
    m_checkBox->setText(text);
}

bool CheckBoxDialog::isChecked() const
{
    return m_checkBox->isChecked();
}

void CheckBoxDialog::setDefaultCheckState(Qt::CheckState state)
{
    m_checkBox->setChecked(state == Qt::Checked);
}
