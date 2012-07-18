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
#include "gm_settings.h"
#include "ui_gm_settings.h"
#include "gm_settingsscriptinfo.h"
#include "../gm_manager.h"
#include "../gm_script.h"

#include "mainapplication.h"

#include <QDesktopServices>
#include <QMessageBox>

GM_Settings::GM_Settings(GM_Manager* manager, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::GM_Settings)
    , m_manager(manager)
{
    ui->setupUi(this);

    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(showItemInfo(QListWidgetItem*)));
    connect(ui->listWidget, SIGNAL(removeItemRequested(QListWidgetItem*)), this, SLOT(removeItem(QListWidgetItem*)));
    connect(ui->openDirectory, SIGNAL(clicked()), this, SLOT(openScriptsDirectory()));
    connect(ui->link, SIGNAL(clicked(QPoint)), this, SLOT(openUserscripts()));

    loadScripts();
}

void GM_Settings::openUserscripts()
{
    mApp->addNewTab(QUrl("http://www.userscripts.org"));
    close();
}

void GM_Settings::showItemInfo(QListWidgetItem* item)
{
    GM_Script* script = getScript(item);
    if (!script) {
        return;
    }

    GM_SettingsScriptInfo info(script, this);
    info.exec();
}

void GM_Settings::removeItem(QListWidgetItem* item)
{
    GM_Script* script = getScript(item);
    if (!script) {
        return;
    }

    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Remove script"),
                                         tr("Are you sure you want to remove '%1'?").arg(script->name()),
                                         QMessageBox::Yes | QMessageBox::No);

    if (button == QMessageBox::Yes && m_manager->removeScript(script)) {
        delete item;
    }
}

void GM_Settings::itemChanged(QListWidgetItem* item)
{
    GM_Script* script = getScript(item);
    if (!script) {
        return;
    }

    if (item->checkState() == Qt::Checked) {
        m_manager->enableScript(script);
    }
    else {
        m_manager->disableScript(script);
    }
}

void GM_Settings::openScriptsDirectory()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_manager->scriptsDirectory()));
}

void GM_Settings::loadScripts()
{
    disconnect(ui->listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));

    foreach(GM_Script * script, m_manager->allScripts()) {
        QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
        QIcon icon = QIcon(":/gm/data/script.png");
        item->setIcon(icon);

        item->setText(script->name());
        item->setData(Qt::UserRole, script->version());
        item->setData(Qt::UserRole + 1, script->description());

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(script->isEnabled() ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole + 10, qVariantFromValue((void*)script));

        ui->listWidget->addItem(item);
    }

    ui->listWidget->sortItems();

    bool itemMoved;
    do {
        itemMoved = false;
        for (int i = 0; i < ui->listWidget->count(); ++i) {
            QListWidgetItem* topItem = ui->listWidget->item(i);
            QListWidgetItem* bottomItem = ui->listWidget->item(i + 1);
            if (!topItem || !bottomItem) {
                continue;
            }

            if (topItem->checkState() == Qt::Unchecked && bottomItem->checkState() == Qt::Checked) {
                QListWidgetItem* item = ui->listWidget->takeItem(i + 1);
                ui->listWidget->insertItem(i, item);
                itemMoved = true;
            }
        }
    }
    while (itemMoved);

    connect(ui->listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
}

GM_Script* GM_Settings::getScript(QListWidgetItem* item)
{
    if (!item) {
        return 0;
    }

    GM_Script* script = static_cast<GM_Script*>(item->data(Qt::UserRole + 10).value<void*>());
    return script;
}

GM_Settings::~GM_Settings()
{
    delete ui;
}
