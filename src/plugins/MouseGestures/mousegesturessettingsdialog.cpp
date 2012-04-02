/* ============================================================
* Mouse Gestures plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
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
#include "mousegesturessettingsdialog.h"
#include "ui_mousegesturessettingsdialog.h"
#include "licenseviewer.h"

MouseGesturesSettingsDialog::MouseGesturesSettingsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::MouseGesturesSettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->licenseButton, SIGNAL(clicked()), this, SLOT(showLicense()));
}

MouseGesturesSettingsDialog::~MouseGesturesSettingsDialog()
{
    delete ui;
}

void MouseGesturesSettingsDialog::showLicense()
{
    LicenseViewer* v = new LicenseViewer(this);
    v->setLicenseFile(":mousegestures/data/copyright");

    v->show();
}
