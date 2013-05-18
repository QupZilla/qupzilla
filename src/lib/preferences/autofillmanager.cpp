/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "autofillmanager.h"
#include "ui_autofillmanager.h"
#include "autofill.h"
#include "passwordmanager.h"
#include "passwordbackends/passwordbackend.h"
#include "mainapplication.h"
#include "settings.h"

#include <QUrl>
#include <QMenu>
#include <QTimer>
#include <QSqlQuery>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>

AutoFillManager::AutoFillManager(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::AutoFillManager)
    , m_passwordsShown(false)
{
    ui->setupUi(this);

    connect(ui->removePass, SIGNAL(clicked()), this, SLOT(removePass()));
    connect(ui->removeAllPass, SIGNAL(clicked()), this, SLOT(removeAllPass()));
    connect(ui->editPass, SIGNAL(clicked()), this, SLOT(editPass()));
    connect(ui->showPasswords, SIGNAL(clicked()), this, SLOT(showPasswords()));
    connect(ui->search, SIGNAL(textChanged(QString)), ui->treePass, SLOT(filterString(QString)));
    connect(ui->changeBackend, SIGNAL(clicked()), this, SLOT(changePasswordBackend()));

    connect(ui->removeExcept, SIGNAL(clicked()), this, SLOT(removeExcept()));
    connect(ui->removeAllExcept, SIGNAL(clicked()), this, SLOT(removeAllExcept()));

    QMenu* menu = new QMenu(this);
    menu->addAction(tr("Import Passwords from File..."), this, SLOT(importPasswords()));
    menu->addAction(tr("Export Passwords to File..."), this, SLOT(exportPasswords()));
    ui->importExport->setMenu(menu);
    ui->importExport->setPopupMode(QToolButton::InstantPopup);
    ui->search->setPlaceholderText(tr("Search"));

    // Password backends
    ui->currentBackend->setText(QString("<b>%1</b>").arg(mApp->autoFill()->passwordManager()->activeBackend()->name()));

    // Load passwords
    QTimer::singleShot(0, this, SLOT(loadPasswords()));
}

void AutoFillManager::loadPasswords()
{
    ui->showPasswords->setText(tr("Show Passwords"));
    m_passwordsShown = false;

    QVector<PasswordEntry> allEntries = mApp->autoFill()->getAllFormData();

    ui->treePass->clear();
    foreach (const PasswordEntry &entry, allEntries) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treePass);
        item->setText(0, entry.host);
        item->setText(1, entry.username);
        item->setText(2, "*****");

        QVariant v;
        v.setValue<PasswordEntry>(entry);
        item->setData(0, Qt::UserRole + 10, v);
        ui->treePass->addTopLevelItem(item);
    }

    QSqlQuery query;
    query.exec("SELECT server, id FROM autofill_exceptions");
    ui->treeExcept->clear();
    while (query.next()) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeExcept);
        item->setText(0, query.value(0).toString());
        item->setData(0, Qt::UserRole + 10, query.value(1).toString());
        ui->treeExcept->addTopLevelItem(item);
    }

    ui->treePass->sortByColumn(-1);
    ui->treeExcept->sortByColumn(-1);
}

void AutoFillManager::changePasswordBackend()
{
    QHash<QString, PasswordBackend*> backends = mApp->autoFill()->passwordManager()->availableBackends();
    QStringList items;

    int current = 0;

    foreach (const QString &key, backends.keys()) {
        if (backends[key] == mApp->autoFill()->passwordManager()->activeBackend()) {
            current = items.size();
        }

        items << backends[key]->name();
    }

    QString item = QInputDialog::getItem(this, tr("Change backend..."), tr("Change backend:"), items, current, false);

    if (!item.isEmpty()) {
        Settings settings;
        settings.beginGroup("PasswordManager");

        PasswordBackend* backend = 0;
        foreach (const QString &key, backends.keys()) {
            if (backends[key]->name() == item) {
                backend = backends[key];
                settings.setValue("Backend", key);
                break;
            }
        }

        settings.endGroup();

        mApp->autoFill()->passwordManager()->switchBackend(backend);
    }

    QTimer::singleShot(0, this, SLOT(loadPasswords()));
}

void AutoFillManager::showPasswords()
{
    if (m_passwordsShown) {
        for (int i = 0; i < ui->treePass->topLevelItemCount(); i++) {
            QTreeWidgetItem* item = ui->treePass->topLevelItem(i);
            if (!item) {
                continue;
            }
            item->setText(2, "*****");
        }

        ui->showPasswords->setText(tr("Show Passwords"));
        m_passwordsShown = false;

        return;
    }

    m_passwordsShown = true;

    int result = QMessageBox::question(this, tr("Show Passwords"), tr("Are you sure that you want to show all passwords?"),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result != QMessageBox::Yes) {
        return;
    }

    for (int i = 0; i < ui->treePass->topLevelItemCount(); i++) {
        QTreeWidgetItem* item = ui->treePass->topLevelItem(i);
        if (!item) {
            continue;
        }

        item->setText(2, item->data(0, Qt::UserRole + 10).value<PasswordEntry>().password);
    }

    ui->showPasswords->setText(tr("Hide Passwords"));
}

void AutoFillManager::removePass()
{
    QTreeWidgetItem* curItem = ui->treePass->currentItem();
    if (!curItem) {
        return;
    }

    PasswordEntry entry = curItem->data(0, Qt::UserRole + 10).value<PasswordEntry>();
    mApp->autoFill()->removeEntry(entry);

    delete curItem;
}

void AutoFillManager::removeAllPass()
{
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                                         tr("Are you sure to delete all passwords on your computer?"), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) {
        return;
    }

    mApp->autoFill()->removeAllEntries();
    ui->treePass->clear();
}

void AutoFillManager::editPass()
{
    QTreeWidgetItem* curItem = ui->treePass->currentItem();
    if (!curItem) {
        return;
    }

    PasswordEntry entry = curItem->data(0, Qt::UserRole + 10).value<PasswordEntry>();

    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit password"), tr("Change password:"), QLineEdit::Normal, entry.password, &ok);

    if (ok && !text.isEmpty()) {
        QByteArray oldPass = "=" + QUrl::toPercentEncoding(entry.password);
        entry.data.replace(oldPass, "=" + QUrl::toPercentEncoding(text.toUtf8()));
        entry.password = text;

        QVariant v;
        v.setValue<PasswordEntry>(entry);
        curItem->setData(0, Qt::UserRole + 10, v);

        mApp->autoFill()->updateEntry(entry);

        if (m_passwordsShown) {
            curItem->setText(2, text);
        }
    }
}

void AutoFillManager::removeExcept()
{
    QTreeWidgetItem* curItem = ui->treeExcept->currentItem();
    if (!curItem) {
        return;
    }
    QString id = curItem->data(0, Qt::UserRole + 10).toString();
    QSqlQuery query;
    query.prepare("DELETE FROM autofill_exceptions WHERE id=?");
    query.addBindValue(id);
    query.exec();

    delete curItem;
}

void AutoFillManager::removeAllExcept()
{
    QSqlQuery query;
    query.exec("DELETE FROM autofill_exceptions");

    ui->treeExcept->clear();
}

void AutoFillManager::showExceptions()
{
    ui->tabWidget->setCurrentIndex(1);
}

void AutoFillManager::importPasswords()
{
    const QString &fileName = QFileDialog::getOpenFileName(this, tr("Choose file..."), QDir::homePath() + "/passwords.xml", "*.xml");
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        ui->importExportLabel->setText(tr("Cannot read file!"));
        return;
    }

    bool status = AutoFill::importPasswords(file.readAll());
    file.close();

    ui->importExportLabel->setText(status ? tr("Successfully imported") : tr("Error while importing!"));
    loadPasswords();
}

void AutoFillManager::exportPasswords()
{
    const QString &fileName = QFileDialog::getSaveFileName(this, tr("Choose file..."), QDir::homePath() + "/passwords.xml", "*.xml");
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        ui->importExportLabel->setText(tr("Cannot write to file!"));
        return;
    }

    file.write(AutoFill::exportPasswords());
    file.close();

    ui->importExportLabel->setText(tr("Successfully exported"));
}

AutoFillManager::~AutoFillManager()
{
    delete ui;
}
