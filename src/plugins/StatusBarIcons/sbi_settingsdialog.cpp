/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
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
#include "sbi_settingsdialog.h"
#include "ui_sbi_settingsdialog.h"
#include "sbi_iconsmanager.h"

SBI_SettingsDialog::SBI_SettingsDialog(SBI_IconsManager* manager, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SBI_SettingsDialog)
    , m_manager(manager)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    ui->showImagesIcon->setChecked(m_manager->showImagesIcon());
    ui->showJavaScriptIcon->setChecked(m_manager->showJavaScriptIcon());
    ui->showNetworkIcon->setChecked(m_manager->showNetworkIcon());
    ui->showZoomWidget->setChecked(m_manager->showZoomWidget());

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

void SBI_SettingsDialog::saveSettings()
{
    m_manager->setShowImagesIcon(ui->showImagesIcon->isChecked());
    m_manager->setShowJavaScriptIcon(ui->showJavaScriptIcon->isChecked());
    m_manager->setShowNetworkIcon(ui->showNetworkIcon->isChecked());
    m_manager->setShowZoomWidget(ui->showZoomWidget->isChecked());

    m_manager->reloadIcons();
    close();
}

SBI_SettingsDialog::~SBI_SettingsDialog()
{
    delete ui;
}
