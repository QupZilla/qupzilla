/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "browserwindow.h"
#include "cookiejar.h"
#include "mainapplication.h"
#include "qztools.h"
#include "settings.h"
#include "iconprovider.h"

#include <QNetworkCookie>
#include <QMessageBox>
#include <QDateTime>
#include <QShortcut>
#include <QTimer>
#include <QInputDialog>
#include <QCloseEvent>

CookieManager::CookieManager(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CookieManager)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    QzTools::centerWidgetOnScreen(this);

    if (isRightToLeft()) {
        ui->cookieTree->headerItem()->setTextAlignment(0, Qt::AlignRight | Qt::AlignVCenter);
        ui->cookieTree->headerItem()->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
        ui->cookieTree->setLayoutDirection(Qt::LeftToRight);
        ui->whiteList->setLayoutDirection(Qt::LeftToRight);
        ui->blackList->setLayoutDirection(Qt::LeftToRight);
    }

    // Stored Cookies
    connect(ui->cookieTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui->removeAll, SIGNAL(clicked()), this, SLOT(removeAll()));
    connect(ui->removeOne, SIGNAL(clicked()), this, SLOT(remove()));
    connect(ui->close, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    connect(ui->close2, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    connect(ui->close3, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    connect(ui->search, SIGNAL(textChanged(QString)), this, SLOT(filterString(QString)));

    // Cookie Filtering
    connect(ui->whiteAdd, SIGNAL(clicked()), this, SLOT(addWhitelist()));
    connect(ui->whiteRemove, SIGNAL(clicked()), this, SLOT(removeWhitelist()));
    connect(ui->blackAdd, SIGNAL(clicked()), this, SLOT(addBlacklist()));
    connect(ui->blackRemove, SIGNAL(clicked()), this, SLOT(removeBlacklist()));

    // Cookie Settings
    Settings settings;
    settings.beginGroup("Cookie-Settings");
    ui->saveCookies->setChecked(settings.value("allowCookies", true).toBool());
    ui->filter3rdParty->setChecked(settings.value("filterThirdPartyCookies", false).toBool());
    ui->filterTracking->setChecked(settings.value("filterTrackingCookie", false).toBool());
    ui->deleteCookiesOnClose->setChecked(settings.value("deleteCookiesOnClose", false).toBool());
    ui->whiteList->addItems(settings.value("whitelist", QStringList()).toStringList());
    ui->blackList->addItems(settings.value("blacklist", QStringList()).toStringList());
    settings.endGroup();

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
    ui->filter3rdParty->hide();
#endif

    ui->search->setPlaceholderText(tr("Search"));
    ui->cookieTree->setDefaultItemShowMode(TreeWidget::ItemsCollapsed);
    ui->cookieTree->sortItems(0, Qt::AscendingOrder);
    ui->cookieTree->header()->setDefaultSectionSize(220);
    ui->cookieTree->setFocus();

    QShortcut* removeShortcut = new QShortcut(QKeySequence("Del"), this);
    connect(removeShortcut, SIGNAL(activated()), this, SLOT(deletePressed()));

    connect(ui->search, SIGNAL(textChanged(QString)), this, SLOT(filterString(QString)));
    connect(mApp->cookieJar(), &CookieJar::cookieAdded, this, &CookieManager::addCookie);
    connect(mApp->cookieJar(), &CookieJar::cookieRemoved, this, &CookieManager::removeCookie);

    // Load cookies
    foreach (const QNetworkCookie &cookie, mApp->cookieJar()->getAllCookies())
        addCookie(cookie);

    QzTools::setWmClass("Cookies", this);
}

void CookieManager::removeAll()
{
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                                         tr("Are you sure you want to delete all cookies on your computer?"), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) {
        return;
    }

    mApp->cookieJar()->deleteAllCookies();

    m_itemHash.clear();
    m_domainHash.clear();
    ui->cookieTree->clear();
}

void CookieManager::remove()
{
    QTreeWidgetItem* current = ui->cookieTree->currentItem();
    if (!current) {
        return;
    }

    QList<QNetworkCookie> cookies;

    if (current->childCount()) {
        for (int i = 0; i < current->childCount(); ++i) {
            QTreeWidgetItem *item = current->child(i);
            if (item && m_itemHash.contains(item)) {
                cookies.append(m_itemHash.value(item));
            }
        }
    } else if (m_itemHash.contains(current)) {
        cookies.append(m_itemHash.value(current));
    }

    foreach (const QNetworkCookie &cookie, cookies) {
        mApp->cookieJar()->deleteCookie(cookie);
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

    const QNetworkCookie cookie = qvariant_cast<QNetworkCookie>(current->data(0, Qt::UserRole + 10));

    ui->name->setText(cookie.name());
    ui->value->setText(cookie.value());
    ui->server->setText(cookie.domain());
    ui->path->setText(cookie.path());
    cookie.isSecure() ? ui->secure->setText(tr("Secure only")) : ui->secure->setText(tr("All connections"));
    cookie.isSessionCookie() ? ui->expiration->setText(tr("Session cookie")) : ui->expiration->setText(QDateTime(cookie.expirationDate()).toString("hh:mm:ss dddd d. MMMM yyyy"));

    ui->removeOne->setText(tr("Remove cookie"));
}

void CookieManager::addWhitelist()
{
    const QString server = QInputDialog::getText(this, tr("Add to whitelist"), tr("Server:"));

    if (server.isEmpty()) {
        return;
    }

    if (!ui->blackList->findItems(server, Qt::MatchFixedString).isEmpty()) {
        QMessageBox::information(this, tr("Already blacklisted!"), tr("The server \"%1\" is already in blacklist, please remove it first.").arg(server));
        return;
    }

    if (ui->whiteList->findItems(server, Qt::MatchFixedString).isEmpty()) {
        ui->whiteList->addItem(server);
    }
}

void CookieManager::removeWhitelist()
{
    delete ui->whiteList->currentItem();
}

void CookieManager::addBlacklist()
{
    const QString server = QInputDialog::getText(this, tr("Add to blacklist"), tr("Server:"));
    addBlacklist(server);
}

void CookieManager::addBlacklist(const QString &server)
{
    if (server.isEmpty()) {
        return;
    }

    if (!ui->whiteList->findItems(server, Qt::MatchFixedString).isEmpty()) {
        QMessageBox::information(this, tr("Already whitelisted!"), tr("The server \"%1\" is already in whitelist, please remove it first.").arg(server));
        return;
    }

    if (ui->blackList->findItems(server, Qt::MatchFixedString).isEmpty()) {
        ui->blackList->addItem(server);
    }
}

QString CookieManager::cookieDomain(const QNetworkCookie &cookie) const
{
    QString domain = cookie.domain();
    if (domain.startsWith(QLatin1Char('.'))) {
        domain = domain.mid(1);
    }
    return domain;
}

QTreeWidgetItem *CookieManager::cookieItem(const QNetworkCookie &cookie) const
{
    QHashIterator<QTreeWidgetItem*, QNetworkCookie> it(m_itemHash);
    while (it.hasNext()) {
        it.next();
        if (it.value() == cookie)
            return it.key();
    }
    return Q_NULLPTR;
}

void CookieManager::removeBlacklist()
{
    delete ui->blackList->currentItem();
}

void CookieManager::deletePressed()
{
    if (ui->cookieTree->hasFocus()) {
        remove();
    }
    else if (ui->whiteList->hasFocus()) {
        removeWhitelist();
    }
    else if (ui->blackList->hasFocus()) {
        removeBlacklist();
    }
}

void CookieManager::filterString(const QString &string)
{
    if (string.isEmpty()) {
        for (int i = 0; i < ui->cookieTree->topLevelItemCount(); ++i) {
            ui->cookieTree->topLevelItem(i)->setHidden(false);
            ui->cookieTree->topLevelItem(i)->setExpanded(ui->cookieTree->defaultItemShowMode() == TreeWidget::ItemsExpanded);
        }
    }
    else {
        for (int i = 0; i < ui->cookieTree->topLevelItemCount(); ++i) {
            QString text = "." + ui->cookieTree->topLevelItem(i)->text(0);
            ui->cookieTree->topLevelItem(i)->setHidden(!text.contains(string, Qt::CaseInsensitive));
            ui->cookieTree->topLevelItem(i)->setExpanded(true);
        }
    }
}

void CookieManager::addCookie(const QNetworkCookie &cookie)
{
    QTreeWidgetItem* item;
    const QString domain = cookieDomain(cookie);

    QTreeWidgetItem* findParent = m_domainHash.value(domain);
    if (findParent) {
        item = new QTreeWidgetItem(findParent);
    }
    else {
        QTreeWidgetItem* newParent = new QTreeWidgetItem(ui->cookieTree);
        newParent->setText(0, domain);
        newParent->setIcon(0, IconProvider::standardIcon(QStyle::SP_DirIcon));
        newParent->setData(0, Qt::UserRole + 10, cookie.domain());
        ui->cookieTree->addTopLevelItem(newParent);
        m_domainHash[domain] = newParent;

        item = new QTreeWidgetItem(newParent);
    }

    item->setText(0, "." + domain);
    item->setText(1, cookie.name());
    item->setData(0, Qt::UserRole + 10, QVariant::fromValue(cookie));
    ui->cookieTree->addTopLevelItem(item);

    m_itemHash[item] = cookie;
}

void CookieManager::removeCookie(const QNetworkCookie &cookie)
{
    QTreeWidgetItem *item = cookieItem(cookie);
    if (!item)
        return;

    m_itemHash.remove(item);

    if (item->parent() && item->parent()->childCount() == 1) {
        m_domainHash.remove(cookieDomain(cookie));
        delete item->parent();
        item = Q_NULLPTR;
    }

    delete item;
}

void CookieManager::closeEvent(QCloseEvent* e)
{
    QStringList whitelist;
    QStringList blacklist;

    for (int i = 0; i < ui->whiteList->count(); ++i) {
        whitelist.append(ui->whiteList->item(i)->text());
    }

    for (int i = 0; i < ui->blackList->count(); ++i) {
        blacklist.append(ui->blackList->item(i)->text());
    }

    Settings settings;
    settings.beginGroup("Cookie-Settings");
    settings.setValue("allowCookies", ui->saveCookies->isChecked());
    settings.setValue("filterThirdPartyCookies", ui->filter3rdParty->isChecked());
    settings.setValue("filterTrackingCookie", ui->filterTracking->isChecked());
    settings.setValue("deleteCookiesOnClose", ui->deleteCookiesOnClose->isChecked());
    settings.setValue("whitelist", whitelist);
    settings.setValue("blacklist", blacklist);
    settings.endGroup();

    mApp->cookieJar()->loadSettings();

    e->accept();
}

void CookieManager::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
        close();
    }

    QWidget::keyPressEvent(e);
}

CookieManager::~CookieManager()
{
    delete ui;
}
