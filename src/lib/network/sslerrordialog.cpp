/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#include "sslerrordialog.h"
#include "ui_sslerrordialog.h"
#include "iconprovider.h"

#include <QPushButton>

SslErrorDialog::SslErrorDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SslErrorDialog)
    , m_result(No)
{
    ui->setupUi(this);
    ui->icon->setPixmap(IconProvider::standardIcon(QStyle::SP_MessageBoxCritical).pixmap(52));
    // Disabled until there is reliable way to save certificate error
    //ui->buttonBox->addButton(tr("Only for this session"), QDialogButtonBox::ApplyRole);
    ui->buttonBox->button(QDialogButtonBox::No)->setFocus();

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}

SslErrorDialog::~SslErrorDialog()
{
    delete ui;
}

void SslErrorDialog::setText(const QString &text)
{
    ui->text->setText(text);
}

SslErrorDialog::Result SslErrorDialog::result()
{
    return m_result;
}

void SslErrorDialog::buttonClicked(QAbstractButton* button)
{
    switch (ui->buttonBox->buttonRole(button)) {
    case QDialogButtonBox::YesRole:
        m_result = Yes;
        accept();
        break;

    case QDialogButtonBox::ApplyRole:
        m_result = OnlyForThisSession;
        accept();
        break;

    default:
        m_result = No;
        reject();
        break;
    }
}
