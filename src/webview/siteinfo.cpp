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
#include "siteinfo.h"
#include "ui_siteinfo.h"
#include "qupzilla.h"
#include "webview.h"
#include "webpage.h"
#include "downloaditem.h"
#include "certificateinfowidget.h"

QString SiteInfo::showCertInfo(const QString &string)
{
    if (string.isEmpty())
        return tr("<not set in certificate>");
    else return string;
}

SiteInfo::SiteInfo(QupZilla* mainClass, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SiteInfo)
    , p_QupZilla(mainClass)
    , m_certWidget(0)
{
    ui->setupUi(this);
    WebView* view = p_QupZilla->weView();
    QWebFrame* frame = view->page()->mainFrame();
    QString title = view->title();
    QSslCertificate cert = view->webPage()->sslCertificate();

    //GENERAL
    ui->heading->setText(QString("<b>%1</b>:").arg(title));
    ui->siteAddress->setText(frame->baseUrl().toString());
    ui->sizeLabel->setText( DownloadItem::fileSizeToString(view->webPage()->totalBytes()) );
    QString encoding;

    //Meta
    QWebElementCollection meta = frame->findAllElements("meta");
    for (int i = 0; i<meta.count(); i++) {
        QWebElement element = meta.at(i);

        QString content = element.attribute("content");
        QString name = element.attribute("name");
        if (name.isEmpty())
            name = element.attribute("http-equiv");
        if (!element.attribute("charset").isEmpty())
            encoding = element.attribute("charset");
        if (content.contains("charset="))
            encoding = content.mid(content.indexOf("charset=")+8);

        if (content.isEmpty() || name.isEmpty())
            continue;
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeTags);
        item->setText(0, name);
        item->setText(1, content);
        ui->treeTags->addTopLevelItem(item);
    }
    if (encoding.isEmpty())
        encoding = mApp->webSettings()->defaultTextEncoding();
    ui->encodingLabel->setText(encoding.toUpper());

    //MEDIA
    QWebElementCollection img = frame->findAllElements("img");
    for (int i = 0; i<img.count(); i++) {
        QWebElement element = img.at(i);

        QString src = element.attribute("src");
        QString alt = element.attribute("alt");
        if (alt.isEmpty()) {
            if (src.indexOf("/") == -1)
                alt = src;
            else {
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
        ui->securityLabel->setText(tr("<b>Connection is Encrypted.</b>"));
        ui->certLabel->setText(tr("<b>Your connection to this page is secured with this certificate: </b>"));
        m_certWidget = new CertificateInfoWidget(cert);
        ui->certFrame->addWidget(m_certWidget);

    } else {
        ui->securityLabel->setText(tr("<b>Connection Not Encrypted.</b>"));
        ui->certLabel->setText(tr("<b>Your connection to this page is not secured!</b>"));
    }

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
    connect(ui->secDetailsButton, SIGNAL(clicked()), this, SLOT(securityDetailsClicked()));
    connect(ui->treeImages, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(showImagePreview(QTreeWidgetItem*)));
    ui->treeImages->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeImages, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(imagesCustomContextMenuRequested(const QPoint&)));
}

void SiteInfo::imagesCustomContextMenuRequested(const QPoint& p)
{
    QTreeWidgetItem* item = ui->treeImages->itemAt(p);
    if (!item)
        return;

    QMenu menu;
    menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy Image Location"), this, SLOT(copyActionData()))->setData(item->text(1));
    menu.addAction(tr("Copy Image Name"), this, SLOT(copyActionData()))->setData(item->text(0));
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme("document-save"), tr("Save Image to Disk"), this, SLOT(downloadImage()))->setData(ui->treeImages->indexOfTopLevelItem(item));
    menu.exec(QCursor::pos());
}

void SiteInfo::copyActionData()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        qApp->clipboard()->setText(action->data().toString());
    }
}

void SiteInfo::downloadImage()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QTreeWidgetItem* item = ui->treeImages->topLevelItem(action->data().toInt());
        if (!item)
            return;

        QUrl imageUrl = item->text(1);
        if (imageUrl.host().isEmpty()) {
            imageUrl.setHost(QUrl(ui->siteAddress->text()).host());
            imageUrl.setScheme(QUrl(ui->siteAddress->text()).scheme());
        }
        QIODevice* cacheData = mApp->networkCache()->data(imageUrl);
        if (!cacheData) {
            QMessageBox::warning(this, tr("Error!"), tr("This preview is not available!"));
            return;
        }

        QString filePath = QFileDialog::getSaveFileName(this, tr("Save image..."), QDir::homePath()+"/"+item->text(0));
        if (filePath.isEmpty())
            return;

        QFile file(filePath);
        if (!file.open(QFile::WriteOnly)) {
            QMessageBox::critical(this, tr("Error!"), tr("Cannot write to file!"));
            return;
        }
        file.write(cacheData->readAll());
        file.close();
    }
}

void SiteInfo::showImagePreview(QTreeWidgetItem *item)
{
    if (!item)
        return;
    QUrl imageUrl = item->text(1);
    if (imageUrl.host().isEmpty()) {
        imageUrl.setHost(QUrl(ui->siteAddress->text()).host());
        imageUrl.setScheme(QUrl(ui->siteAddress->text()).scheme());
    }
    QIODevice* cacheData = mApp->networkCache()->data(imageUrl);
    QPixmap pixmap;
    bool invalidPixmap = false;
    QGraphicsScene* scene = new QGraphicsScene(ui->mediaPreview);

    if (!cacheData)
        invalidPixmap = true;
    else {
        pixmap.loadFromData(cacheData->readAll());
        if (pixmap.isNull())
            invalidPixmap = true;
    }
    if (invalidPixmap)
        scene->addText(tr("Preview not available"));
    else
        scene->addPixmap(pixmap);

    ui->mediaPreview->setScene(scene);
}

void SiteInfo::securityDetailsClicked()
{
    ui->listWidget->setCurrentRow(2);
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
    if (m_certWidget)
        delete m_certWidget;
}
