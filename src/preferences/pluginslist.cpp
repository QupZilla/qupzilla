/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#include "pluginslist.h"
#include "ui_pluginslist.h"
#include "pluginproxy.h"
#include "mainapplication.h"
#include "plugininterface.h"

PluginsList::PluginsList(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PluginsList)
{
    ui->setupUi(this);

    //Application Extensions
    refresh();
    connect(ui->butSettings, SIGNAL(clicked()), this, SLOT(settingsClicked()));
    connect(ui->list, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(currentChanged(QListWidgetItem*)));
    connect(ui->butLoad, SIGNAL(clicked()), this, SLOT(reloadPlugins()));
    connect(ui->allowAppPlugins, SIGNAL(clicked(bool)), this, SLOT(allowAppPluginsChanged(bool)));

    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Plugin-Settings");
    ui->allowAppPlugins->setChecked(settings.value("EnablePlugins", true).toBool());
    settings.endGroup();
    allowAppPluginsChanged(ui->allowAppPlugins->isChecked());

    //WebKit Plugins
    connect(ui->add, SIGNAL(clicked()), this, SLOT(addWhitelist()));
    connect(ui->remove, SIGNAL(clicked()), this, SLOT(removeWhitelist()));
    connect(ui->allowClick2Flash, SIGNAL(clicked(bool)), this, SLOT(allowC2FChanged(bool)));

    settings.beginGroup("ClickToFlash");
    QStringList whitelist = mApp->plugins()->c2f_getWhiteList();
    ui->allowClick2Flash->setChecked(settings.value("Enable", true).toBool());
    settings.endGroup();
    foreach(QString site, whitelist) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->whitelist);
        item->setText(0, site);
    }
    allowC2FChanged(ui->allowClick2Flash->isChecked());
}

void PluginsList::addWhitelist()
{
    QString site = QInputDialog::getText(this, tr("Add site to whitelist"), tr("Server without http:// (ex. youtube.com)"));
    if (site.isEmpty()) {
        return;
    }

    mApp->plugins()->c2f_addWhitelist(site);
    ui->whitelist->insertTopLevelItem(0, new QTreeWidgetItem(QStringList(site)));
}

void PluginsList::removeWhitelist()
{
    QTreeWidgetItem* item = ui->whitelist->currentItem();
    if (!item) {
        return;
    }

    mApp->plugins()->c2f_removeWhitelist(item->text(0));
    delete item;
}

void PluginsList::save()
{
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Plugin-Settings");
    settings.setValue("EnablePlugins", ui->allowAppPlugins->isChecked());
    settings.endGroup();

    reloadPlugins();
}

void PluginsList::allowAppPluginsChanged(bool state)
{
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Plugin-Settings");
    settings.setValue("EnablePlugins", state);
    settings.endGroup();

    ui->verticalFrame->setEnabled(state);
}

void PluginsList::allowC2FChanged(bool state)
{
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("ClickToFlash");
    settings.setValue("Enable", state);
    settings.endGroup();

    ui->whitelist->setEnabled(state);
    ui->add->setEnabled(state);
    ui->remove->setEnabled(state);

    mApp->plugins()->c2f_setEnabled(state);
}

void PluginsList::refresh()
{
    ui->list->clear();
    ui->butSettings->setEnabled(false);

    QStringList availablePlugins = mApp->plugins()->getAvailablePlugins();
    QStringList allowedPlugins = mApp->plugins()->getAllowedPlugins();
    foreach(QString fileName, availablePlugins) {
        PluginInterface* plugin = mApp->plugins()->getPlugin(fileName);
        if (!plugin) {
            continue;
        }

        QListWidgetItem* item = new QListWidgetItem(ui->list);
        item->setText("" + plugin->pluginName() + " (" + plugin->pluginVersion() + ") by " + plugin->pluginAuthor() + "\n"
                      + plugin->pluginInfo() + "\n" + plugin->pluginDescription());

        QIcon icon = plugin->pluginIcon();
        if (icon.isNull()) {
            icon = QIcon(":/icons/preferences/extension.png");
        }
        item->setIcon(icon);

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState((allowedPlugins.contains(fileName)) ? Qt::Checked : Qt::Unchecked);
        item->setWhatsThis(plugin->hasSettings() ? "1" : "0");
        item->setToolTip(fileName);

        ui->list->addItem(item);
    }
}

void PluginsList::currentChanged(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    QString has = item->whatsThis();
    bool show;
    if (has == "1") {
        show = true;
    }
    else {
        show = false;
    }

    if (item->checkState() == Qt::Unchecked) {
        show = false;
    }

    ui->butSettings->setEnabled(show);
}

void PluginsList::settingsClicked()
{
    if (!ui->list->currentItem()) {
        return;
    }

    QString name = ui->list->currentItem()->toolTip();
    PluginInterface* plugin = mApp->plugins()->getPlugin(name);
    plugin->showSettings();
}

void PluginsList::reloadPlugins()
{
    QStringList allowedPlugins;
    for (int i = 0; i < ui->list->count(); i++) {
        if (ui->list->item(i)->checkState() == Qt::Checked) {
            allowedPlugins.append(ui->list->item(i)->toolTip());
        }
    }
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Plugin-Settings");
    settings.setValue("AllowedPlugins", allowedPlugins);
    settings.endGroup();

    mApp->plugins()->loadSettings();
    mApp->plugins()->loadPlugins();

    refresh();
}

PluginsList::~PluginsList()
{
    delete ui;
}
