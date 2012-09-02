/* ============================================================
* Personal Information Manager plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
* Copyright (C) 2012  Mladen Pejaković <pejakm@gmail.com>
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

#include "PIM_settings.h"
#include "ui_PIM_settings.h"
#include "PIM_handler.h"

#include <QSettings>

PIM_Settings::PIM_Settings(const QString &settingsFile, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PIM_Settings)
    , m_settingsFile(settingsFile)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup("PIM");
    ui->pim_firstname->setText(settings.value("FirstName", QString()).toString());
    ui->pim_lastname->setText(settings.value("LastName", QString()).toString());
    ui->pim_email->setText(settings.value("Email", QString()).toString());
    ui->pim_phone->setText(settings.value("Phone", QString()).toString());
    ui->pim_mobile->setText(settings.value("Mobile", QString()).toString());
    ui->pim_address->setText(settings.value("Address", QString()).toString());
    ui->pim_city->setText(settings.value("City", QString()).toString());
    ui->pim_zip->setText(settings.value("Zip", QString()).toString());
    ui->pim_state->setText(settings.value("State", QString()).toString());
    ui->pim_country->setText(settings.value("Country", QString()).toString());
    ui->pim_homepage->setText(settings.value("HomePage", QString()).toString());
    ui->pim_special1->setText(settings.value("Special1", QString()).toString());
    ui->pim_special2->setText(settings.value("Special2", QString()).toString());
    ui->pim_special3->setText(settings.value("Special3", QString()).toString());
    settings.endGroup();

    connect(this, SIGNAL(accepted()), this, SLOT(dialogAccepted()));
}

void PIM_Settings::dialogAccepted()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup("PIM");
    settings.setValue("FirstName", ui->pim_firstname->text());
    settings.setValue("LastName", ui->pim_lastname->text());
    settings.setValue("Email", ui->pim_email->text());
    settings.setValue("Phone", ui->pim_phone->text());
    settings.setValue("Mobile", ui->pim_mobile->text());
    settings.setValue("Address", ui->pim_address->text());
    settings.setValue("City", ui->pim_city->text());
    settings.setValue("Zip", ui->pim_zip->text());
    settings.setValue("State", ui->pim_state->text());
    settings.setValue("Country", ui->pim_country->text());
    settings.setValue("HomePage", ui->pim_homepage->text());
    settings.setValue("Special1", ui->pim_special1->text());
    settings.setValue("Special2", ui->pim_special2->text());
    settings.setValue("Special3", ui->pim_special3->text());
    settings.endGroup();
}

PIM_Settings::~PIM_Settings()
{
    delete ui;
}
