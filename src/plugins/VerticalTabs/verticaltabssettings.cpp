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

#include <QDir>
#include <QFileDialog>

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

    loadThemes();

    connect(ui->theme, SIGNAL(activated(int)), this, SLOT(themeValueChanged(int)));
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        m_plugin->setViewType(ui->tabListView->isChecked() ? VerticalTabsPlugin::TabListView : VerticalTabsPlugin::TabTreeView);
        m_plugin->setAddChildBehavior(ui->appendChild->isChecked() ? VerticalTabsPlugin::AppendChild : VerticalTabsPlugin::PrependChild);
        m_plugin->setReplaceTabBar(ui->replaceTabBar->isChecked());
        m_plugin->setTheme(ui->theme->currentData().toString());
        accept();
    });
}

VerticalTabsSettings::~VerticalTabsSettings()
{
    delete ui;
}

void VerticalTabsSettings::themeValueChanged(int index)
{
    const int customIndex = ui->theme->count() - 1;
    if (index == customIndex) {
        const QString path = QFileDialog::getOpenFileName(this, tr("Theme file"), QDir::homePath(), {QSL("*.css")});
        if (path.isEmpty()) {
            loadThemes();
        } else {
            ui->theme->setToolTip(path);
            ui->theme->setItemData(customIndex, path);
        }
    } else {
        ui->theme->setToolTip(QString());
    }
}

void VerticalTabsSettings::loadThemes()
{
    ui->theme->clear();
    bool found = false;
    const auto files = QDir(QSL(":verticaltabs/data/themes")).entryInfoList({QSL("*.css")});
    for (const QFileInfo &file : files) {
        ui->theme->addItem(file.baseName(), file.absoluteFilePath());
        if (file.absoluteFilePath() == m_plugin->theme()) {
            ui->theme->setCurrentIndex(ui->theme->count() - 1);
            found = true;
        }
    }
    ui->theme->setToolTip(m_plugin->theme());
    ui->theme->addItem(tr("Custom..."), found ? QString() : m_plugin->theme());
    if (!found) {
        ui->theme->setCurrentIndex(ui->theme->count() - 1);
    }
}
