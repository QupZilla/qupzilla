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
#include "adblocktreewidget.h"
#include "adblocksubscription.h"

#include <QMenu>
#include <QTimer>
#include <QInputDialog>

AdBlockTreeWidget::AdBlockTreeWidget(AdBlockSubscription* subscription, QWidget* parent)
    : TreeWidget(parent)
    , m_subscription(subscription)
    , m_topItem(0)
    , m_itemChangingBlock(false)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setDefaultItemShowMode(TreeWidget::ItemsExpanded);
    setHeaderHidden(true);
    setAlternatingRowColors(true);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(itemChanged(QTreeWidgetItem*)));
    connect(m_subscription, SIGNAL(subscriptionUpdated()), this, SLOT(subscriptionUpdated()));

    QTimer::singleShot(0, this, SLOT(refresh()));
}

AdBlockSubscription* AdBlockTreeWidget::subscription() const
{
    return m_subscription;
}

void AdBlockTreeWidget::contextMenuRequested(const QPoint &pos)
{
    if (!m_subscription->canEditRules()) {
        return;
    }

    QTreeWidgetItem* item = itemAt(pos);
    if (!item) {
        return;
    }

    QMenu menu;
    menu.addAction(tr("Add Rule"), this, SLOT(addRule()));
    menu.addSeparator();
    QAction* deleteAction = menu.addAction(tr("Remove Rule"), this, SLOT(removeRule()));

    if (!item->parent()) {
        deleteAction->setDisabled(true);
    }

    menu.exec(viewport()->mapToGlobal(pos));
}

void AdBlockTreeWidget::itemChanged(QTreeWidgetItem* item)
{
    if (!item || m_itemChangingBlock) {
        return;
    }

    m_itemChangingBlock = true;

    if (item->checkState(0) == Qt::Unchecked && !item->text(0).startsWith("!")) {
        // Disable rule
        int offset = item->data(0, Qt::UserRole + 10).toInt();
        QFont italicFont;
        italicFont.setItalic(true);
        item->setFont(0, italicFont);
        item->setText(0, item->text(0).prepend("!"));

        m_subscription->disableRule(offset);
    }
    else if (item->checkState(0) == Qt::Checked && item->text(0).startsWith("!")) {
        // Enable rule
        int offset = item->data(0, Qt::UserRole + 10).toInt();
        item->setFont(0, QFont());
        QString newText = item->text(0).mid(1);
        item->setText(0, newText);

        m_subscription->enableRule(offset);
    }
    else if (m_subscription->canEditRules()) {
        // Custom rule has been changed
        int offset = item->data(0, Qt::UserRole + 10).toInt();

        AdBlockRule rul(item->text(0));
        m_subscription->replaceRule(rul, offset);
    }

    m_itemChangingBlock = false;
}

void AdBlockTreeWidget::addRule()
{
    if (!m_subscription->canEditRules()) {
        return;
    }

    QString newRule = QInputDialog::getText(this, tr("Add Custom Rule"), tr("Please write your rule here:"));
    if (newRule.isEmpty()) {
        return;
    }

    int offset = m_subscription->addRule(AdBlockRule(newRule));

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, newRule);
    item->setData(0, Qt::UserRole + 10, offset);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, Qt::Checked);

    m_itemChangingBlock = true;
    m_topItem->addChild(item);
    m_itemChangingBlock = false;
}

void AdBlockTreeWidget::removeRule()
{
    QTreeWidgetItem* item = currentItem();
    if (!item || !m_subscription->canEditRules() || item == m_topItem) {
        return;
    }

    int offset = item->data(0, Qt::UserRole + 10).toInt();

    m_subscription->removeRule(offset);
    deleteItem(item);
}

void AdBlockTreeWidget::subscriptionUpdated()
{
    refresh();

    m_itemChangingBlock = true;
    m_topItem->setText(0, tr("%1 (recently updated)").arg(m_subscription->title()));
    m_itemChangingBlock = false;
}

void AdBlockTreeWidget::refresh()
{
    m_itemChangingBlock = true;
    clear();

    QFont boldFont;
    boldFont.setBold(true);
    QFont italicFont;
    italicFont.setItalic(true);

    m_topItem = new QTreeWidgetItem(this);
    m_topItem->setText(0, m_subscription->title());
    m_topItem->setFont(0, boldFont);
    addTopLevelItem(m_topItem);

    const QList<AdBlockRule> &allRules = m_subscription->allRules();

    int index = 0;
    foreach(const AdBlockRule & rule, allRules) {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_topItem);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, (rule.isEnabled()) ? Qt::Checked : Qt::Unchecked);
        item->setText(0, rule.filter());
        item->setData(0, Qt::UserRole + 10, index);

        if (m_subscription->canEditRules()) {
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }

        if (!rule.isEnabled()) {
            item->setFont(0, italicFont);
        }

        ++index;
    }

    expandAll();
    m_itemChangingBlock = false;
}
