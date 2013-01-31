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
#include "autofill.h"
#include "ui_autofillmanager.h"

#include <QMenu>
#include <QTimer>
#include <QSqlQuery>
#include <QMessageBox>
#include <QInputDialog>
#include <QUrl>
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

    connect(ui->removeExcept, SIGNAL(clicked()), this, SLOT(removeExcept()));
    connect(ui->removeAllExcept, SIGNAL(clicked()), this, SLOT(removeAllExcept()));

    QMenu* menu = new QMenu(this);
    menu->addAction(tr("Import Passwords from File..."), this, SLOT(importPasswords()));
    menu->addAction(tr("Export Passwords to File..."), this, SLOT(exportPasswords()));
    ui->importExport->setMenu(menu);
    ui->importExport->setPopupMode(QToolButton::InstantPopup);
    ui->search->setPlaceholderText(tr("Search"));

    QTimer::singleShot(0, this, SLOT(loadPasswords()));
}

void AutoFillManager::loadPasswords()
{
    ui->showPasswords->setText(tr("Show Passwords"));
    m_passwordsShown = false;

    QSqlQuery query;
    query.exec("SELECT server, username, password, id FROM autofill");
    ui->treePass->clear();

    while (query.next()) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treePass);
        item->setText(0, query.value(0).toString());
        item->setText(1, query.value(1).toString());
        item->setText(2, "*****");
        item->setData(0, Qt::UserRole + 10, query.value(3).toString());
        item->setData(0, Qt::UserRole + 11, query.value(2).toString());
        ui->treePass->addTopLevelItem(item);
    }

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

        item->setText(2, item->data(0, Qt::UserRole + 11).toString());
    }

    ui->showPasswords->setText(tr("Hide Passwords"));
}

void AutoFillManager::removePass()
{
    QTreeWidgetItem* curItem = ui->treePass->currentItem();
    if (!curItem) {
        return;
    }
    QString id = curItem->data(0, Qt::UserRole + 10).toString();
    QSqlQuery query;
    query.prepare("DELETE FROM autofill WHERE id=?");
    query.addBindValue(id);
    query.exec();

    delete curItem;
}

void AutoFillManager::removeAllPass()
{
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                                         tr("Are you sure to delete all passwords on your computer?"), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query;
    query.exec("DELETE FROM autofill");

    ui->treePass->clear();
}

void AutoFillManager::editPass()
{
    QTreeWidgetItem* curItem = ui->treePass->currentItem();
    if (!curItem) {
        return;
    }
    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit password"), tr("Change password:"), QLineEdit::Normal, curItem->data(0, Qt::UserRole + 11).toString(), &ok);

    if (ok && !text.isEmpty()) {
        QSqlQuery query;
        query.prepare("SELECT data, password FROM autofill WHERE id=?");
        query.addBindValue(curItem->data(0, Qt::UserRole + 10).toString());
        query.exec();
        query.next();

        QByteArray data = query.value(0).toByteArray();
        QByteArray oldPass = "=" + QUrl::toPercentEncoding(query.value(1).toByteArray());
        data.replace(oldPass, "=" + QUrl::toPercentEncoding(text.toUtf8()));

        query.prepare("UPDATE autofill SET data=?, password=? WHERE id=?");
        query.bindValue(0, data);
        query.bindValue(1, text);
        query.bindValue(2, curItem->data(0, Qt::UserRole + 10).toString());
        query.exec();

        if (m_passwordsShown) {
            curItem->setText(2, text);
        }

        curItem->setData(0, Qt::UserRole + 11, text);
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
