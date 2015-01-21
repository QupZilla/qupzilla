/* ============================================================
* FlashCookieManager plugin for QupZilla
* Copyright (C) 2014  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#include "fcm_notification.h"
#include "ui_fcm_notification.h"
#include "iconprovider.h"
#include "fcm_plugin.h"

FCM_Notification::FCM_Notification(FCM_Plugin* manager, int newOriginsCount)
    : AnimatedWidget(AnimatedWidget::Down, 300, 0)
    , ui(new Ui::FCM_Notification)
    , m_manager(manager)
{
    setAutoFillBackground(true);
    ui->setupUi(widget());

    ui->close->setIcon(IconProvider::standardIcon(QStyle::SP_DialogCloseButton));

    if (newOriginsCount == 1) {
        ui->textLabel->setText(tr("A new flash cookie was detected"));
    }
    else {
        ui->textLabel->setText(tr("%1 new flash cookies were detected").arg(newOriginsCount));
    }
    connect(ui->buttonView, SIGNAL(clicked()), m_manager, SLOT(showFlashCookieManager()));
    connect(ui->buttonView, SIGNAL(clicked()), this, SLOT(hide()));
    connect(ui->close, SIGNAL(clicked()), this, SLOT(hide()));

    startAnimation();
}

FCM_Notification::~FCM_Notification()
{
    delete ui;
}
