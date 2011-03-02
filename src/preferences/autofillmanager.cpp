#include "autofillmanager.h"
#include "ui_autofillmanager.h"

AutoFillManager::AutoFillManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AutoFillManager)
{
    ui->setupUi(this);

    connect(ui->removePass, SIGNAL(clicked()), this, SLOT(removePass()));
    connect(ui->removeAllPass, SIGNAL(clicked()), this, SLOT(removeAllPass()));
    connect(ui->editPass, SIGNAL(clicked()), this, SLOT(editPass()));

    connect(ui->removeExcept, SIGNAL(clicked()), this, SLOT(removeExcept()));
    connect(ui->removeAllExcept, SIGNAL(clicked()), this, SLOT(removeAllExcept()));

    QTimer::singleShot(0, this, SLOT(loadPasswords()));
}

void AutoFillManager::loadPasswords()
{
    QSqlQuery query;
    query.exec("SELECT server, password, id FROM autofill");
    ui->treePass->clear();
    while(query.next()) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treePass);
        item->setText(0, query.value(0).toString());
        item->setText(1, query.value(1).toString());
        item->setWhatsThis(1, query.value(2).toString());
        ui->treePass->addTopLevelItem(item);
    }

    query.exec("SELECT server, id FROM autofill_exceptions");
    ui->treeExcept->clear();
    while(query.next()) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeExcept);
        item->setText(0, query.value(0).toString());
        item->setWhatsThis(0, query.value(1).toString());
        ui->treeExcept->addTopLevelItem(item);
    }
}

void AutoFillManager::removePass()
{
    QTreeWidgetItem* curItem = ui->treePass->currentItem();
    if (!curItem)
        return;
    QString id = curItem->whatsThis(1);
    QSqlQuery query;
    query.exec("DELETE FROM autofill WHERE id="+id);
    loadPasswords();
}

void AutoFillManager::removeAllPass()
{
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                         tr("Are you sure to delete all passwords on your computer?"), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.exec("DELETE FROM autofill");
    loadPasswords();
}

void AutoFillManager::editPass()
{
    QTreeWidgetItem* curItem = ui->treePass->currentItem();
    if (!curItem)
        return;
    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit password"), tr("Change password:"), QLineEdit::Normal, curItem->text(1), &ok);

    if (ok && !text.isEmpty()) {
        QSqlQuery query;
        query.exec("UPDATE autofill SET password='"+text+"' WHERE id="+curItem->whatsThis(1));
        loadPasswords();
    }
}

void AutoFillManager::removeExcept()
{
    QTreeWidgetItem* curItem = ui->treeExcept->currentItem();
    if (!curItem)
        return;
    QString id = curItem->whatsThis(0);
    QSqlQuery query;
    qDebug() << id;
    query.exec("DELETE FROM autofill_exceptions WHERE id="+id);
    loadPasswords();
}

void AutoFillManager::removeAllExcept()
{
    QSqlQuery query;
    query.exec("DELETE FROM autofill_exceptions");
    loadPasswords();
}

void AutoFillManager::showExceptions()
{
    ui->tabWidget->setCurrentIndex(1);
}

AutoFillManager::~AutoFillManager()
{
    delete ui;
}
