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
#include "ui_html5permissionsnotification.h"
#include "html5permissionsnotification.h"
#include "html5permissionsmanager.h"
#include "mainapplication.h"
#include "iconprovider.h"

#if QTWEBENGINE_DISABLED
#include <QWebEngineFrame>

#ifdef USE_QTWEBKIT_2_2
HTML5PermissionsNotification::HTML5PermissionsNotification(const QString &host, QWebEngineFrame* frame, const QWebEnginePage::Feature &feature)
    : AnimatedWidget(AnimatedWidget::Down, 300, 0)
    , ui(new Ui::HTML5PermissionsNotification)
    , m_host(host)
    , m_frame(frame)
    , m_feature(feature)
{
    setAutoFillBackground(true);
    ui->setupUi(widget());

    ui->close->setIcon(IconProvider::standardIcon(QStyle::SP_DialogCloseButton));

    QString message;
    QString site = m_host.isEmpty() ? tr("this site") : QString("<b>%1</b>").arg(m_host);

    if (feature == QWebEnginePage::Notifications) {
        ui->iconLabel->setPixmap(QPixmap(":icons/other/notification.png"));
        message = tr("Allow %1 to show desktop notifications?").arg(site);
    }
    else if (feature == QWebEnginePage::Geolocation) {
        ui->iconLabel->setPixmap(QPixmap(":icons/other/geolocation.png"));
        message = tr("Allow %1 to locate your position?").arg(site);
    }

    ui->textLabel->setText(message);

    connect(ui->allow, SIGNAL(clicked()), this, SLOT(grantPermissions()));
    connect(ui->deny, SIGNAL(clicked()), this, SLOT(denyPermissions()));
    connect(ui->close, SIGNAL(clicked()), this, SLOT(denyPermissions()));

    startAnimation();
}

void HTML5PermissionsNotification::grantPermissions()
{
    if (!m_frame || !m_frame->page()) {
        return;
    }

    QWebEnginePage* page = m_frame->page();
    page->setFeaturePermission(m_frame, m_feature, QWebEnginePage::PermissionGrantedByUser);

    if (ui->remember->isChecked()) {
        mApp->html5PermissionsManager()->rememberPermissions(m_host, m_feature, QWebEnginePage::PermissionGrantedByUser);
    }

    hide();
}

void HTML5PermissionsNotification::denyPermissions()
{
    if (!m_frame || !m_frame->page()) {
        return;
    }

    QWebEnginePage* page = m_frame->page();
    page->setFeaturePermission(m_frame, m_feature, QWebEnginePage::PermissionDeniedByUser);

    if (ui->remember->isChecked()) {
        mApp->html5PermissionsManager()->rememberPermissions(m_host, m_feature, QWebEnginePage::PermissionDeniedByUser);
    }

    hide();
}

HTML5PermissionsNotification::~HTML5PermissionsNotification()
{
    delete ui;
}
#endif // USE_QTWEBKIT_2_2

#endif
