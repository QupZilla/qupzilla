/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "qtwin.h"
#include "historymodel.h"
#include "iconprovider.h"

HistoryManager::HistoryManager(QupZilla* mainClass, QWidget* parent) :
    QWidget(parent)
    ,ui(new Ui::HistoryManager)
    ,p_QupZilla(mainClass)
    ,m_historyModel(mApp->history())
{
    ui->setupUi(this);
    //CENTER on scren
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect &size = QWidget::geometry();
    QWidget::move( (screen.width()-size.width())/2, (screen.height()-size.height())/2 );

#ifdef Q_WS_WIN
    if (QtWin::isCompositionEnabled()) {
        QtWin::extendFrameIntoClientArea(this);
        ui->gridLayout->setContentsMargins(0, 0, 0, 0);
    }
#endif

    connect(ui->historyTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this, SLOT(itemDoubleClicked(QTreeWidgetItem*)));
    connect(ui->close, SIGNAL(clicked(QAbstractButton*)), this, SLOT(hide()));
    connect(ui->deleteB, SIGNAL(clicked()), this, SLOT(deleteItem()));
    connect(ui->clearAll, SIGNAL(clicked()), this, SLOT(clearHistory()));
//    connect(ui->search, SIGNAL(textChanged(QString)), ui->historyTree, SLOT(filterStringWithoutTopItems(QString)));
    connect(ui->search, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(search()));
    connect(ui->historyTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));
    connect(ui->historyTree, SIGNAL(itemControlClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));

    connect(m_historyModel, SIGNAL(historyEntryAdded(HistoryModel::HistoryEntry)), this, SLOT(historyEntryAdded(HistoryModel::HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyEntryDeleted(HistoryModel::HistoryEntry)), this, SLOT(historyEntryDeleted(HistoryModel::HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyClear()), ui->historyTree, SLOT(clear()));

    //QTimer::singleShot(0, this, SLOT(refreshTable()));

    ui->search->setInactiveText(tr("Search"));
}

QupZilla* HistoryManager::getQupZilla()
{
    if (!p_QupZilla)
        p_QupZilla = mApp->getWindow();
    return p_QupZilla;
}

void HistoryManager::setMainWindow(QupZilla* window)
{
    if (window)
        p_QupZilla = window;
}

void HistoryManager::itemDoubleClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty())
        return;
    getQupZilla()->loadAddress(QUrl(item->text(1)));
}

void HistoryManager::itemControlClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty())
        return;
    getQupZilla()->tabWidget()->addView(QUrl(item->text(1)));
}

void HistoryManager::loadInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender()))
        getQupZilla()->tabWidget()->addView(action->data().toUrl(), tr("New Tab"), TabWidget::NewNotSelectedTab);
}

void HistoryManager::contextMenuRequested(const QPoint &position)
{
    if (!ui->historyTree->itemAt(position))
        return;
    QString link = ui->historyTree->itemAt(position)->text(1);
    if (link.isEmpty())
        return;

    QMenu menu;
    menu.addAction(tr("Open link in actual tab"), getQupZilla(), SLOT(loadActionUrl()))->setData(link);
    menu.addAction(tr("Open link in new tab"), this, SLOT(loadInNewTab()))->setData(link);
    menu.addSeparator();

    menu.addSeparator();
    menu.addAction(tr("Close"), this, SLOT(close()));

    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y()+1);
    menu.exec(p);
}

void HistoryManager::deleteItem()
{
    QTreeWidgetItem* item = ui->historyTree->currentItem();
    if (!item || item->text(1).isEmpty())
        return;

    int id = item->whatsThis(1).toInt();
    m_historyModel->deleteHistoryEntry(id);
}

void HistoryManager::historyEntryAdded(const HistoryModel::HistoryEntry &entry)
{
    QLocale locale(getQupZilla()->activeLanguage().remove(".qm"));

    QString localDate; //date.toString("dddd d. MMMM yyyy");
    QString month = locale.monthName(entry.date.toString("M").toInt());
    localDate =  entry.date.toString(" d. ") + month + entry.date.toString(" yyyy");

    QTreeWidgetItem* item;
    QList<QTreeWidgetItem*> findParent = ui->historyTree->findItems(localDate, 0);
    if (findParent.count() == 1) {
        item = new QTreeWidgetItem(findParent.at(0));
    } else {
        QTreeWidgetItem* newParent = new QTreeWidgetItem(ui->historyTree);
        newParent->setText(0, localDate);
        newParent->setIcon(0, QIcon(":/icons/menu/history_entry.png"));
        ui->historyTree->addTopLevelItem(newParent);
        item = new QTreeWidgetItem(newParent);
    }

    item->setText(0, entry.title);
    item->setText(1, entry.url.toEncoded());
    item->setToolTip(0, entry.title);
    item->setToolTip(1, entry.url.toEncoded());

    item->setWhatsThis(1, QString::number(entry.id));
    item->setIcon(0, _iconForUrl(entry.url));
    ui->historyTree->addTopLevelItem(item);
}

void HistoryManager::historyEntryDeleted(const HistoryModel::HistoryEntry &entry)
{
    QList<QTreeWidgetItem*> list = ui->historyTree->allItems();
    foreach (QTreeWidgetItem* item, list) {
        if (!item)
            continue;
        if (item->whatsThis(1).toInt() != entry.id)
            continue;
        delete item;
        return;
    }
}

void HistoryManager::clearHistory()
{
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                         tr("Are you sure to delete all history?"), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes)
        return;

    m_historyModel->clearHistory();
    m_historyModel->optimizeHistory();
}

void HistoryManager::refreshTable()
{
    ui->historyTree->setUpdatesEnabled(false);
    ui->historyTree->clear();

    QLocale locale(getQupZilla()->activeLanguage().remove(".qm"));

    QSqlQuery query;
    query.exec("SELECT title, url, id, date FROM history ORDER BY date DESC");

    while(query.next()) {
        QString title = query.value(0).toString();
        QUrl url = query.value(1).toUrl();
        int id = query.value(2).toInt();
        qint64 unixDate = query.value(3).toLongLong();
        QDateTime date = QDateTime();
        date = date.fromMSecsSinceEpoch(unixDate);

        QString localDate; //date.toString("dddd d. MMMM yyyy");
        QString month = locale.monthName(date.toString("M").toInt());
        localDate =  date.toString(" d. ") + month + date.toString(" yyyy");

        QTreeWidgetItem* item;
        QList<QTreeWidgetItem*> findParent = ui->historyTree->findItems(localDate, 0);
        if (findParent.count() == 1) {
            item = new QTreeWidgetItem(findParent.at(0));
        }else{
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

void HistoryManager::search()
{
    QString searchText = ui->search->text();
    if (searchText.isEmpty()) {
        refreshTable();
        return;
    }

    refreshTable();
    ui->historyTree->setUpdatesEnabled(false);

    QList<QTreeWidgetItem*> items = ui->historyTree->findItems("*"+searchText+"*", Qt::MatchRecursive | Qt::MatchWildcard);

    QList<QTreeWidgetItem*> foundItems;
    foreach(QTreeWidgetItem* fitem, items) {
        if (fitem->text(1).isEmpty())
            continue;
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

HistoryManager::~HistoryManager()
{
    delete ui;
}
