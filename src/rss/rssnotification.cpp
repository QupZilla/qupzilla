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
#include "rssnotification.h"
#include "ui_rssnotification.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "iconprovider.h"

RSSNotification::RSSNotification(QString host, QWidget* parent) :
    AnimatedWidget(AnimatedWidget::Down, 300, parent),
    ui(new Ui::RSSNotification)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(widget());
    ui->closeButton->setIcon(IconProvider::standardIcon(QStyle::SP_DialogCloseButton));
    ui->label->setText(tr("You have successfuly added RSS feed \"%1\".").arg(host));

    connect(ui->pushButton, SIGNAL(clicked()), mApp->getWindow(), SLOT(showRSSManager()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(hide()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(hide()));
    startAnimation();
}

RSSNotification::~RSSNotification()
{
    delete ui;
}
