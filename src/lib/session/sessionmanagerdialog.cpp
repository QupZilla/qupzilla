/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2017 David Rosca <nowrep@gmail.com>
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
#include "sessionmanagerdialog.h"
#include "ui_sessionmanagerdialog.h"
#include "mainapplication.h"
#include "sessionmanager.h"
#include "removeitemfocusdelegate.h"

#include <QFileInfo>
#include <QDateTime>

SessionManagerDialog::SessionManagerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SessionManagerDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->treeWidget->setItemDelegate(new RemoveItemFocusDelegate(ui->treeWidget));

    connect(ui->newButton, &QPushButton::clicked, this, &SessionManagerDialog::newSession);
    connect(ui->renameButton, &QPushButton::clicked, this, &SessionManagerDialog::renameSession);
    connect(ui->cloneButton, &QPushButton::clicked, this, &SessionManagerDialog::cloneSession);
    connect(ui->deleteButton, &QPushButton::clicked, this, &SessionManagerDialog::deleteSession);
    connect(ui->switchToButton, &QPushButton::clicked, this, &SessionManagerDialog::switchToSession);
    connect(ui->treeWidget, &QTreeWidget::currentItemChanged, this, &SessionManagerDialog::updateButtons);

    refresh();
    connect(mApp->sessionManager(), &SessionManager::sessionsMetaDataChanged, this, &SessionManagerDialog::refresh);
}

SessionManagerDialog::~SessionManagerDialog()
{
    delete ui;
}

void SessionManagerDialog::newSession()
{
    mApp->sessionManager()->newSession();
}

void SessionManagerDialog::renameSession()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (!item) {
        return;
    }
    const QString filePath = item->data(0, SessionFileRole).toString();
    if (!filePath.isEmpty()) {
        mApp->sessionManager()->renameSession(filePath);
    }
}

void SessionManagerDialog::cloneSession()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (!item) {
        return;
    }
    const QString filePath = item->data(0, SessionFileRole).toString();
    if (!filePath.isEmpty()) {
        mApp->sessionManager()->cloneSession(filePath);
    }
}

void SessionManagerDialog::deleteSession()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (!item) {
        return;
    }
    const QString filePath = item->data(0, SessionFileRole).toString();
    if (!filePath.isEmpty()) {
        mApp->sessionManager()->deleteSession(filePath);
    }
}

void SessionManagerDialog::switchToSession()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (!item) {
        return;
    }
    const QString filePath = item->data(0, SessionFileRole).toString();
    if (!filePath.isEmpty()) {
        if (item->data(0, IsBackupSessionRole).toBool()) {
            mApp->sessionManager()->replaceSession(filePath);
        } else {
            mApp->sessionManager()->switchToSession(filePath);
        }
    }
}

void SessionManagerDialog::refresh()
{
    ui->treeWidget->clear();

    const auto sessions = mApp->sessionManager()->sessionMetaData();
    for (const auto &session : sessions) {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, session.name);
        item->setText(1, QFileInfo(session.filePath).lastModified().toString(Qt::DefaultLocaleShortDate));
        item->setData(0, SessionFileRole, session.filePath);
        item->setData(0, IsBackupSessionRole, session.isBackup);
        item->setData(0, IsActiveSessionRole, session.isActive);
        item->setData(0, IsDefaultSessionRole, session.isDefault);
        updateItem(item);
        ui->treeWidget->addTopLevelItem(item);
    }

    updateButtons();
}

void SessionManagerDialog::updateButtons()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    const bool isBackup = item && item->data(0, IsBackupSessionRole).toBool();
    const bool isActive = item && item->data(0, IsActiveSessionRole).toBool();
    const bool isDefault = item && item->data(0, IsDefaultSessionRole).toBool();

    ui->renameButton->setEnabled(item && !isDefault && !isBackup);
    ui->cloneButton->setEnabled(item && !isBackup);
    ui->deleteButton->setEnabled(item && !isBackup && !isDefault && !isActive);
    ui->switchToButton->setEnabled(item && !isActive);
    ui->switchToButton->setText(isBackup ? tr("Restore") : tr("Switch To"));
}

void SessionManagerDialog::updateItem(QTreeWidgetItem *item)
{
    const bool isBackup = item->data(0, IsBackupSessionRole).toBool();
    const bool isActive = item->data(0, IsActiveSessionRole).toBool();
    const bool isDefault = item->data(0, IsDefaultSessionRole).toBool();

    QFont font = item->font(0);

    if (isBackup) {
        const QColor color = palette().color(QPalette::Disabled, QPalette::WindowText);
        item->setForeground(0, color);
        item->setForeground(1, color);
    }

    if (isActive) {
        font.setBold(true);
        item->setFont(0, font);
        item->setFont(1, font);
    }

    if (isDefault) {
        font.setItalic(true);
        item->setFont(0, font);
        item->setFont(1, font);
    }
}

void SessionManagerDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    resizeViewHeader();
}

void SessionManagerDialog::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    resizeViewHeader();
}

void SessionManagerDialog::resizeViewHeader()
{
    const int headerWidth = ui->treeWidget->header()->width();
    ui->treeWidget->header()->resizeSection(0, headerWidth - headerWidth / 2.5);
}
