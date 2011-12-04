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
#include "historysidebar.h"
#include "ui_historysidebar.h"
#include "qupzilla.h"
#include "historymodel.h"
#include "iconprovider.h"

HistorySideBar::HistorySideBar(QupZilla* mainClass, QWidget* parent) :
    QWidget(parent)
    , ui(new Ui::HistorySideBar)
    , p_QupZilla(mainClass)
    , m_historyModel(mApp->history())
{
    ui->setupUi(this);
    connect(ui->historyTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*)));
    connect(ui->historyTree, SIGNAL(itemControlClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));
    connect(ui->historyTree, SIGNAL(itemMiddleButtonClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));

    connect(ui->historyTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));
    connect(ui->search, SIGNAL(textEdited(QString)), ui->historyTree, SLOT(filterString(QString)));
//    connect(ui->search, SIGNAL(textEdited(QString)), this, SLOT(search()));

    connect(m_historyModel, SIGNAL(historyEntryAdded(HistoryModel::HistoryEntry)), this, SLOT(historyEntryAdded(HistoryModel::HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyEntryDeleted(HistoryModel::HistoryEntry)), this, SLOT(historyEntryDeleted(HistoryModel::HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyEntryEdited(HistoryModel::HistoryEntry, HistoryModel::HistoryEntry)), this, SLOT(historyEntryEdited(HistoryModel::HistoryEntry, HistoryModel::HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyClear()), ui->historyTree, SLOT(clear()));

    QTimer::singleShot(0, this, SLOT(refreshTable()));
}

void HistorySideBar::itemDoubleClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty()) {
        return;
    }

    QUrl url = QUrl::fromEncoded(item->text(1).toUtf8());
    p_QupZilla->loadAddress(url);
}

void HistorySideBar::itemControlClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty()) {
        return;
    }

    QUrl url = QUrl::fromEncoded(item->text(1).toUtf8());
    p_QupZilla->tabWidget()->addView(url);
}

void HistorySideBar::loadInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        p_QupZilla->tabWidget()->addView(action->data().toUrl(), tr("New Tab"), TabWidget::NewNotSelectedTab);
    }
}

void HistorySideBar::copyAddress()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QApplication::clipboard()->setText(action->data().toUrl().toEncoded());
    }
}

void HistorySideBar::contextMenuRequested(const QPoint &position)
{
    if (!ui->historyTree->itemAt(position)) {
        return;
    }

    QUrl link = QUrl::fromEncoded(ui->historyTree->itemAt(position)->text(1).toUtf8());
    if (link.isEmpty()) {
        return;
    }

    QMenu menu;
    menu.addAction(tr("Open link in actual tab"), p_QupZilla, SLOT(loadActionUrl()))->setData(link);
    menu.addAction(tr("Open link in new tab"), this, SLOT(loadInNewTab()))->setData(link);
    menu.addAction(tr("Copy address"), this, SLOT(copyAddress()))->setData(link);

    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);
}

void HistorySideBar::historyEntryAdded(const HistoryModel::HistoryEntry &entry)
{
    QDate todayDate = QDate::currentDate();
    QDate startOfWeekDate = todayDate.addDays(1 - todayDate.dayOfWeek());

    QDate date = entry.date.date();
    QString localDate;

    if (date == todayDate) {
        localDate = tr("Today");
    }
    else if (date >= startOfWeekDate) {
        localDate = tr("This Week");
    }
    else if (date.month() == todayDate.month()) {
        localDate = tr("This Month");
    }
    else {
        localDate = QString("%1 %2").arg(HistoryModel::titleCaseLocalizedMonth(date.month()), QString::number(date.year()));
    }

    QTreeWidgetItem* item = new QTreeWidgetItem();
    QTreeWidgetItem* parentItem;
    QList<QTreeWidgetItem*> findParent = ui->historyTree->findItems(localDate, 0);
    if (findParent.count() == 1) {
        parentItem = findParent.at(0);
    }
    else {
        parentItem = new QTreeWidgetItem();
        parentItem->setText(0, localDate);
        parentItem->setIcon(0, QIcon(":/icons/menu/history_entry.png"));
        ui->historyTree->addTopLevelItem(parentItem);
    }

    item->setText(0, entry.title);
    item->setText(1, entry.url.toEncoded());
    item->setToolTip(0, entry.url.toEncoded());

    item->setWhatsThis(1, QString::number(entry.id));
    item->setIcon(0, _iconForUrl(entry.url));
    ui->historyTree->prependToParentItem(parentItem, item);
}

void HistorySideBar::historyEntryDeleted(const HistoryModel::HistoryEntry &entry)
{
    QList<QTreeWidgetItem*> list = ui->historyTree->allItems();
    foreach(QTreeWidgetItem * item, list) {
        if (!item) {
            continue;
        }
        if (item->whatsThis(1).toInt() != entry.id) {
            continue;
        }
        ui->historyTree->deleteItem(item);
        return;
    }
}

void HistorySideBar::historyEntryEdited(const HistoryModel::HistoryEntry &before, const HistoryModel::HistoryEntry &after)
{
    historyEntryDeleted(before);
    historyEntryAdded(after);
}

void HistorySideBar::search()
{
    QString searchText = ui->search->text();
    refreshTable();

    if (searchText.isEmpty()) {
        return;
    }

    ui->historyTree->setUpdatesEnabled(false);

    QList<QTreeWidgetItem*> items = ui->historyTree->findItems("*" + searchText + "*", Qt::MatchRecursive | Qt::MatchWildcard);

    QList<QTreeWidgetItem*> foundItems;
    foreach(QTreeWidgetItem * fitem, items) {
        if (fitem->text(1).isEmpty()) {
            continue;
        }
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, fitem->text(0));
        item->setText(1, fitem->text(1));
        item->setWhatsThis(1, fitem->whatsThis(1));
        item->setIcon(0, _iconForUrl(fitem->text(1)));
        foundItems.append(item);
    }
    ui->historyTree->clear();
    ui->historyTree->addTopLevelItems(foundItems);
    ui->historyTree->setUpdatesEnabled(true);
}

void HistorySideBar::refreshTable()
{
    ui->historyTree->setUpdatesEnabled(false);
    ui->historyTree->clear();

    QDate todayDate = QDate::currentDate();
    QDate startOfWeekDate = todayDate.addDays(1 - todayDate.dayOfWeek());
    QSqlQuery query;
    query.exec("SELECT title, url, id, date FROM history ORDER BY date DESC");

    while (query.next()) {
        QString title = query.value(0).toString();
        QUrl url = query.value(1).toUrl();
        int id = query.value(2).toInt();
        QDate date = QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()).date();
        QString localDate;

        if (date == todayDate) {
            localDate = tr("Today");
        }
        else if (date >= startOfWeekDate) {
            localDate = tr("This Week");
        }
        else if (date.month() == todayDate.month()) {
            localDate = tr("This Month");
        }
        else {
            localDate = QString("%1 %2").arg(HistoryModel::titleCaseLocalizedMonth(date.month()), QString::number(date.year()));
        }

        QTreeWidgetItem* item;
        QList<QTreeWidgetItem*> findParent = ui->historyTree->findItems(localDate, 0);
        if (findParent.count() == 1) {
            item = new QTreeWidgetItem(findParent.at(0));
        }
        else {
            QTreeWidgetItem* newParent = new QTreeWidgetItem(ui->historyTree);
            newParent->setText(0, localDate);
            newParent->setIcon(0, QIcon(":/icons/menu/history_entry.png"));
            ui->historyTree->addTopLevelItem(newParent);
            item = new QTreeWidgetItem(newParent);
        }

        item->setText(0, title);
        item->setText(1, url.toEncoded());
        item->setToolTip(0, url.toEncoded());

        item->setWhatsThis(1, QString::number(id));
        item->setIcon(0, _iconForUrl(url));
        ui->historyTree->addTopLevelItem(item);
    }

    ui->historyTree->setUpdatesEnabled(true);
}

HistorySideBar::~HistorySideBar()
{
    delete ui;
}
