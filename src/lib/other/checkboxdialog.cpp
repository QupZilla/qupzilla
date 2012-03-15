/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "ui_checkboxdialog.h"

CheckBoxDialog::CheckBoxDialog(const QDialogButtonBox::StandardButtons &buttons, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CheckBoxDialog)
{
    ui->setupUi(this);

    ui->buttonBox->setStandardButtons(buttons);
}

void CheckBoxDialog::setPixmap(const QPixmap &pixmap)
{
    ui->iconLabel->setPixmap(pixmap);
    ui->iconLabel->setFixedWidth(pixmap.width());
}

void CheckBoxDialog::setText(const QString &text)
{
    ui->textLabel->setText(text);
}

void CheckBoxDialog::setCheckBoxText(const QString &text)
{
    ui->checkBox->setText(text);
}

bool CheckBoxDialog::isChecked() const
{
    return ui->checkBox->isChecked();
}

int CheckBoxDialog::exec()
{
    ui->buttonBox->setFocus();
    setMaximumSize(size());

    return QDialog::exec();
}
