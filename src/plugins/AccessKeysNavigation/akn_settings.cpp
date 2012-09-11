/* ============================================================
* Access Keys Navigation plugin for QupZilla
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

#include "akn_settings.h"
#include "ui_akn_settings.h"
#include "akn_handler.h"
#include "licenseviewer.h"

#include <QSettings>
#include <QTextBrowser>

AKN_Settings::AKN_Settings(AKN_Handler* handler, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AKN_Settings)
    , m_handler(handler)
    , m_settingsFile(handler->settingsFile())
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup("AccessKeysNavigation");
    ui->key->setCurrentIndex(settings.value("Key", 0).toInt());
    ui->doubleClick->setChecked(settings.value("DoublePress", true).toBool());
    settings.endGroup();

    connect(ui->licence, SIGNAL(clicked()), this, SLOT(showLicence()));
    connect(this, SIGNAL(accepted()), this, SLOT(dialogAccepted()));
}

void AKN_Settings::dialogAccepted()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup("AccessKeysNavigation");
    settings.setValue("Key", ui->key->currentIndex());
    settings.setValue("DoublePress", ui->doubleClick->isChecked());
    settings.endGroup();

    m_handler->loadSettings();
}

void AKN_Settings::showLicence()
{
    LicenseViewer* v = new LicenseViewer(this);
    v->setLicenseFile(":accesskeysnavigation/data/copyright");
    v->show();
}

AKN_Settings::~AKN_Settings()
{
    delete ui;
}
