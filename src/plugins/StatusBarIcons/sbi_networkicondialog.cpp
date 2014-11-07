/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#include "sbi_networkicondialog.h"
#include "sbi_networkmanager.h"
#include "sbi_networkproxy.h"
#include "ui_sbi_networkicondialog.h"

#include <QInputDialog>
#include <QMessageBox>

SBI_NetworkIconDialog::SBI_NetworkIconDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SBI_NetworkIconDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    ui->addButton->setIcon(QIcon::fromTheme("document-new", QIcon(":sbi/data/add.png")));
    ui->removeButton->setIcon(QIcon::fromTheme("edit-delete", QIcon(":sbi/data/remove.png")));

    const QHash<QString, SBI_NetworkProxy*> &proxies = SBINetManager->proxies();

    QHashIterator<QString, SBI_NetworkProxy*> it(proxies);
    while (it.hasNext()) {
        it.next();
        ui->comboBox->addItem(it.key());
    }

    updateWidgets();
    showProxy(ui->comboBox->currentText());

    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addProxy()));
    connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(removeProxy()));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(showProxy(QString)));
    connect(ui->proxyButtonBox, SIGNAL(accepted()), this, SLOT(saveProxy()));
    connect(ui->closeButton, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
}

void SBI_NetworkIconDialog::addProxy()
{
    const QString name = QInputDialog::getText(this, tr("Add proxy"), tr("Name of proxy:"));
    if (name.isEmpty() || ui->comboBox->findText(name) > -1) {
        return;
    }

    ui->comboBox->addItem(name);
    ui->comboBox->setCurrentIndex(ui->comboBox->count() - 1);

    updateWidgets();
}

void SBI_NetworkIconDialog::removeProxy()
{
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Remove current proxy"), tr("Are you sure to remove current proxy?"),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (button != QMessageBox::Yes) {
        return;
    }

    int index = ui->comboBox->currentIndex();
    if (index < 0) {
        return;
    }

    SBINetManager->removeProxy(ui->comboBox->currentText());
    ui->comboBox->removeItem(index);

    updateWidgets();
}

void SBI_NetworkIconDialog::saveProxy()
{
    SBINetManager->saveProxy(ui->comboBox->currentText(), ui->proxyWidget->getProxy());
}

void SBI_NetworkIconDialog::showProxy(const QString &name)
{
    SBI_NetworkProxy* proxy = SBINetManager->proxies()[name];

    ui->proxyWidget->clear();

    if (proxy) {
        ui->proxyWidget->setProxy(*proxy);
    }
}

void SBI_NetworkIconDialog::updateWidgets()
{
    ui->removeButton->setEnabled(ui->comboBox->count() > 0);
    ui->noProxiesLabel->setVisible(ui->comboBox->count() == 0);
    ui->proxyWidget->setVisible(ui->comboBox->count() > 0);
}

SBI_NetworkIconDialog::~SBI_NetworkIconDialog()
{
    delete ui;
}
