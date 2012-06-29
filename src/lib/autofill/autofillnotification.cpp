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
#include "autofillnotification.h"
#include "ui_autofillnotification.h"
#include "autofillmodel.h"
#include "mainapplication.h"
#include "animatedwidget.h"
#include "iconprovider.h"

AutoFillNotification::AutoFillNotification(const QUrl &url, const QByteArray &data, const QString &user, const QString &pass, QWidget* parent)
    : AnimatedWidget(AnimatedWidget::Down, 300, parent)
    , ui(new Ui::AutoFillWidget)
    , m_url(url)
    , m_data(data)
    , m_user(user)
    , m_pass(pass)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(widget());
    ui->label->setText(tr("Do you want QupZilla to remember the password for <b>%1</b> on %2?").arg(user, url.host()));
    ui->closeButton->setIcon(qIconProvider->standardIcon(QStyle::SP_DialogCloseButton));

    connect(ui->remember, SIGNAL(clicked()), this, SLOT(remember()));
    connect(ui->never, SIGNAL(clicked()), this, SLOT(never()));
    connect(ui->notnow, SIGNAL(clicked()), this, SLOT(hide()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(hide()));

    startAnimation();
}

void AutoFillNotification::never()
{
    mApp->autoFill()->blockStoringfor(m_url);
    hide();
}

void AutoFillNotification::remember()
{
    mApp->autoFill()->addEntry(m_url, m_data, m_user, m_pass);
    hide();
}

AutoFillNotification::~AutoFillNotification()
{
    delete ui;
}
