/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "siteinfo.h"
#include "ui_siteinfo.h"
#include "qupzilla.h"
#include "webview.h"
#include "webpage.h"

SiteInfo::SiteInfo(QupZilla* mainClass, QWidget* parent) :
    QDialog(parent)
    ,ui(new Ui::SiteInfo)
    ,p_QupZilla(mainClass)
{
    ui->setupUi(this);
    WebView* view = p_QupZilla->weView();
    QWebFrame* frame = view->page()->mainFrame();
    QString title = view->title();
    QSslCertificate cert = view->webPage()->sslCertificate();

    //GENERAL
    ui->heading->setText(QString("<b>%1</b>:").arg(title));
    ui->siteAddress->setText(frame->baseUrl().toString());


    //Meta
    QWebElementCollection meta = frame->findAllElements("meta");
    for (int i = 0; i<meta.count(); i++) {
        QWebElement element = meta.at(i);

        QString content = element.attribute("content");
        QString name = element.attribute("name");
        if (name.isEmpty())
            name = element.attribute("http-equiv");

        if (content.isEmpty() || name.isEmpty())
            continue;
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeTags);
        item->setText(0, name);
        item->setText(1, content);
        ui->treeTags->addTopLevelItem(item);
    }

    //MEDIA
    QWebElementCollection img = frame->findAllElements("img");
    for (int i = 0; i<img.count(); i++) {
        QWebElement element = img.at(i);

        QString src = element.attribute("src");
        QString alt = element.attribute("alt");
        if (alt.isEmpty()) {
            if (src.indexOf("/") == -1)
                alt = src;
            else{
                int pos = src.lastIndexOf("/");
                alt = src.mid(pos);
                alt.remove("/");
            }
        }
        if (src.isEmpty() || alt.isEmpty())
            continue;
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeImages);
        item->setText(0, alt);
        item->setText(1, src);
        ui->treeImages->addTopLevelItem(item);
    }

    //SECURITY
    if (cert.isValid()) {
        ui->certLabel->setText(tr("<b>Your connection to this page is secured: </b>"));
        //Issued to
        ui->issuedToCN->setText( cert.subjectInfo(QSslCertificate::CommonName) );
        ui->issuedToO->setText( cert.subjectInfo(QSslCertificate::Organization) );
        ui->issuedToOU->setText( cert.subjectInfo(QSslCertificate::OrganizationalUnitName) );
        ui->issuedToSN->setText( cert.serialNumber() );
        //Issued By
        ui->issuedByCN->setText( cert.issuerInfo(QSslCertificate::CommonName) );
        ui->issuedByO->setText( cert.issuerInfo(QSslCertificate::Organization) );
        ui->issuedByOU->setText( cert.issuerInfo(QSslCertificate::OrganizationalUnitName) );
        //Validity
        ui->validityIssuedOn->setText( cert.effectiveDate().toString("dddd d. MMMM yyyy") );
        ui->validityExpiresOn->setText( cert.expiryDate().toString("dddd d. MMMM yyyy") );
    } else {
        ui->certFrame->setVisible(false);
        ui->certLabel->setText(tr("<b>Your connection to this page is not secured!</b>"));
    }

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
}

void SiteInfo::itemChanged(QListWidgetItem *item)
{
    if (!item)
        return;
    int index = item->whatsThis().toInt();
    ui->stackedWidget->setCurrentIndex(index);
}

SiteInfo::~SiteInfo()
{
    delete ui;
}
