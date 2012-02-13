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
#include "cookiemanager.h"
#include "ui_cookiemanager.h"
#include "qupzilla.h"
#include "cookiejar.h"
#include "mainapplication.h"
#include "globalfunctions.h"

CookieManager::CookieManager(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CookieManager)
    , m_refreshCookieJar(true)
{
    setWindowModality(Qt::WindowModal);

    ui->setupUi(this);
    qz_centerWidgetOnScreen(this);

    connect(ui->cookieTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
    connect(ui->removeAll, SIGNAL(clicked()), this, SLOT(removeAll()));
    connect(ui->removeOne, SIGNAL(clicked()), this, SLOT(removeCookie()));
    connect(ui->close, SIGNAL(clicked(QAbstractButton*)), this, SLOT(hide()));
    connect(ui->search, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->search, SIGNAL(textChanged(QString)), ui->cookieTree, SLOT(filterString(QString)));

    ui->search->setInactiveText(tr("Search"));
    ui->cookieTree->setDefaultItemShowMode(TreeWidget::ItemsCollapsed);
    ui->cookieTree->sortItems(0, Qt::AscendingOrder);

    QShortcut* removeShortcut = new QShortcut(QKeySequence("Del"), this);
    connect(removeShortcut, SIGNAL(activated()), this, SLOT(removeCookie()));
}

void CookieManager::removeAll()
{
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                                         tr("Are you sure to delete all cookies on your computer?"), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) {
        return;
    }

    QList<QNetworkCookie> emptyList;
    mApp->cookieJar()->setAllCookies(emptyList);
    ui->cookieTree->clear();
}

void CookieManager::removeCookie()
{
    QTreeWidgetItem* current = ui->cookieTree->currentItem();
    if (!current) {
        return;
    }

    QList<QNetworkCookie> allCookies = mApp->cookieJar()->getAllCookies();

    int indexToNavigate = -1;

    if (current->text(1).isEmpty()) {     //Remove whole cookie group
        QString domain = current->whatsThis(0);
        foreach(const QNetworkCookie & cookie, allCookies) {
            if (cookie.domain() == domain || cookie.domain()  ==  domain.mid(1)) {
                allCookies.removeOne(cookie);
            }
        }

        indexToNavigate = ui->cookieTree->indexOfTopLevelItem(current) - 1;
        ui->cookieTree->deleteItem(current);
    }
    else {
        const QNetworkCookie &cookie = qvariant_cast<QNetworkCookie>(current->data(0, Qt::UserRole + 10));
        allCookies.removeOne(cookie);

        indexToNavigate = ui->cookieTree->indexOfTopLevelItem(current->parent());
        ui->cookieTree->deleteItem(current);
    }

    mApp->cookieJar()->setAllCookies(allCookies);

    if (indexToNavigate > 0 && ui->cookieTree->topLevelItemCount() >= indexToNavigate) {
        QTreeWidgetItem* scrollItem = ui->cookieTree->topLevelItem(indexToNavigate);
        if (scrollItem) {
            ui->cookieTree->setCurrentItem(scrollItem);
            ui->cookieTree->scrollToItem(scrollItem);
        }
    }

    if (!ui->search->text().isEmpty()) {
        search();
    }
}

void CookieManager::currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* parent)
{
    Q_UNUSED(parent);
    if (!current) {
        return;
    }

    if (current->text(1).isEmpty()) {
        ui->name->setText(tr("<cookie not selected>"));
        ui->value->setText(tr("<cookie not selected>"));
        ui->server->setText(tr("<cookie not selected>"));
        ui->path->setText(tr("<cookie not selected>"));
        ui->secure->setText(tr("<cookie not selected>"));
        ui->expiration->setText(tr("<cookie not selected>"));

        ui->removeOne->setText(tr("Remove cookies"));
        return;
    }

    const QNetworkCookie &cookie = qvariant_cast<QNetworkCookie>(current->data(0, Qt::UserRole + 10));

    ui->name->setText(cookie.name());
    ui->value->setText(cookie.value());
    ui->server->setText(cookie.domain());
    ui->path->setText(cookie.path());
    cookie.isSecure() ? ui->secure->setText(tr("Secure only")) : ui->secure->setText(tr("All connections"));
    cookie.isSessionCookie() ? ui->expiration->setText(tr("Session cookie")) : ui->expiration->setText(QDateTime(cookie.expirationDate()).toString("hh:mm:ss dddd d. MMMM yyyy"));

    ui->removeOne->setText(tr("Remove cookie"));
}

void CookieManager::refreshTable()
{
    QTimer::singleShot(0, this, SLOT(slotRefreshTable()));
}

void CookieManager::slotRefreshTable()
{
    const QList<QNetworkCookie>& allCookies = mApp->cookieJar()->getAllCookies();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->cookieTree->clear();

    int counter = 0;
    QString cookieDomain;
    for (int i = 0; i < allCookies.count(); ++i) {
        const QNetworkCookie& cookie = allCookies.at(i);
        QTreeWidgetItem* item;

        cookieDomain = cookie.domain();
        if (cookieDomain.startsWith(".")) {
            cookieDomain = cookieDomain.mid(1);
        }

        const QList<QTreeWidgetItem*>& findParent = ui->cookieTree->findItems(cookieDomain, 0);
        if (findParent.count() == 1) {
            item = new QTreeWidgetItem(findParent.at(0));
        }
        else {
            QTreeWidgetItem* newParent = new QTreeWidgetItem(ui->cookieTree);
            newParent->setText(0, cookieDomain);
            newParent->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            newParent->setWhatsThis(0, cookie.domain());
            ui->cookieTree->addTopLevelItem(newParent);
            item = new QTreeWidgetItem(newParent);
        }

        item->setText(0, "." + cookieDomain);
        item->setText(1, cookie.name());
        item->setData(0, Qt::UserRole + 10, qVariantFromValue(cookie));
        ui->cookieTree->addTopLevelItem(item);

        ++counter;
        if (counter > 50) {
            QApplication::processEvents();
            counter = 0;
        }
    }

    QApplication::restoreOverrideCursor();
}

void CookieManager::search()
{
    ui->cookieTree->filterString(ui->search->text());
//    QString searchText = ui->search->text();
//    if (searchText.isEmpty()) {
//        refreshTable(false);
//        return;
//    }

//    refreshTable(false);
//    ui->cookieTree->setUpdatesEnabled(false);

//    QList<QTreeWidgetItem*> items = ui->cookieTree->findItems(".*"+searchText+"*", Qt::MatchRecursive | Qt::MatchWildcard);

//    QList<QTreeWidgetItem*> foundItems;
//    foreach(QTreeWidgetItem* fitem, items) {
//        if (!fitem->text(0).startsWith("."))
//            continue;
//        QTreeWidgetItem* item = new QTreeWidgetItem();
//        item->setText(0, fitem->text(0));
//        item->setText(1, fitem->text(1));
//        item->setWhatsThis(1, fitem->whatsThis(1));
//        foundItems.append(item);
//    }

//    ui->cookieTree->clear();
//    ui->cookieTree->addTopLevelItems(foundItems);
//    ui->cookieTree->setUpdatesEnabled(true);
}

CookieManager::~CookieManager()
{
    delete ui;
}
