/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2013  David Rosca <nowrep@gmail.com>
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

#include <QDesktopServices>

GM_SettingsScriptInfo::GM_SettingsScriptInfo(GM_Script* script, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::GM_SettingsScriptInfo)
    , m_script(script)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    loadScript();

    connect(m_script, SIGNAL(scriptChanged()), this, SLOT(loadScript()));
    connect(ui->editInEditor, SIGNAL(clicked()), this, SLOT(editInTextEditor()));
}

void GM_SettingsScriptInfo::editInTextEditor()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_script->fileName()));
}

void GM_SettingsScriptInfo::loadScript()
{
    setWindowTitle(tr("Script Details of %1").arg(m_script->name()));

    ui->name->setText(m_script->name());
    ui->nspace->setText(m_script->nameSpace());
    ui->version->setText(m_script->version());
    ui->url->setText(m_script->downloadUrl().toString());
    ui->startAt->setText(m_script->startAt() == GM_Script::DocumentStart ? "document-start" : "document-end");
    ui->description->setText(m_script->description());
    ui->include->setText(m_script->include().join("<br/>"));
    ui->exclude->setText(m_script->exclude().join("<br/>"));

    ui->version->setVisible(!m_script->version().isEmpty());
    ui->labelVersion->setVisible(!m_script->version().isEmpty());

    ui->url->setVisible(!m_script->downloadUrl().isEmpty());
    ui->labelUrl->setVisible(!m_script->downloadUrl().isEmpty());
}

GM_SettingsScriptInfo::~GM_SettingsScriptInfo()
{
    delete ui;
}

