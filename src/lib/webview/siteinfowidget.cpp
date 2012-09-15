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
#include "siteinfowidget.h"
#include "ui_siteinfowidget.h"
#include "qupzilla.h"
#include "mainapplication.h"
#include "webpage.h"
#include "tabbedwebview.h"

#include <QToolTip>
#include <QSqlQuery>

SiteInfoWidget::SiteInfoWidget(QupZilla* mainClass, QWidget* parent)
    : LocationBarPopup(parent)
    , ui(new Ui::SiteInfoWidget)
    , p_QupZilla(mainClass)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    setPopupAlignment(Qt::AlignLeft);

    WebView* view = p_QupZilla->weView();
    WebPage* webPage = view->page();
    QUrl url = view->url();

    if (webPage->sslCertificate().isValid()) {
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

    query.prepare("SELECT sum(count) FROM history WHERE url LIKE ?");
    query.addBindValue(QString("%1://%2%").arg(scheme, host));
    query.exec();

    if (query.next()) {
        int count = query.value(0).toInt();
        if (count > 3) {
            ui->historyLabel->setText(tr("This is your <b>%1</b> visit of this site.").arg(QString::number(count) + "."));
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
            ui->historyLabel->setText(tr("This is your <b>%1</b> visit of this site.").arg(text));
        }
    }
    connect(ui->pushButton, SIGNAL(clicked()), p_QupZilla, SLOT(showPageInfo()));
}

SiteInfoWidget::~SiteInfoWidget()
{
    delete ui;
}
