/* ============================================================
* VerticalTabs plugin for QupZilla
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "verticaltabssettings.h"
#include "ui_verticaltabssettings.h"
#include "verticaltabsplugin.h"

VerticalTabsSettings::VerticalTabsSettings(VerticalTabsPlugin *plugin, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::VerticalTabsSettings)
    , m_plugin(plugin)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    ui->tabListView->setChecked(m_plugin->viewType() == VerticalTabsPlugin::TabListView);
    ui->tabTreeView->setChecked(m_plugin->viewType() == VerticalTabsPlugin::TabTreeView);
    ui->appendChild->setChecked(m_plugin->addChildBehavior() == VerticalTabsPlugin::AppendChild);
    ui->prependChild->setChecked(m_plugin->addChildBehavior() == VerticalTabsPlugin::PrependChild);
    ui->replaceTabBar->setChecked(m_plugin->replaceTabBar());

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        m_plugin->setViewType(ui->tabListView->isChecked() ? VerticalTabsPlugin::TabListView : VerticalTabsPlugin::TabTreeView);
        m_plugin->setAddChildBehavior(ui->appendChild->isChecked() ? VerticalTabsPlugin::AppendChild : VerticalTabsPlugin::PrependChild);
        m_plugin->setReplaceTabBar(ui->replaceTabBar->isChecked());
        accept();
    });
}

VerticalTabsSettings::~VerticalTabsSettings()
{
    delete ui;
}
