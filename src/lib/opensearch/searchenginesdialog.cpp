/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#include "mainapplication.h"
#include "removeitemfocusdelegate.h"

#include <QMessageBox>

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

    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(editEngine()));

    ui->treeWidget->setItemDelegate(new RemoveItemFocusDelegate(ui->treeWidget));
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
    engine.postData = dialog.postData().toUtf8();
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

    if (isDefaultEngine(item)) {
        SearchEngine en = getEngine(item);
        QMessageBox::warning(this, tr("Remove Engine"),
                             tr("You can't remove the default search engine.<br>"
                                "Set a different engine as default before removing %1.").arg(en.name));
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
    dialog.setPostData(engine.postData);
    dialog.setShortcut(engine.shortcut);
    dialog.setIcon(engine.icon);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    engine.name = dialog.name();
    engine.url = dialog.url();
    engine.postData = dialog.postData().toUtf8();
    engine.shortcut = dialog.shortcut();
    engine.icon = dialog.icon();

    if (engine.name.isEmpty() || engine.url.isEmpty()) {
        return;
    }

    setEngine(item, engine);

    changeItemToDefault(item, isDefaultEngine(item));
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
        if (isDefaultEngine(i)) {
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

bool SearchEnginesDialog::isDefaultEngine(QTreeWidgetItem* item)
{
    return item->data(0, DefaultRole).toBool();
}

SearchEngine SearchEnginesDialog::getEngine(QTreeWidgetItem* item)
{
    return item->data(0, EngineRole).value<SearchEngine>();
}

void SearchEnginesDialog::setEngine(QTreeWidgetItem* item, SearchEngine engine)
{
    QVariant v;
    v.setValue<SearchEngine>(engine);
    item->setData(0, EngineRole, v);
    item->setText(0, engine.name);
}

void SearchEnginesDialog::changeItemToDefault(QTreeWidgetItem* item, bool isDefault)
{
    QFont font = item->font(0);
    font.setBold(isDefault);

    item->setFont(0, font);
    item->setFont(1, font);
    item->setData(0, DefaultRole, isDefault);
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
    const SearchEngine defaultEngine = mApp->searchEnginesManager()->defaultEngine();

    foreach (const SearchEngine &en, m_manager->allEngines()) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        setEngine(item, en);
        changeItemToDefault(item, en == defaultEngine);
        item->setIcon(0, en.icon);
        item->setText(1, en.shortcut);

        ui->treeWidget->addTopLevelItem(item);
    }
}

void SearchEnginesDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    resizeViewHeader();
}

void SearchEnginesDialog::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    resizeViewHeader();
}

void SearchEnginesDialog::resizeViewHeader()
{
    const int headerWidth = ui->treeWidget->header()->width();
    ui->treeWidget->header()->resizeSection(0, headerWidth - headerWidth / 4);
}

void SearchEnginesDialog::accept()
{
    if (ui->treeWidget->topLevelItemCount() < 1) {
        return;
    }

    QVector<SearchEngine> allEngines;

    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);
        if (!item) {
            continue;
        }

        SearchEngine engine = getEngine(item);
        allEngines.append(engine);

        if (isDefaultEngine(item)) {
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
