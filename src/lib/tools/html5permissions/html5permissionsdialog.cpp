/* ============================================================
* QupZilla - Qt web browser
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

HTML5PermissionsDialog::HTML5PermissionsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::HTML5PermissionsDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    loadSettings();

    ui->treeWidget->header()->resizeSection(0, 220);

    connect(ui->remove, &QPushButton::clicked, this, &HTML5PermissionsDialog::removeEntry);
    connect(ui->feature, SIGNAL(currentIndexChanged(int)), this, SLOT(featureIndexChanged()));
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &HTML5PermissionsDialog::saveSettings);

    showFeaturePermissions(currentFeature());
}

HTML5PermissionsDialog::~HTML5PermissionsDialog()
{
    delete ui;
}

void HTML5PermissionsDialog::showFeaturePermissions(QWebEnginePage::Feature feature)
{
    if (!m_granted.contains(feature) || !m_denied.contains(feature)) {
        return;
    }

    ui->treeWidget->clear();

    foreach (const QString &site, m_granted.value(feature)) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, site);
        item->setText(1, tr("Allow"));
        item->setData(0, Qt::UserRole + 10, Allow);
        ui->treeWidget->addTopLevelItem(item);
    }

    foreach (const QString &site, m_denied.value(feature)) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, site);
        item->setText(1, tr("Deny"));
        item->setData(0, Qt::UserRole + 10, Deny);
        ui->treeWidget->addTopLevelItem(item);
    }
}

void HTML5PermissionsDialog::featureIndexChanged()
{
    showFeaturePermissions(currentFeature());
}

void HTML5PermissionsDialog::removeEntry()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (!item) {
        return;
    }

    Role role = static_cast<Role>(item->data(0, Qt::UserRole + 10).toInt());
    const QString origin = item->text(0);

    if (role == Allow)
        m_granted[currentFeature()].removeOne(origin);
    else
        m_denied[currentFeature()].removeOne(origin);

    delete item;
}

QWebEnginePage::Feature HTML5PermissionsDialog::currentFeature() const
{
    switch (ui->feature->currentIndex()) {
    case 0:
        return QWebEnginePage::Notifications;
    case 1:
        return QWebEnginePage::Geolocation;
    case 2:
        return QWebEnginePage::MediaAudioCapture;
    case 3:
        return QWebEnginePage::MediaVideoCapture;
    case 4:
        return QWebEnginePage::MediaAudioVideoCapture;
    case 5:
        return QWebEnginePage::MouseLock;
    default:
        Q_UNREACHABLE();
        return QWebEnginePage::Notifications;
    }
}

void HTML5PermissionsDialog::loadSettings()
{
    Settings settings;
    settings.beginGroup("HTML5Notifications");

    m_granted[QWebEnginePage::Notifications] = settings.value("NotificationsGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::Notifications] = settings.value("NotificationsDenied", QStringList()).toStringList();

    m_granted[QWebEnginePage::Geolocation] = settings.value("GeolocationGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::Geolocation] = settings.value("GeolocationDenied", QStringList()).toStringList();

    m_granted[QWebEnginePage::MediaAudioCapture] = settings.value("MediaAudioCaptureGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::MediaAudioCapture] = settings.value("MediaAudioCaptureDenied", QStringList()).toStringList();

    m_granted[QWebEnginePage::MediaVideoCapture] = settings.value("MediaVideoCaptureGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::MediaVideoCapture] = settings.value("MediaVideoCaptureDenied", QStringList()).toStringList();

    m_granted[QWebEnginePage::MediaAudioVideoCapture] = settings.value("MediaAudioVideoCaptureGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::MediaAudioVideoCapture] = settings.value("MediaAudioVideoCaptureDenied", QStringList()).toStringList();

    m_granted[QWebEnginePage::MouseLock] = settings.value("MouseLockGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::MouseLock] = settings.value("MouseLockDenied", QStringList()).toStringList();

    settings.endGroup();
}

void HTML5PermissionsDialog::saveSettings()
{
    Settings settings;
    settings.beginGroup("HTML5Notifications");

    settings.setValue("NotificationsGranted", m_granted[QWebEnginePage::Notifications]);
    settings.setValue("NotificationsDenied", m_denied[QWebEnginePage::Notifications]);

    settings.setValue("GeolocationGranted", m_granted[QWebEnginePage::Geolocation]);
    settings.setValue("GeolocationDenied", m_denied[QWebEnginePage::Geolocation]);

    settings.setValue("MediaAudioCaptureGranted", m_granted[QWebEnginePage::MediaAudioCapture]);
    settings.setValue("MediaAudioCaptureDenied", m_denied[QWebEnginePage::MediaAudioCapture]);

    settings.setValue("MediaVideoCaptureGranted", m_granted[QWebEnginePage::MediaVideoCapture]);
    settings.setValue("MediaVideoCaptureDenied", m_denied[QWebEnginePage::MediaVideoCapture]);

    settings.setValue("MediaAudioVideoCaptureGranted", m_granted[QWebEnginePage::MediaAudioVideoCapture]);
    settings.setValue("MediaAudioVideoCaptureDenied", m_denied[QWebEnginePage::MediaAudioVideoCapture]);

    settings.setValue("MouseLockGranted", m_granted[QWebEnginePage::MouseLock]);
    settings.setValue("MouseLockDenied", m_denied[QWebEnginePage::MouseLock]);

    settings.endGroup();

    mApp->html5PermissionsManager()->loadSettings();
}
