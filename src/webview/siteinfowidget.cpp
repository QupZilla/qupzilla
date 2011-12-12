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
#include "siteinfowidget.h"
#include "ui_siteinfowidget.h"
#include "qupzilla.h"
#include "webpage.h"

SiteInfoWidget::SiteInfoWidget(QupZilla* mainClass, QWidget* parent)
    : QMenu(parent)
    , ui(new Ui::SiteInfoWidget)
    , p_QupZilla(mainClass)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    WebView* view = p_QupZilla->weView();
    QUrl url = view->url();

    if (view->webPage()->sslCertificate().isValid()) {
        ui->secureLabel->setText(tr("Your connection to this site is <b>secured</b>."));
        ui->secureIcon->setPixmap(QPixmap(":/icons/locationbar/accept.png"));
    }
    else {
        ui->secureLabel->setText(tr("Your connection to this site is <b>unsecured</b>."));
        ui->secureIcon->setPixmap(QPixmap(":/icons/locationbar/warning.png"));
    }

    QString scheme = url.scheme();
    QSqlQuery query;
    QString host = url.host();

    query.exec("SELECT sum(count) FROM history WHERE url LIKE '" + scheme + "://" + host + "%' ");
    if (query.next()) {
        int count = query.value(0).toInt();
        if (count > 3) {
            ui->historyLabel->setText(tr("This is your <b>%1.</b> visit of this site.").arg(count));
            ui->historyIcon->setPixmap(QPixmap(":/icons/locationbar/accept.png"));
        }
        else if (count == 0) {
            ui->historyLabel->setText(tr("You have <b>never</b> visited this site before."));
            ui->historyIcon->setPixmap(QPixmap(":/icons/locationbar/warning.png"));
        }
        else {
            ui->historyIcon->setPixmap(QPixmap(":/icons/locationbar/warning.png"));
            QString text;
            if (count == 1) {
                text = tr("first");
            }
            else if (count == 2) {
                text = tr("second");
            }
            else if (count == 3) {
                text = tr("third");
            }
            ui->historyLabel->setText(tr("This is your <b>%1.</b> visit of this site.").arg(text));
        }
    }
    connect(ui->pushButton, SIGNAL(clicked()), p_QupZilla, SLOT(showPageInfo()));
}

void SiteInfoWidget::showAt(QWidget* _parent)
{
    QPoint p = _parent->mapToGlobal(QPoint(0, 0));
    move(p.x(), p.y() + _parent->height());
    show();
}

SiteInfoWidget::~SiteInfoWidget()
{
    delete ui;
}
