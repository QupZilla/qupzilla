/* ============================================================
* AutoScroll - Autoscroll for QupZilla
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#include "autoscrollsettings.h"
#include "ui_autoscrollsettings.h"
#include "qzcommon.h"

#include <QSettings>

AutoScrollSettings::AutoScrollSettings(const QString &settingsPath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AutoScrollSettings)
    , m_settingsPath(settingsPath)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    QSettings settings(m_settingsPath + QL1S("/extensions.ini"), QSettings::IniFormat);
    settings.beginGroup(QSL("AutoScroll"));

    ui->speed->setValue(11 - settings.value(QSL("Speed"), 5).toInt());
    ui->ctrlScroll->setChecked(settings.value(QSL("CtrlClick"), false).toBool());
    ui->middleScroll->setChecked(settings.value(QSL("MiddleClick"), true).toBool());

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

AutoScrollSettings::~AutoScrollSettings()
{
    delete ui;
}

void AutoScrollSettings::accepted()
{
    QSettings settings(m_settingsPath + QL1S("/extensions.ini"), QSettings::IniFormat);
    settings.beginGroup(QSL("AutoScroll"));

    settings.setValue(QSL("Speed"), 11 - ui->speed->value());
    settings.setValue(QSL("CtrlClick"), ui->ctrlScroll->isChecked());
    settings.setValue(QSL("MiddleClick"), ui->middleScroll->isChecked());

    close();

    emit settingsChanged();
}
