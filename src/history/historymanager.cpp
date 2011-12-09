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
#include "historymanager.h"
#include "ui_historymanager.h"
#include "qupzilla.h"
#include "historymodel.h"
#include "iconprovider.h"
#include "browsinglibrary.h"
#include "globalfunctions.h"

HistoryManager::HistoryManager(QupZilla* mainClass, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HistoryManager)
    , p_QupZilla(mainClass)
    , m_historyModel(mApp->history())
{
    ui->setupUi(this);
    ui->historyTree->setDefaultItemShowMode(TreeWidget::ItemsCollapsed);
    ui->historyTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    qz_centerWidgetOnScreen(this);
    ui->deleteB->setShortcut(QKeySequence("Del"));

    connect(ui->historyTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*)));
    connect(ui->historyTree, SIGNAL(itemMiddleButtonClicked(QTreeWidgetItem*)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*)));

    connect(ui->deleteB, SIGNAL(clicked()), this, SLOT(deleteItem()));
    connect(ui->clearAll, SIGNAL(clicked()), this, SLOT(clearHistory()));
    connect(ui->historyTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));

    connect(m_historyModel, SIGNAL(historyEntryAdded(HistoryModel::HistoryEntry)), this, SLOT(historyEntryAdded(HistoryModel::HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyEntryDeleted(HistoryModel::HistoryEntry)), this, SLOT(historyEntryDeleted(HistoryModel::HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyEntryEdited(HistoryModel::HistoryEntry, HistoryModel::HistoryEntry)), this, SLOT(historyEntryEdited(HistoryModel::HistoryEntry, HistoryModel::HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyClear()), ui->historyTree, SLOT(clear()));

    connect(ui->optimizeDb, SIGNAL(clicked(QPoint)), this, SLOT(optimizeDb()));

    //QTimer::singleShot(0, this, SLOT(refreshTable()));

    ui->historyTree->setFocus();
}

QupZilla* HistoryManager::getQupZilla()
{
    if (!p_QupZilla.data()) {
        p_QupZilla = mApp->getWindow();
    }
    return p_QupZilla.data();
}

void HistoryManager::setMainWindow(QupZilla* window)
{
    if (window) {
        p_QupZilla = window;
    }
}

void HistoryManager::itemDoubleClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty()) {
        return;
    }

    QUrl url = QUrl::fromEncoded(item->text(1).toUtf8());
    getQupZilla()->tabWidget()->addView(url);
}

void HistoryManager::loadInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        getQupZilla()->tabWidget()->addView(action->data().toUrl(), tr("New Tab"), TabWidget::NewNotSelectedTab);
    }
}

void HistoryManager::contextMenuRequested(const QPoint &position)
{
    if (!ui->historyTree->itemAt(position)) {
        return;
    }
    QUrl link = QUrl::fromEncoded(ui->historyTree->itemAt(position)->text(1).toUtf8());
    if (link.isEmpty()) {
        return;
    }

    QMenu menu;
    menu.addAction(tr("Open link in actual tab"), getQupZilla(), SLOT(loadActionUrl()))->setData(link);
    menu.addAction(tr("Open link in new tab"), this, SLOT(loadInNewTab()))->setData(link);
    menu.addSeparator();

    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);
}

void HistoryManager::deleteItem()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    foreach(QTreeWidgetItem * item, ui->historyTree->selectedItems()) {
        if (!item) {
            return;
        }

        if (!item->parent()) {
            for (int i = 0; i < item->childCount(); i++) {
                QTreeWidgetItem* children = item->child(i);
                int id = children->whatsThis(1).toInt();
                m_historyModel->deleteHistoryEntry(id);
            }
            ui->historyTree->deleteItem(item);
        }
        else {
            int id = item->whatsThis(1).toInt();
            m_historyModel->deleteHistoryEntry(id);
        }
    }
    QApplication::restoreOverrideCursor();
}

void HistoryManager::historyEntryAdded(const HistoryModel::HistoryEntry &entry)
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
    item->setToolTip(0, entry.title);
    item->setToolTip(1, entry.url.toEncoded());

    item->setWhatsThis(1, QString::number(entry.id));
    item->setIcon(0, _iconForUrl(entry.url));
    ui->historyTree->prependToParentItem(parentItem, item);
}

void HistoryManager::historyEntryDeleted(const HistoryModel::HistoryEntry &entry)
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

void HistoryManager::historyEntryEdited(const HistoryModel::HistoryEntry &before, const HistoryModel::HistoryEntry &after)
{
    historyEntryDeleted(before);
    historyEntryAdded(after);
}

void HistoryManager::clearHistory()
{
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                                         tr("Are you sure to delete all history?"), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) {
        return;
    }

    m_historyModel->clearHistory();
    m_historyModel->optimizeHistory();
}

void HistoryManager::refreshTable()
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

        QTreeWidgetItem* item = new QTreeWidgetItem();
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
        item->setToolTip(0, title);
        item->setToolTip(1, url.toEncoded());

        item->setWhatsThis(1, QString::number(id));
        item->setIcon(0, _iconForUrl(url));
        ui->historyTree->addTopLevelItem(item);
    }

    ui->historyTree->setUpdatesEnabled(true);
}

void HistoryManager::search(const QString &searchText)
{
    ui->historyTree->filterString(searchText);
    return;
    if (searchText.isEmpty()) {
        refreshTable();
        return;
    }

    refreshTable();
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

void HistoryManager::optimizeDb()
{
    BrowsingLibrary* b = qobject_cast<BrowsingLibrary*>(parentWidget()->parentWidget());
    if (!b) {
        return;
    }
    b->optimizeDatabase();
}


HistoryManager::~HistoryManager()
{
    delete ui;
}
