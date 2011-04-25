#include "historysidebar.h"
#include "ui_historysidebar.h"
#include "qupzilla.h"
#include "historymodel.h"
#include "iconprovider.h"

HistorySideBar::HistorySideBar(QupZilla* mainClass, QWidget* parent) :
    QWidget(parent)
    ,ui(new Ui::HistorySideBar)
    ,p_QupZilla(mainClass)
    ,m_historyModel(mApp->history())
{
    ui->setupUi(this);
    connect(ui->historyTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this, SLOT(itemDoubleClicked(QTreeWidgetItem*)));
    connect(ui->historyTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));
    connect(ui->historyTree, SIGNAL(itemControlClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));
    connect(ui->search, SIGNAL(textEdited(QString)), this, SLOT(search()));

    connect(m_historyModel, SIGNAL(historyEntryAdded(HistoryModel::HistoryEntry)), this, SLOT(historyEntryAdded(HistoryModel::HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyEntryDeleted(HistoryModel::HistoryEntry)), this, SLOT(historyEntryDeleted(HistoryModel::HistoryEntry)));
    connect(m_historyModel, SIGNAL(historyClear()), ui->historyTree, SLOT(clear()));

    QTimer::singleShot(0, this, SLOT(refreshTable()));
}

void HistorySideBar::itemDoubleClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty())
        return;
    p_QupZilla->loadAddress(QUrl(item->text(1)));
}

void HistorySideBar::itemControlClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty())
        return;
    p_QupZilla->tabWidget()->addView(QUrl(item->text(1)));
}

void HistorySideBar::loadInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender()))
        p_QupZilla->tabWidget()->addView(action->data().toUrl(), tr("New Tab"), TabWidget::NewNotSelectedTab);
}

void HistorySideBar::contextMenuRequested(const QPoint &position)
{
    if (!ui->historyTree->itemAt(position))
        return;
    QString link = ui->historyTree->itemAt(position)->text(1);
    if (link.isEmpty())
        return;

    QMenu menu;
    menu.addAction(tr("Open link in actual tab"), p_QupZilla, SLOT(loadActionUrl()))->setData(link);
    menu.addAction(tr("Open link in new tab"), this, SLOT(loadInNewTab()))->setData(link);
    menu.addSeparator();
    menu.addAction(tr("Remove Entry"), this, SLOT(deleteItem()));

    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y()+1);
    menu.exec(p);
}

void HistorySideBar::deleteItem()
{
    QTreeWidgetItem* item = ui->historyTree->currentItem();
    if (!item)
        return;
    if (item->text(1).isEmpty())
        return;

    int id = item->whatsThis(1).toInt();
    m_historyModel->deleteHistoryEntry(id);
}

void HistorySideBar::historyEntryAdded(const HistoryModel::HistoryEntry &entry)
{
    QLocale locale(p_QupZilla->activeLanguage().remove(".qm"));

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

void HistorySideBar::historyEntryDeleted(const HistoryModel::HistoryEntry &entry)
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

void HistorySideBar::search()
{
    QString searchText = ui->search->text();
    refreshTable();

    if (searchText.isEmpty())
        return;

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

void HistorySideBar::refreshTable()
{
    ui->historyTree->setUpdatesEnabled(false);
    ui->historyTree->clear();

    QLocale locale(p_QupZilla->activeLanguage().remove(".qm"));

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
        //QString day = locale.dayName(date.toString("d").toInt());

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

HistorySideBar::~HistorySideBar()
{
    delete ui;
}
