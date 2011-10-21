#include "searchenginesdialog.h"
#include "ui_searchenginesdialog.h"
#include "editsearchengine.h"
#include "searchenginesmanager.h"
#include "mainapplication.h"

SearchEnginesDialog::SearchEnginesDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SearchEnginesDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    m_manager = mApp->searchEnginesManager();

    connect(ui->add, SIGNAL(clicked()), this, SLOT(addEngine()));
    connect(ui->remove, SIGNAL(clicked()), this, SLOT(removeEngine()));
    connect(ui->edit, SIGNAL(clicked()), this, SLOT(editEngine()));
    connect(ui->defaults, SIGNAL(clicked()), this, SLOT(defaults()));
    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(editEngine()));

    reloadEngines();
}

void SearchEnginesDialog::addEngine()
{
    EditSearchEngine dialog(tr("Add Search Engine"), this);
    dialog.hideIconLabels();

    if (dialog.exec() != QDialog::Accepted)
        return;

    SearchEngine engine;
    engine.name = dialog.name();
    engine.url = dialog.url();
    engine.shortcut = dialog.shortcut();
    engine.icon = SearchEnginesManager::iconForSearchEngine(dialog.url());

    if (engine.name.isEmpty() || engine.url.isEmpty())
        return;

    QTreeWidgetItem* item = new QTreeWidgetItem();
    QVariant v;
    v.setValue<SearchEngine>(engine);
    item->setData(0, Qt::UserRole, v);

    item->setText(0, engine.name);
    item->setIcon(0, engine.icon);
    item->setText(1, engine.shortcut);

    ui->treeWidget->addTopLevelItem(item);
}

void SearchEnginesDialog::removeEngine()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (!item || ui->treeWidget->topLevelItemCount() == 1)
        return;

    delete item;
}

void SearchEnginesDialog::editEngine()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (!item)
        return;

    SearchEngine engine = item->data(0, Qt::UserRole).value<SearchEngine>();

    EditSearchEngine dialog(tr("Edit Search Engine"), this);

    dialog.setName(engine.name);
    dialog.setUrl(engine.url);
    dialog.setShortcut(engine.shortcut);
    dialog.setIcon(engine.icon);

    if (dialog.exec() != QDialog::Accepted)
        return;

    engine.name = dialog.name();
    engine.url = dialog.url();
    engine.shortcut = dialog.shortcut();
    engine.icon = dialog.icon();

    if (engine.name.isEmpty() || engine.url.isEmpty())
        return;

    QVariant v;
    v.setValue<SearchEngine>(engine);
    item->setData(0, Qt::UserRole, v);

    item->setText(0, engine.name);
    item->setIcon(0, engine.icon);
    item->setText(1, engine.shortcut);
}

void SearchEnginesDialog::defaults()
{
    m_manager->restoreDefaults();
    reloadEngines();
}

void SearchEnginesDialog::reloadEngines()
{
    ui->treeWidget->clear();

    foreach (SearchEngine en, m_manager->allEngines()) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setIcon(0, en.icon);
        item->setText(0, en.name);
        item->setText(1, en.shortcut);
        QVariant v;
        v.setValue<SearchEngine>(en);
        item->setData(0, Qt::UserRole, v);

        ui->treeWidget->addTopLevelItem(item);
    }
}

void SearchEnginesDialog::accept()
{
    if (ui->treeWidget->topLevelItemCount() < 1)
        return;

    QList<SearchEngine> allEngines;

    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);
        if (!item)
            continue;

        SearchEngine engine = item->data(0, Qt::UserRole).value<SearchEngine>();
        allEngines.append(engine);
    }

    m_manager->setAllEngines(allEngines);

    QDialog::accept();
}

SearchEnginesDialog::~SearchEnginesDialog()
{
    delete ui;
}
