/* ============================================================
* FlashCookieManager plugin for QupZilla
* Copyright (C) 2014  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
*
* some codes and ideas are taken from cookiemanager.cpp and cookiemanager.ui
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
#include "fcm_dialog.h"
#include "ui_fcm_dialog.h"
#include "qztools.h"
#include "iconprovider.h"
#include "fcm_plugin.h"

#include <QMessageBox>
#include <QShortcut>
#include <QTimer>
#include <QInputDialog>
#include <QCloseEvent>
#include <QMenu>

FCM_Dialog::FCM_Dialog(FCM_Plugin* manager, QWidget* parent)
    : QDialog(parent, Qt::WindowStaysOnTopHint)
    , ui(new Ui::FCM_Dialog)
    , m_manager(manager)
{
    ui->setupUi(this);
    QzTools::centerWidgetOnScreen(this);

    ui->path->hide();
    ui->labelPath->hide();

    if (isRightToLeft()) {
        ui->flashCookieTree->headerItem()->setTextAlignment(0, Qt::AlignRight | Qt::AlignVCenter);
        ui->flashCookieTree->setLayoutDirection(Qt::LeftToRight);
        ui->whiteList->setLayoutDirection(Qt::LeftToRight);
        ui->blackList->setLayoutDirection(Qt::LeftToRight);
    }

    connect(ui->flashCookieTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui->removeAll, SIGNAL(clicked()), this, SLOT(removeAll()));
    connect(ui->removeOne, SIGNAL(clicked()), this, SLOT(removeCookie()));
    connect(ui->close, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    connect(ui->close2, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    connect(ui->close3, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    connect(ui->search, SIGNAL(textChanged(QString)), this, SLOT(filterString(QString)));
    connect(ui->reloadFromDisk, SIGNAL(clicked()), this, SLOT(reloadFromDisk()));

    connect(ui->whiteAdd, SIGNAL(clicked()), this, SLOT(addWhitelist()));
    connect(ui->whiteRemove, SIGNAL(clicked()), this, SLOT(removeWhitelist()));
    connect(ui->blackAdd, SIGNAL(clicked()), this, SLOT(addBlacklist()));
    connect(ui->blackRemove, SIGNAL(clicked()), this, SLOT(removeBlacklist()));

    connect(ui->autoMode, SIGNAL(toggled(bool)), ui->notification, SLOT(setEnabled(bool)));
    connect(ui->autoMode, SIGNAL(toggled(bool)), ui->labelNotification, SLOT(setEnabled(bool)));
    connect(ui->browseFlashDataPath, SIGNAL(clicked()), this, SLOT(selectFlashDataPath()));

    ui->autoMode->setChecked(m_manager->readSettings().value(QL1S("autoMode")).toBool());
    ui->notification->setEnabled(m_manager->readSettings().value(QL1S("autoMode")).toBool());
    ui->notification->setChecked(m_manager->readSettings().value(QL1S("notification")).toBool());
    ui->deleteAllOnStartExit->setChecked(m_manager->readSettings().value(QL1S("deleteAllOnStartExit")).toBool());
    ui->flashDataPath->setText(m_manager->flashPlayerDataPath());

    ui->labelNotification->setEnabled(ui->autoMode->isChecked());

    ui->search->setPlaceholderText(tr("Search"));
    ui->flashCookieTree->setDefaultItemShowMode(TreeWidget::ItemsCollapsed);
    ui->flashCookieTree->sortItems(0, Qt::AscendingOrder);
    ui->flashCookieTree->header()->setDefaultSectionSize(220);
    ui->flashCookieTree->setFocus();

    ui->flashCookieTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->flashCookieTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(cookieTreeContextMenuRequested(QPoint)));

    QShortcut* removeShortcut = new QShortcut(QKeySequence("Del"), this);
    connect(removeShortcut, SIGNAL(activated()), this, SLOT(deletePressed()));

    QzTools::setWmClass("FlashCookies", this);
}

void FCM_Dialog::removeAll()
{
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                                         tr("Are you sure to delete all flash cookies on your computer?"), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) {
        return;
    }

    const QList<FlashCookie> &flashCookies = m_manager->flashCookies();
    foreach (const FlashCookie &flashCookie, flashCookies) {
         m_manager->removeCookie(flashCookie);
    }

    ui->flashCookieTree->clear();
    m_manager->clearNewOrigins();
    m_manager->clearCache();
}

void FCM_Dialog::removeCookie()
{
    QTreeWidgetItem* current = ui->flashCookieTree->currentItem();
    if (!current) {
        return;
    }

    const QVariant data = current->data(0, Qt::UserRole + 10);

    if (data.isNull()) {     //Remove whole cookie group
        const QString origin = current->text(0);
        const QList<FlashCookie> &flashCookies = m_manager->flashCookies();
        foreach (const FlashCookie &flashCookie, flashCookies) {
            if (flashCookie.origin == origin) {
                m_manager->removeCookie(flashCookie);
            }
        }

        ui->flashCookieTree->deleteItem(current);
    }
    else {
        const FlashCookie flashCookie = qvariant_cast<FlashCookie>(data);
        m_manager->removeCookie(flashCookie);

        QTreeWidgetItem* parentItem = current->parent();
        ui->flashCookieTree->deleteItem(current);

        if (parentItem->childCount() == 0) {
            ui->flashCookieTree->deleteItem(parentItem);
        }
    }
}

void FCM_Dialog::currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* parent)
{
    Q_UNUSED(parent);
    if (!current) {
        return;
    }
    ui->textEdit->clear();
    const QVariant data = current->data(0, Qt::UserRole + 10);
    if (data.isNull()) {
        ui->name->setText(tr("<flash cookie not selected>"));
        ui->size->setText(tr("<flash cookie not selected>"));
        ui->server->setText(tr("<flash cookie not selected>"));
        ui->lastModified->setText(tr("<flash cookie not selected>"));

        ui->removeOne->setText(tr("Remove flash cookies"));
        ui->path->hide();
        ui->labelPath->hide();
        return;
    }

    const FlashCookie flashCookie = qvariant_cast<FlashCookie>(data);

    QString suffix;
    if (flashCookie.path.startsWith(m_manager->flashPlayerDataPath() + QL1S("/macromedia.com/support/flashplayer/sys"))) {
        suffix = tr(" (settings)");
    }
    ui->name->setText(flashCookie.name + suffix);
    ui->size->setText(QString::number(flashCookie.size) + tr(" Byte"));
    ui->textEdit->setPlainText(flashCookie.contents);
    ui->server->setText(flashCookie.origin);
    ui->path->setText(QString("<a href=\"%1\">%2</a>").arg(QUrl::fromLocalFile(flashCookie.path).toString()).arg(QDir::toNativeSeparators(flashCookie.path)));
    ui->lastModified->setText(flashCookie.lastModification.toString());

    ui->removeOne->setText(tr("Remove flash cookie"));

    ui->labelPath->show();
    ui->path->show();
}

void FCM_Dialog::refreshView(bool forceReload)
{
    disconnect(ui->search, SIGNAL(textChanged(QString)), this, SLOT(filterString(QString)));
    ui->search->clear();
    ui->textEdit->clear();
    connect(ui->search, SIGNAL(textChanged(QString)), this, SLOT(filterString(QString)));

    if (forceReload) {
        m_manager->clearCache();
        m_manager->clearNewOrigins();
    }

    QTimer::singleShot(0, this, SLOT(refreshFlashCookiesTree()));
    QTimer::singleShot(0, this, SLOT(refreshFilters()));
}

void FCM_Dialog::showPage(int index)
{
    ui->tabWidget->setCurrentIndex(index);
}

void FCM_Dialog::refreshFlashCookiesTree()
{
    const QList<FlashCookie> &flashCookies = m_manager->flashCookies();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->flashCookieTree->clear();

    int counter = 0;
    QPointer<FCM_Dialog> guard = this;
    QHash<QString, QTreeWidgetItem*> hash;
    for (int i = 0; i < flashCookies.count(); ++i) {
        const FlashCookie flashCookie = flashCookies.at(i);
        QTreeWidgetItem* item;

        QString cookieOrigin = flashCookie.origin;
        if (cookieOrigin.startsWith(QLatin1Char('.'))) {
            cookieOrigin = cookieOrigin.mid(1);
        }

        QTreeWidgetItem* findParent = hash.value(cookieOrigin);
        if (findParent) {
            item = new QTreeWidgetItem(findParent);
        }
        else {
            QTreeWidgetItem* newParent = new QTreeWidgetItem(ui->flashCookieTree);
            newParent->setText(0, cookieOrigin);
            newParent->setIcon(0, IconProvider::standardIcon(QStyle::SP_DirIcon));
            ui->flashCookieTree->addTopLevelItem(newParent);
            hash[cookieOrigin] = newParent;

            item = new QTreeWidgetItem(newParent);
        }

        QString suffix;
        if (flashCookie.path.startsWith(m_manager->flashPlayerDataPath() + QL1S("/macromedia.com/support/flashplayer/sys"))) {
            suffix = tr(" (settings)");
        }

        if (m_manager->newCookiesList().contains(flashCookie.path + QL1C('/') + flashCookie.name)) {
            suffix += tr(" [new]");
            QFont font = item->font(0);
            font.setBold(true);
            item->setFont(0, font);
            item->parent()->setExpanded(true);
        }

        item->setText(0, flashCookie.name + suffix);
        item->setData(0, Qt::UserRole + 10, QVariant::fromValue(flashCookie));
        ui->flashCookieTree->addTopLevelItem(item);

        ++counter;
        if (counter > 200) {
            QApplication::processEvents();
            counter = 0;
        }

        if (!guard) {
            break;
        }
    }

    QApplication::restoreOverrideCursor();
}

void FCM_Dialog::refreshFilters()
{
    ui->whiteList->clear();
    ui->blackList->clear();

    ui->whiteList->addItems(m_manager->readSettings().value(QL1S("flashCookiesWhitelist")).toStringList());
    ui->blackList->addItems(m_manager->readSettings().value(QL1S("flashCookiesBlacklist")).toStringList());
}

void FCM_Dialog::addWhitelist()
{
    const QString origin = QInputDialog::getText(this, tr("Add to whitelist"), tr("Origin:"));

    addWhitelist(origin);
}

void FCM_Dialog::addWhitelist(const QString &origin)
{
    if (origin.isEmpty()) {
        return;
    }

    if (!ui->blackList->findItems(origin, Qt::MatchFixedString).isEmpty()) {
        QMessageBox::information(this, tr("Already whitelisted!"), tr("The server \"%1\" is already in blacklist, please remove it first.").arg(origin));
        return;
    }

    if (ui->whiteList->findItems(origin, Qt::MatchFixedString).isEmpty()) {
        ui->whiteList->addItem(origin);
    }
}

void FCM_Dialog::removeWhitelist()
{
    delete ui->whiteList->currentItem();
}

void FCM_Dialog::addBlacklist()
{
    const QString origin = QInputDialog::getText(this, tr("Add to blacklist"), tr("Origin:"));

    addBlacklist(origin);
}

void FCM_Dialog::addBlacklist(const QString &origin)
{
    if (origin.isEmpty()) {
        return;
    }

    if (!ui->whiteList->findItems(origin, Qt::MatchFixedString).isEmpty()) {
        QMessageBox::information(this, tr("Already whitelisted!"), tr("The origin \"%1\" is already in whitelist, please remove it first.").arg(origin));
        return;
    }

    if (ui->blackList->findItems(origin, Qt::MatchFixedString).isEmpty()) {
        ui->blackList->addItem(origin);
    }
}

void FCM_Dialog::removeBlacklist()
{
    delete ui->blackList->currentItem();
}

void FCM_Dialog::deletePressed()
{
    if (ui->flashCookieTree->hasFocus()) {
        removeCookie();
    }
    else if (ui->whiteList->hasFocus()) {
        removeWhitelist();
    }
    else if (ui->blackList->hasFocus()) {
        removeBlacklist();
    }
}

void FCM_Dialog::autoModeChanged(bool state)
{
    ui->notification->setEnabled(state);
}

void FCM_Dialog::filterString(const QString &string)
{
    if (string.isEmpty()) {
        for (int i = 0; i < ui->flashCookieTree->topLevelItemCount(); ++i) {
            ui->flashCookieTree->topLevelItem(i)->setHidden(false);
            ui->flashCookieTree->topLevelItem(i)->setExpanded(ui->flashCookieTree->defaultItemShowMode() == TreeWidget::ItemsExpanded);
        }
    }
    else {
        for (int i = 0; i < ui->flashCookieTree->topLevelItemCount(); ++i) {
            QString text = QL1C('.') + ui->flashCookieTree->topLevelItem(i)->text(0);
            ui->flashCookieTree->topLevelItem(i)->setHidden(!text.contains(string, Qt::CaseInsensitive));
            ui->flashCookieTree->topLevelItem(i)->setExpanded(true);
        }
    }
}

void FCM_Dialog::reloadFromDisk()
{
    refreshView(true);
}

void FCM_Dialog::selectFlashDataPath()
{
    QString path = QzTools::getExistingDirectory(QL1S("FCM_Plugin_FlashDataPath"), this, tr("Select Flash Data Path"), ui->flashDataPath->text());
    if (!path.isEmpty()) {
        ui->flashDataPath->setText(path);
    }
}

void FCM_Dialog::cookieTreeContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    QAction* actAddBlacklist = menu.addAction(tr("Add to blacklist"));
    QAction* actAddWhitelist = menu.addAction(tr("Add to whitelist"));

    QTreeWidgetItem* item = ui->flashCookieTree->itemAt(pos);

    if (!item) {
        return;
    }

    ui->flashCookieTree->setCurrentItem(item);

    QAction* activatedAction = menu.exec(ui->flashCookieTree->viewport()->mapToGlobal(pos));

    const QString origin = item->childCount() > 0 ? item->text(0)
                                                  : item->data(0, Qt::UserRole + 10).value<FlashCookie>().origin;

    if (activatedAction == actAddBlacklist) {
        addBlacklist(origin);
    }
    else if (activatedAction == actAddWhitelist) {
        addWhitelist(origin);
    }
}

void FCM_Dialog::closeEvent(QCloseEvent* e)
{
    m_manager->clearNewOrigins();

    QStringList flashWhitelist;
    QStringList flashBlacklist;

    for (int i = 0; i < ui->whiteList->count(); ++i) {
        flashWhitelist.append(ui->whiteList->item(i)->text());
    }

    for (int i = 0; i < ui->blackList->count(); ++i) {
        flashBlacklist.append(ui->blackList->item(i)->text());
    }

    QVariantHash settingsHash;
    settingsHash.insert(QL1S("autoMode"), QVariant(ui->autoMode->isChecked()));
    settingsHash.insert(QL1S("deleteAllOnStartExit"), QVariant(ui->deleteAllOnStartExit->isChecked()));
    settingsHash.insert(QL1S("notification"), QVariant(ui->notification->isChecked()));
    settingsHash.insert(QL1S("flashCookiesWhitelist"), flashWhitelist);
    settingsHash.insert(QL1S("flashCookiesBlacklist"), flashBlacklist);

    QString path = ui->flashDataPath->text();
    path.replace(QL1C('\\'), QL1C('/'));

    settingsHash.insert(QL1S("flashDataPath"), path);

    m_manager->writeSettings(settingsHash);

    e->accept();
}

void FCM_Dialog::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
        close();
    }

    QWidget::keyPressEvent(e);
}

FCM_Dialog::~FCM_Dialog()
{
    delete ui;
}
