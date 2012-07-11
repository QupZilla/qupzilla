/* ============================================================
* GreaseMonkey plugin for QupZilla
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
#include "gm_settingsscriptinfo.h"
#include "ui_gm_settingsscriptinfo.h"
#include "../gm_script.h"

GM_SettingsScriptInfo::GM_SettingsScriptInfo(GM_Script* script, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::GM_SettingsScriptInfo)
{
    ui->setupUi(this);
    setWindowTitle(tr("Script Details of %1").arg(script->name()));

    ui->name->setText(script->fullName());
    ui->version->setText(script->version());
    ui->url->setText(script->downloadUrl().toString());
    ui->startAt->setText(script->startAt() == GM_Script::DocumentStart ? "document-start" : "document-end");
    ui->description->setText(script->description());
    ui->include->setText(script->include().join("<br/>"));
    ui->exclude->setText(script->exclude().join("<br/>"));
}

GM_SettingsScriptInfo::~GM_SettingsScriptInfo()
{
    delete ui;
}
