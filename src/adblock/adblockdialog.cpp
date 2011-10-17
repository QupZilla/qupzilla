/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
/**
 * Copyright (c) 2009, Benjamin C. Meyer <ben@meyerhome.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "adblockdialog.h"
#include "adblockmanager.h"
#include "adblocksubscription.h"
#include "mainapplication.h"

AdBlockDialog::AdBlockDialog(QWidget *parent)
    : QDialog(parent)
    , m_itemChangingBlock(false)
    , m_manager(AdBlockManager::instance())
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupUi(this);
    adblockCheckBox->setChecked(m_manager->isEnabled());

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    treeWidget->setDefaultItemShowMode(TreeWidget::ItemsExpanded);

    connect(adblockCheckBox, SIGNAL(toggled(bool)), m_manager, SLOT(setEnabled(bool)));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addCustomRule()));
    connect(reloadButton, SIGNAL(clicked()), this, SLOT(updateSubscription()));
    connect(search, SIGNAL(textChanged(QString)), treeWidget, SLOT(filterString(QString)));
    connect(m_manager->subscription(), SIGNAL(changed()), this, SLOT(refreshAfterUpdate()));
    connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested()));

//    QTimer::singleShot(0, this, SLOT(firstRefresh()));
    firstRefresh();
}

void AdBlockDialog::editRule()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if (!item || !(item->flags() & Qt::ItemIsEditable))
        return;

    item->setSelected(true);
}

void AdBlockDialog::deleteRule()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if (!item)
        return;

    int offset = item->whatsThis(0).toInt();
    m_manager->subscription()->removeRule(offset);
    treeWidget->deleteItem(item);
    refresh();
}

void AdBlockDialog::customContextMenuRequested()
{
    QMenu menu;
    menu.addAction(tr("Add Rule"), this, SLOT(addCustomRule()));
    menu.addSeparator();
    menu.addAction(tr("Delete Rule"), this, SLOT(deleteRule()));
    menu.exec(QCursor::pos());
}

void AdBlockDialog::firstRefresh()
{
    refresh();
    connect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemChanged(QTreeWidgetItem*)));
}

void AdBlockDialog::refreshAfterUpdate()
{
    QMessageBox::information(this, tr("Update completed"), tr("EasyList has been successfuly updated."));
    refresh();
}

void AdBlockDialog::refresh()
{
    m_itemChangingBlock = true;
    treeWidget->setUpdatesEnabled(false);
    treeWidget->clear();

    QFont boldFont;
    boldFont.setBold(true);
    QFont italicFont;
    italicFont.setItalic(true);

    m_customRulesItem = new QTreeWidgetItem(treeWidget);
    m_customRulesItem->setText(0,tr("Custom Rules"));
    m_customRulesItem->setFont(0, boldFont);
    treeWidget->addTopLevelItem(m_customRulesItem);

    m_easyListItem = new QTreeWidgetItem(treeWidget);
    m_easyListItem->setText(0,"EasyList");
    m_easyListItem->setFont(0, boldFont);
    treeWidget->addTopLevelItem(m_easyListItem);

    bool customRulesStarted = false;
    QList<AdBlockRule> allRules = m_manager->subscription()->allRules();

    int index = 0;
    foreach (const AdBlockRule rule, allRules) {
        index++;
        if (rule.filter().contains("*******- user custom filters")) {
            customRulesStarted = true;
            continue;
        }
        QTreeWidgetItem* item = new QTreeWidgetItem(customRulesStarted ? m_customRulesItem : m_easyListItem);
        if (item->parent() == m_customRulesItem)
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, (rule.filter().startsWith("!") ) ? Qt::Unchecked : Qt::Checked);
        item->setText(0, rule.filter());
        item->setWhatsThis(0, QString::number(index-1));
        if (rule.filter().startsWith("!"))
            item->setFont(0, italicFont);
    }
    treeWidget->expandAll();
    treeWidget->setUpdatesEnabled(true);
    m_itemChangingBlock = false;
}

void AdBlockDialog::itemChanged(QTreeWidgetItem *item)
{
    if (!item || m_itemChangingBlock)
        return;

    m_itemChangingBlock = true;

    if (item->checkState(0) == Qt::Unchecked && !item->text(0).startsWith("!")) { //Disable rule
        int offset = item->whatsThis(0).toInt();
        QFont italicFont;
        italicFont.setItalic(true);
        item->setFont(0, italicFont);
        item->setText(0, item->text(0).prepend("!"));

        AdBlockRule rul(item->text(0));
        m_manager->subscription()->replaceRule(rul, offset);

    } else if (item->checkState(0) == Qt::Checked && item->text(0).startsWith("!")) { //Enable rule
        int offset = item->whatsThis(0).toInt();
        item->setFont(0, QFont());
        QString newText = item->text(0).mid(1);
        item->setText(0, newText);

        AdBlockRule rul(newText);
        m_manager->subscription()->replaceRule(rul, offset);

    } else { //Custom rule has been changed
        int offset = item->whatsThis(0).toInt();

        AdBlockRule rul(item->text(0));
        m_manager->subscription()->replaceRule(rul, offset);

    }

    m_itemChangingBlock = false;
}


void AdBlockDialog::addCustomRule()
{
    QString newRule = QInputDialog::getText(this, tr("Add Custom Rule"), tr("Please write your rule here:"));
    if (newRule.isEmpty())
        return;

    AdBlockSubscription* subscription = m_manager->subscription();
    int offset = subscription->addRule(AdBlockRule(newRule));
    m_itemChangingBlock = true;
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, newRule);
    item->setWhatsThis(0, QString::number(offset));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, Qt::Checked);
    treeWidget->appendToParentItem(m_customRulesItem, item);
    m_itemChangingBlock = false;
}

void AdBlockDialog::updateSubscription()
{
    AdBlockSubscription* subscription = m_manager->subscription();
    subscription->updateNow();
}
