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
#include "historymanager.h"
#include "ui_historymanager.h"
#include "qupzilla.h"
#include "mainapplication.h"
#include "historymodel.h"
#include "iconprovider.h"
#include "browsinglibrary.h"
#include "globalfunctions.h"
#include "tabwidget.h"

HistoryManager::HistoryManager(QupZilla* mainClass, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HistoryManager)
    , p_QupZilla(mainClass)
    , m_historyModel(mApp->history())
{
    ui->setupUi(this);
    ui->historyTree->setDefaultItemShowMode(TreeWidget::ItemsCollapsed);
    ui->historyTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->deleteB->setShortcut(QKeySequence("Del"));

    connect(ui->historyTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*)));
    connect(ui->historyTree, SIGNAL(itemMiddleButtonClicked(QTreeWidgetItem*)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*)));

    connect(ui->deleteB, SIGNAL(clicked()), this, SLOT(deleteItem()));
    connect(ui->clearAll, SIGNAL(clicked()), this, SLOT(clearHistory()));
    connect(ui->historyTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));

    connect(m_historyModel, SIGNAL(historyEntryAdded(HistoryEntry)), this, SLOT(historyEntryAdded(HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyEntryDeleted(HistoryEntry)), this, SLOT(historyEntryDeleted(HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyEntryEdited(HistoryEntry, HistoryEntry)), this, SLOT(historyEntryEdited(HistoryEntry, HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyClear()), ui->historyTree, SLOT(clear()));

    connect(ui->optimizeDb, SIGNAL(clicked(QPoint)), this, SLOT(optimizeDb()));

    //QTimer::singleShot(0, this, SLOT(refreshTable()));

    ui->historyTree->setFocus();
}

QupZilla* HistoryManager::getQupZilla()
{
    if (!p_QupZilla) {
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
    getQupZilla()->tabWidget()->addView(url, item->text(0));
}

void HistoryManager::loadInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        getQupZilla()->tabWidget()->addView(action->data().toUrl(), Qz::NT_NotSelectedTab);
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
    menu.addAction(tr("Open link in current tab"), getQupZilla(), SLOT(loadActionUrl()))->setData(link);
    menu.addAction(tr("Open link in new tab"), this, SLOT(loadInNewTab()))->setData(link);
    menu.addSeparator();
    menu.addAction(tr("Copy address"), this, SLOT(copyUrl()))->setData(link);

    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);
}

void HistoryManager::copyUrl()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QApplication::clipboard()->setText(action->data().toUrl().toEncoded());
    }
}

void HistoryManager::deleteItem()
{
    foreach(QTreeWidgetItem * item, ui->historyTree->selectedItems()) {
        if (!item) {
            return;
        }

        if (!item->parent()) {
            for (int i = 0; i < item->childCount(); i++) {
                QTreeWidgetItem* children = item->child(i);
                int id = children->whatsThis(1).toInt();
                m_historyModel->deleteHistoryEntry(id);

                ui->historyTree->deleteItem(children);
                m_ignoredIds.append(id);
            }
            ui->historyTree->deleteItem(item);
        }
        else {
            int id = item->whatsThis(1).toInt();
            m_historyModel->deleteHistoryEntry(id);

            ui->historyTree->deleteItem(item);
            m_ignoredIds.append(id);
        }
    }
}

void HistoryManager::historyEntryAdded(const HistoryEntry &entry)
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
        ui->historyTree->insertTopLevelItem(0, parentItem);
    }

    item->setText(0, entry.title);
    item->setText(1, entry.url.toEncoded());
    item->setToolTip(0, entry.title);
    item->setToolTip(1, entry.url.toEncoded());

    item->setWhatsThis(1, QString::number(entry.id));
    item->setIcon(0, _iconForUrl(entry.url));
    ui->historyTree->prependToParentItem(parentItem, item);
}

void HistoryManager::historyEntryDeleted(const HistoryEntry &entry)
{
    if (m_ignoredIds.contains(entry.id)) {
        m_ignoredIds.removeOne(entry.id);
        return;
    }

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

void HistoryManager::historyEntryEdited(const HistoryEntry &before, const HistoryEntry &after)
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
    QTimer::singleShot(0, this, SLOT(slotRefreshTable()));
}

void HistoryManager::slotRefreshTable()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->historyTree->clear();

    QDate todayDate = QDate::currentDate();
    QDate startOfWeekDate = todayDate.addDays(1 - todayDate.dayOfWeek());
    QSqlQuery query;
    query.exec("SELECT title, url, id, date FROM history ORDER BY date DESC");

    int counter = 0;
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

        ++counter;
        if (counter > 200) {
            QApplication::processEvents();
            counter = 0;
        }
    }

    QApplication::restoreOverrideCursor();
}

void HistoryManager::search(const QString &searchText)
{
    ui->historyTree->filterString(searchText);
//    if (searchText.isEmpty()) {
//        refreshTable();
//        return;
//    }

//    refreshTable();
//    ui->historyTree->setUpdatesEnabled(false);

//    QList<QTreeWidgetItem*> items = ui->historyTree->findItems("*" + searchText + "*", Qt::MatchRecursive | Qt::MatchWildcard);

//    QList<QTreeWidgetItem*> foundItems;
//    foreach(QTreeWidgetItem * fitem, items) {
//        if (fitem->text(1).isEmpty()) {
//            continue;
//        }
//        QTreeWidgetItem* item = new QTreeWidgetItem();
//        item->setText(0, fitem->text(0));
//        item->setText(1, fitem->text(1));
//        item->setWhatsThis(1, fitem->whatsThis(1));
//        item->setIcon(0, _iconForUrl(fitem->text(1)));
//        foundItems.append(item);
//    }
//    ui->historyTree->clear();
//    ui->historyTree->addTopLevelItems(foundItems);
//    ui->historyTree->setUpdatesEnabled(true);
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
