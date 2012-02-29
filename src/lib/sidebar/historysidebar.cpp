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
#include "historysidebar.h"
#include "ui_historysidebar.h"
#include "qupzilla.h"
#include "tabwidget.h"
#include "mainapplication.h"
#include "historymodel.h"
#include "iconprovider.h"

#include <QMenu>
#include <QClipboard>
#include <QTimer>
#include <QSqlQuery>

HistorySideBar::HistorySideBar(QupZilla* mainClass, QWidget* parent)
    : QWidget(parent)
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

    connect(m_historyModel, SIGNAL(historyEntryAdded(HistoryEntry)), this, SLOT(historyEntryAdded(HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyEntryDeleted(HistoryEntry)), this, SLOT(historyEntryDeleted(HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyEntryEdited(HistoryEntry, HistoryEntry)), this, SLOT(historyEntryEdited(HistoryEntry, HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyClear()), ui->historyTree, SLOT(clear()));

    QTimer::singleShot(0, this, SLOT(slotRefreshTable()));
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
    p_QupZilla->tabWidget()->addView(url, item->text(0), Qz::NT_NotSelectedTab);
}

void HistorySideBar::loadInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        p_QupZilla->tabWidget()->addView(action->data().toUrl(), Qz::NT_NotSelectedTab);
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
    menu.addAction(tr("Open link in current tab"), p_QupZilla, SLOT(loadActionUrl()))->setData(link);
    menu.addAction(tr("Open link in new tab"), this, SLOT(loadInNewTab()))->setData(link);
    menu.addSeparator();
    menu.addAction(tr("Copy address"), this, SLOT(copyAddress()))->setData(link);

    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);
}

void HistorySideBar::historyEntryAdded(const HistoryEntry &entry)
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
    item->setToolTip(0, entry.url.toEncoded());

    item->setWhatsThis(1, QString::number(entry.id));
    item->setIcon(0, _iconForUrl(entry.url));
    ui->historyTree->prependToParentItem(parentItem, item);
}

void HistorySideBar::historyEntryDeleted(const HistoryEntry &entry)
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

void HistorySideBar::historyEntryEdited(const HistoryEntry &before, const HistoryEntry &after)
{
    historyEntryDeleted(before);
    historyEntryAdded(after);
}

void HistorySideBar::slotRefreshTable()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->historyTree->clear();

    QDate todayDate = QDate::currentDate();
    QDate startOfWeekDate = todayDate.addDays(1 - todayDate.dayOfWeek());
    QSqlQuery query;
    query.exec("SELECT title, url, id, date FROM history ORDER BY date DESC");

    int counter = 0;
    QHash<QString, QTreeWidgetItem*> hash;
    while (query.next()) {
        const QString &title = query.value(0).toString();
        const QUrl &url = query.value(1).toUrl();
        int id = query.value(2).toInt();
        const QDate &date = QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()).date();
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
        QTreeWidgetItem* findParent = hash[localDate];
        if (findParent) {
            item = new QTreeWidgetItem(findParent);
        }
        else {
            QTreeWidgetItem* newParent = new QTreeWidgetItem(ui->historyTree);
            newParent->setText(0, localDate);
            newParent->setIcon(0, QIcon(":/icons/menu/history_entry.png"));
            ui->historyTree->addTopLevelItem(newParent);
            hash[localDate] = newParent;

            item = new QTreeWidgetItem(newParent);
        }

        item->setText(0, title);
        item->setText(1, url.toEncoded());
        item->setToolTip(0, url.toEncoded());

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

HistorySideBar::~HistorySideBar()
{
    delete ui;
}
