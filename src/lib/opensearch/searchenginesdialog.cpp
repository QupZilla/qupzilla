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
#include "searchenginesdialog.h"
#include "ui_searchenginesdialog.h"
#include "editsearchengine.h"
#include "searchenginesmanager.h"
#include "mainapplication.h"
#include <QMessageBox>

namespace
{
const int EngineRole = Qt::UserRole;
const int DefaultRole = Qt::UserRole + 1;

bool isDefault(QTreeWidgetItem* item)
{
    return item->data(0, DefaultRole).toBool();
}

SearchEngine getEngine(QTreeWidgetItem* item)
{
    return item->data(0, EngineRole).value<SearchEngine>();
}

void setEngine(QTreeWidgetItem* item, SearchEngine engine)
{
    QVariant v;
    v.setValue<SearchEngine>(engine);
    item->setData(0, EngineRole, v);
}

void changeItemToDefault(QTreeWidgetItem* item, bool isDefault)
{
    QString txt = item->data(0, EngineRole).value<SearchEngine>().name;
    if (isDefault) {
        txt.append(QString(" (%1)").arg(QObject::tr("Default")));
    }

    item->setText(0, txt);
    item->setData(0, DefaultRole, isDefault);
}
}

SearchEnginesDialog::SearchEnginesDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SearchEnginesDialog)
    , m_manager(mApp->searchEnginesManager())
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    connect(ui->add, SIGNAL(clicked()), this, SLOT(addEngine()));
    connect(ui->remove, SIGNAL(clicked()), this, SLOT(removeEngine()));
    connect(ui->edit, SIGNAL(clicked()), this, SLOT(editEngine()));
    connect(ui->setAsDefault, SIGNAL(clicked()), this, SLOT(setDefaultEngine()));
    connect(ui->defaults, SIGNAL(clicked()), this, SLOT(defaults()));
    connect(ui->moveUp, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(ui->moveDown, SIGNAL(clicked()), this, SLOT(moveDown()));

    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(editEngine()));

    ui->treeWidget->sortByColumn(-1);
    reloadEngines();
}

void SearchEnginesDialog::addEngine()
{
    EditSearchEngine dialog(tr("Add Search Engine"), this);
    dialog.hideIconLabels();

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    SearchEngine engine;
    engine.name = dialog.name();
    engine.url = dialog.url();
    engine.shortcut = dialog.shortcut();
    engine.icon = SearchEnginesManager::iconForSearchEngine(QUrl::fromEncoded(dialog.url().toUtf8()));

    if (engine.name.isEmpty() || engine.url.isEmpty()) {
        return;
    }

    QTreeWidgetItem* item = new QTreeWidgetItem();
    setEngine(item, engine);

    changeItemToDefault(item, false);
    item->setIcon(0, engine.icon);
    item->setText(1, engine.shortcut);

    ui->treeWidget->addTopLevelItem(item);
}

void SearchEnginesDialog::removeEngine()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (!item || ui->treeWidget->topLevelItemCount() == 1) {
        return;
    }

    if (isDefault(item)) {
        SearchEngine en = getEngine(item);
        QMessageBox::warning(this, tr("Remove Engine"),
                             tr("You can't remove the default search engine.<br>"
                                "Set a different engine as Default before removing %1.").arg(en.name));
    }
    else {
        delete item;
    }
}

void SearchEnginesDialog::editEngine()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (!item) {
        return;
    }

    SearchEngine engine = getEngine(item);

    EditSearchEngine dialog(tr("Edit Search Engine"), this);

    dialog.setName(engine.name);
    dialog.setUrl(engine.url);
    dialog.setShortcut(engine.shortcut);
    dialog.setIcon(engine.icon);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    engine.name = dialog.name();
    engine.url = dialog.url();
    engine.shortcut = dialog.shortcut();
    engine.icon = dialog.icon();

    if (engine.name.isEmpty() || engine.url.isEmpty()) {
        return;
    }

    setEngine(item, engine);

    changeItemToDefault(item, isDefault(item));
    item->setIcon(0, engine.icon);
    item->setText(1, engine.shortcut);
}

void SearchEnginesDialog::setDefaultEngine()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (!item) {
        return;
    }

    for (int j = 0; j < ui->treeWidget->topLevelItemCount(); ++j) {
        QTreeWidgetItem* i = ui->treeWidget->topLevelItem(j);
        if (isDefault(i)) {
            if (i == item) {
                return;
            }
            changeItemToDefault(i, false);
            break;
        }
    }

    changeItemToDefault(item, true);
}

void SearchEnginesDialog::defaults()
{
    m_manager->restoreDefaults();
    reloadEngines();
}

void SearchEnginesDialog::moveUp()
{
    QTreeWidgetItem* currentItem = ui->treeWidget->currentItem();
    int index = ui->treeWidget->indexOfTopLevelItem(currentItem);

    if (!currentItem || index == 0) {
        return;
    }

    ui->treeWidget->takeTopLevelItem(index);
    ui->treeWidget->insertTopLevelItem(index - 1, currentItem);
    ui->treeWidget->setCurrentItem(currentItem);
}

void SearchEnginesDialog::moveDown()
{
    QTreeWidgetItem* currentItem = ui->treeWidget->currentItem();
    int index = ui->treeWidget->indexOfTopLevelItem(currentItem);

    if (!currentItem || !ui->treeWidget->itemBelow(currentItem)) {
        return;
    }

    ui->treeWidget->takeTopLevelItem(index);
    ui->treeWidget->insertTopLevelItem(index + 1, currentItem);
    ui->treeWidget->setCurrentItem(currentItem);
}

void SearchEnginesDialog::reloadEngines()
{
    ui->treeWidget->clear();
    const QString defaultEngineName = mApp->searchEnginesManager()->defaultEngine().name;

    foreach(const SearchEngine & en, m_manager->allEngines()) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        setEngine(item, en);
        changeItemToDefault(item, en.name == defaultEngineName);
        item->setIcon(0, en.icon);
        item->setText(1, en.shortcut);

        ui->treeWidget->addTopLevelItem(item);
    }
}

void SearchEnginesDialog::accept()
{
    if (ui->treeWidget->topLevelItemCount() < 1) {
        return;
    }

    QList<SearchEngine> allEngines;

    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);
        if (!item) {
            continue;
        }

        SearchEngine engine = getEngine(item);
        allEngines.append(engine);

        if (isDefault(item)) {
            m_manager->setDefaultEngine(engine);
        }
    }

    m_manager->setAllEngines(allEngines);

    QDialog::accept();
}

SearchEnginesDialog::~SearchEnginesDialog()
{
    delete ui;
}
