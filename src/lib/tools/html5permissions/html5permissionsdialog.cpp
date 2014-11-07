/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#include "html5permissionsdialog.h"
#include "ui_html5permissionsdialog.h"
#include "settings.h"
#include "mainapplication.h"
#include "html5permissionsmanager.h"

HTML5PermissionsDialog::HTML5PermissionsDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::HTML5PermissionsDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    loadSettings();

    foreach (const QString &site, m_notificationsGranted) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->notifTree);
        item->setText(0, site);
        item->setText(1, tr("Allow"));
        item->setData(0, Qt::UserRole + 10, Allow);

        ui->notifTree->addTopLevelItem(item);
    }

    foreach (const QString &site, m_notificationsDenied) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->notifTree);
        item->setText(0, site);
        item->setText(1, tr("Deny"));
        item->setData(0, Qt::UserRole + 10, Deny);

        ui->notifTree->addTopLevelItem(item);
    }

    foreach (const QString &site, m_geolocationGranted) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->geoTree);
        item->setText(0, site);
        item->setText(1, tr("Allow"));
        item->setData(0, Qt::UserRole + 10, Allow);

        ui->geoTree->addTopLevelItem(item);
    }

    foreach (const QString &site, m_geolocationDenied) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->geoTree);
        item->setText(0, site);
        item->setText(1, tr("Deny"));
        item->setData(0, Qt::UserRole + 10, Deny);

        ui->geoTree->addTopLevelItem(item);
    }

    ui->notifTree->header()->resizeSection(0, 220);
    ui->geoTree->header()->resizeSection(0, 220);

    connect(ui->notifRemove, SIGNAL(clicked()), this, SLOT(removeNotifEntry()));
    connect(ui->geoRemove, SIGNAL(clicked()), this, SLOT(removeGeoEntry()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
}

void HTML5PermissionsDialog::setCurrentTab(int index)
{
    ui->tabWidget->setCurrentIndex(index);
}

void HTML5PermissionsDialog::removeNotifEntry()
{
    QTreeWidgetItem* item = ui->notifTree->currentItem();
    if (!item) {
        return;
    }

    Role role = static_cast<Role>(item->data(0, Qt::UserRole + 10).toInt());
    QString site = item->text(0);

    if (role == Allow) {
        m_notificationsGranted.removeOne(site);
    }
    else {
        m_notificationsDenied.removeOne(site);
    }

    delete item;
}

void HTML5PermissionsDialog::removeGeoEntry()
{
    QTreeWidgetItem* item = ui->geoTree->currentItem();
    if (!item) {
        return;
    }

    Role role = static_cast<Role>(item->data(0, Qt::UserRole + 10).toInt());
    QString site = item->text(0);

    if (role == Allow) {
        m_geolocationGranted.removeOne(site);
    }
    else {
        m_geolocationDenied.removeOne(site);
    }

    delete item;
}

void HTML5PermissionsDialog::loadSettings()
{
    Settings settings;
    settings.beginGroup("HTML5Notifications");
    m_notificationsGranted = settings.value("NotificationsGranted", QStringList()).toStringList();
    m_notificationsDenied = settings.value("NotificationsDenied", QStringList()).toStringList();
    m_geolocationGranted = settings.value("GeolocationGranted", QStringList()).toStringList();
    m_geolocationDenied = settings.value("GeolocationDenied", QStringList()).toStringList();
    settings.endGroup();
}

void HTML5PermissionsDialog::saveSettings()
{
    Settings settings;
    settings.beginGroup("HTML5Notifications");
    settings.setValue("NotificationsGranted", m_notificationsGranted);
    settings.setValue("NotificationsDenied", m_notificationsDenied);
    settings.setValue("GeolocationGranted", m_geolocationGranted);
    settings.setValue("GeolocationDenied", m_geolocationDenied);
    settings.endGroup();

    mApp->html5PermissionsManager()->loadSettings();
}

HTML5PermissionsDialog::~HTML5PermissionsDialog()
{
    delete ui;
}
