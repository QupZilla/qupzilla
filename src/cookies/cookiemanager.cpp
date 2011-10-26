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
#include "cookiemanager.h"
#include "ui_cookiemanager.h"
#include "qupzilla.h"
#include "cookiejar.h"
#include "mainapplication.h"
#include "globalfunctions.h"

CookieManager::CookieManager(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CookieManager)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::WindowModal);

    ui->setupUi(this);
    qz_centerWidgetOnScreen(this);

    //QTimer::singleShot(0, this, SLOT(refreshTable()));

    connect(ui->cookieTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),this, SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui->removeAll, SIGNAL(clicked()), this, SLOT(removeAll()));
    connect(ui->removeCookie, SIGNAL(clicked()), this, SLOT(removeCookie()));
    connect(ui->close, SIGNAL(clicked(QAbstractButton*)), this, SLOT(hide()));
    connect(ui->search, SIGNAL(returnPressed()), this, SLOT(search()));
//    connect(ui->search, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(search()));
    connect(ui->search, SIGNAL(textChanged(QString)), ui->cookieTree, SLOT(filterString(QString)));

    ui->search->setInactiveText(tr("Search"));
    ui->cookieTree->setDefaultItemShowMode(TreeWidget::ItemsCollapsed);
}

void CookieManager::removeAll()
{
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                         tr("Are you sure to delete all cookies on your computer?"), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes)
        return;

    m_cookies.clear();
    mApp->cookieJar()->setAllCookies(m_cookies);
    ui->cookieTree->clear();
}

void CookieManager::removeCookie()
{
    QTreeWidgetItem* current = ui->cookieTree->currentItem();
    if (!current)
        return;

    if (current->text(1).isEmpty()) {     //Remove whole cookie group
        QString domain = current->whatsThis(0);
        foreach(QNetworkCookie cok, m_cookies) {
            if (cok.domain() == domain || cok.domain()  ==  domain.mid(1))
                m_cookies.removeOne(cok);
        }

        ui->cookieTree->deleteItem(current);
        mApp->cookieJar()->setAllCookies(m_cookies);
        refreshTable(false);
        return;
    }

    int index = current->whatsThis(1).toInt();
    m_cookies.removeAt(index);
    mApp->cookieJar()->setAllCookies(m_cookies);
    refreshTable(false);

    if (!ui->search->text().isEmpty())
        search();
}

void CookieManager::currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* parent)
{
    Q_UNUSED(parent);
    if (!current)
        return;

    ui->removeCookie->setEnabled(true);

    if (current->text(1).isEmpty()) {
        ui->name->setText(tr("<cookie not selected>"));
        ui->value->setText(tr("<cookie not selected>"));
        ui->server->setText(tr("<cookie not selected>"));
        ui->path->setText(tr("<cookie not selected>"));
        ui->secure->setText(tr("<cookie not selected>"));
        ui->expiration->setText(tr("<cookie not selected>"));
        ui->removeCookie->setText(tr("Remove cookies"));
        return;
    }
    ui->removeCookie->setText(tr("Remove cookie"));

    int index = current->whatsThis(1).toInt();
    QNetworkCookie cok = m_cookies.at(index);

    ui->name->setText(cok.name());
    ui->value->setText(cok.value());
    ui->server->setText(cok.domain());
    ui->path->setText(cok.path());
    cok.isSecure() ? ui->secure->setText(tr("Secure only")) : ui->secure->setText(tr("All connections"));
    cok.isSessionCookie() ? ui->expiration->setText(tr("Session cookie")) : ui->expiration->setText(QDateTime(cok.expirationDate()).toString("hh:mm:ss dddd d. MMMM yyyy"));

}

void CookieManager::refreshTable(bool refreshCookieJar)
{
    if (refreshCookieJar)
        m_cookies = mApp->cookieJar()->getAllCookies();

    ui->cookieTree->setUpdatesEnabled(false);
    ui->cookieTree->clear();

    QString cookServer;
    for (int i = 0; i<m_cookies.count(); i++) {
        QNetworkCookie cok = m_cookies.at(i);
        QTreeWidgetItem* item;

        cookServer = cok.domain();
        if (cookServer.startsWith("."))
            cookServer = cookServer.mid(1);

        QList<QTreeWidgetItem*> findParent = ui->cookieTree->findItems(cookServer, 0);
        if (findParent.count() == 1) {
            item = new QTreeWidgetItem(findParent.at(0));
        }else{
            QTreeWidgetItem* newParent = new QTreeWidgetItem(ui->cookieTree);
            newParent->setText(0, cookServer);
            newParent->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            newParent->setWhatsThis(0, cok.domain());
            ui->cookieTree->addTopLevelItem(newParent);
            item = new QTreeWidgetItem(newParent);
        }

        item->setText(0,"."+cookServer);
        item->setText(1, cok.name());
        item->setWhatsThis(1, QString::number(i));
        ui->cookieTree->addTopLevelItem(item);
    }

    ui->cookieTree->setUpdatesEnabled(true);
}

void CookieManager::search()
{
    QString searchText = ui->search->text();
    if (searchText.isEmpty()) {
        refreshTable(false);
        return;
    }

    refreshTable(false);
    ui->cookieTree->setUpdatesEnabled(false);

    QList<QTreeWidgetItem*> items = ui->cookieTree->findItems(".*"+searchText+"*", Qt::MatchRecursive | Qt::MatchWildcard);

    QList<QTreeWidgetItem*> foundItems;
    foreach(QTreeWidgetItem* fitem, items) {
        if (!fitem->text(0).startsWith("."))
            continue;
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, fitem->text(0));
        item->setText(1, fitem->text(1));
        item->setWhatsThis(1, fitem->whatsThis(1));
        foundItems.append(item);
    }
    ui->cookieTree->clear();
    ui->cookieTree->addTopLevelItems(foundItems);
    ui->cookieTree->setUpdatesEnabled(true);
}

CookieManager::~CookieManager()
{
    delete ui;
}
