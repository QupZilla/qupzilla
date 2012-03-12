/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "pluginlistdelegate.h"
#include "settings.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>

#ifdef PORTABLE_BUILD
#define DEFAULT_ENABLE_PLUGINS false
#else
#define DEFAULT_ENABLE_PLUGINS true
#endif

PluginsList::PluginsList(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PluginsList)
{
    ui->setupUi(this);

    //Application Extensions
    Settings settings;
    settings.beginGroup("Plugin-Settings");
    bool appPluginsEnabled = settings.value("EnablePlugins", DEFAULT_ENABLE_PLUGINS).toBool();
    settings.endGroup();

    ui->allowAppPlugins->setChecked(appPluginsEnabled);
    ui->list->setEnabled(appPluginsEnabled);

    connect(ui->butSettings, SIGNAL(clicked()), this, SLOT(settingsClicked()));
    connect(ui->list, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(currentChanged(QListWidgetItem*)));
    connect(ui->list, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
    connect(ui->allowAppPlugins, SIGNAL(clicked(bool)), this, SLOT(allowAppPluginsChanged(bool)));

    ui->list->setItemDelegate(new PluginListDelegate(ui->list));

    //WebKit Plugins
    connect(ui->add, SIGNAL(clicked()), this, SLOT(addWhitelist()));
    connect(ui->remove, SIGNAL(clicked()), this, SLOT(removeWhitelist()));
    connect(ui->allowClick2Flash, SIGNAL(clicked(bool)), this, SLOT(allowC2FChanged(bool)));

    settings.beginGroup("ClickToFlash");
    QStringList whitelist = mApp->plugins()->c2f_getWhiteList();
    ui->allowClick2Flash->setChecked(settings.value("Enable", true).toBool());
    settings.endGroup();
    foreach(const QString & site, whitelist) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->whitelist);
        item->setText(0, site);
    }

    allowC2FChanged(ui->allowClick2Flash->isChecked());

    if (appPluginsEnabled) {
        QTimer::singleShot(0, this, SLOT(refresh()));
    }
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
    QStringList allowedPlugins;
    for (int i = 0; i < ui->list->count(); i++) {
        QListWidgetItem* item = ui->list->item(i);

        if (item->checkState() == Qt::Checked) {
            const Plugins::Plugin &plugin = item->data(Qt::UserRole + 10).value<Plugins::Plugin>();

            allowedPlugins.append(plugin.fullPath);
        }
    }

    Settings settings;
    settings.beginGroup("Plugin-Settings");
    settings.setValue("EnablePlugins", ui->allowAppPlugins->isChecked());
    settings.setValue("AllowedPlugins", allowedPlugins);
    settings.endGroup();
}

void PluginsList::allowAppPluginsChanged(bool state)
{
    ui->list->setEnabled(state);

    if (state) {
        refresh();
    }
    else {
        for (int i = 0; i < ui->list->count(); i++) {
            QListWidgetItem* item = ui->list->item(i);

            if (item->checkState() == Qt::Checked) {
                item->setCheckState(Qt::Unchecked);
            }
        }
    }
}

void PluginsList::allowC2FChanged(bool state)
{
    Settings settings;
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
    disconnect(ui->list, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));

    const QList<Plugins::Plugin> &allPlugins = mApp->plugins()->getAvailablePlugins();

    foreach(const Plugins::Plugin & plugin, allPlugins) {
        PluginSpec spec = plugin.pluginSpec;

        QListWidgetItem* item = new QListWidgetItem(ui->list);
        QString pluginInfo = QString("<b>%1</b> %2 (%3)<br/>\n%4<br/>\n%5\n").arg(spec.name, spec.version, spec.author, spec.info, spec.description);
        item->setText(pluginInfo);


        QIcon icon = QIcon(spec.icon);
        if (icon.isNull()) {
            icon = QIcon(":/icons/preferences/extension.png");
        }
        item->setIcon(icon);

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(plugin.isLoaded() ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole + 10, qVariantFromValue(plugin));

        ui->list->addItem(item);
    }

    connect(ui->list, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
}

void PluginsList::currentChanged(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    const Plugins::Plugin &plugin = item->data(Qt::UserRole + 10).value<Plugins::Plugin>();
    bool showSettings = plugin.pluginSpec.hasSettings;

    if (!plugin.isLoaded()) {
        showSettings = false;
    }

    ui->butSettings->setEnabled(showSettings);
}

void PluginsList::itemChanged(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    Plugins::Plugin plugin = item->data(Qt::UserRole + 10).value<Plugins::Plugin>();

    if (item->checkState() == Qt::Checked) {
        mApp->plugins()->loadPlugin(&plugin);
    }
    else {
        mApp->plugins()->unloadPlugin(&plugin);
    }

    disconnect(ui->list, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));

    if (item->checkState() == Qt::Checked && !plugin.isLoaded()) {
        item->setCheckState(Qt::Unchecked);
        QMessageBox::critical(this, tr("Error!"), tr("Cannot load plugin!"));
    }

    item->setData(Qt::UserRole + 10, qVariantFromValue(plugin));

    connect(ui->list, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));


    currentChanged(ui->list->currentItem());
}

void PluginsList::settingsClicked()
{
    QListWidgetItem* item = ui->list->currentItem();
    if (!item) {
        return;
    }

    Plugins::Plugin plugin = item->data(Qt::UserRole + 10).value<Plugins::Plugin>();

    if (!plugin.isLoaded()) {
        mApp->plugins()->loadPlugin(&plugin);

        item->setData(Qt::UserRole + 10, qVariantFromValue(plugin));
    }

    if (plugin.isLoaded()) {
        plugin.instance->showSettings(this);
    }
}

PluginsList::~PluginsList()
{
    delete ui;
}
