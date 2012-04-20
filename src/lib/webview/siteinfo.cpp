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
#include "siteinfo.h"
#include "ui_siteinfo.h"
#include "webview.h"
#include "webpage.h"
#include "mainapplication.h"
#include "downloaditem.h"
#include "certificateinfowidget.h"
#include "globalfunctions.h"
#include "iconprovider.h"

#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QNetworkDiskCache>
#include <QWebFrame>
#include <QClipboard>
#include <QWebSecurityOrigin>
#include <QWebDatabase>

QString SiteInfo::showCertInfo(const QString &string)
{
    if (string.isEmpty()) {
        return tr("<not set in certificate>");
    }
    else {
        return string;
    }
}

SiteInfo::SiteInfo(WebView* view, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SiteInfo)
    , m_certWidget(0)
    , m_view(view)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    ui->listWidget->item(0)->setIcon(QIcon::fromTheme("document-properties", QIcon(":/icons/preferences/document-properties.png")));
    ui->listWidget->item(1)->setIcon(QIcon::fromTheme("applications-graphics", QIcon(":/icons/preferences/applications-graphics.png")));
    ui->listWidget->item(2)->setIcon(QIcon::fromTheme("text-x-sql", QIcon(":/icons/preferences/text-x-sql.png")));
    ui->listWidget->item(3)->setIcon(QIcon::fromTheme("dialog-password", QIcon(":/icons/preferences/dialog-password.png")));
    ui->listWidget->item(0)->setSelected(true);

    WebPage* webPage = view->page();
    QWebFrame* frame = view->page()->mainFrame();
    QString title = view->title();
    QSslCertificate cert = webPage->sslCertificate();
    m_baseUrl = frame->baseUrl();

    //GENERAL
    ui->heading->setText(QString("<b>%1</b>:").arg(title));
    ui->siteAddress->setText(view->url().toString());
    ui->sizeLabel->setText(DownloadItem::fileSizeToString(webPage->totalBytes()));
    QString encoding;

    //Meta
    QWebElementCollection meta = frame->findAllElements("meta");
    for (int i = 0; i < meta.count(); i++) {
        QWebElement element = meta.at(i);

        QString content = element.attribute("content");
        QString name = element.attribute("name");
        if (name.isEmpty()) {
            name = element.attribute("http-equiv");
        }
        if (!element.attribute("charset").isEmpty()) {
            encoding = element.attribute("charset");
        }
        if (content.contains("charset=")) {
            encoding = content.mid(content.indexOf("charset=") + 8);
        }

        if (content.isEmpty() || name.isEmpty()) {
            continue;
        }
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeTags);
        item->setText(0, name);
        item->setText(1, content);
        ui->treeTags->addTopLevelItem(item);
    }
    if (encoding.isEmpty()) {
        encoding = mApp->webSettings()->defaultTextEncoding();
    }
    ui->encodingLabel->setText(encoding.toUpper());

    //MEDIA
    QWebElementCollection img = frame->findAllElements("img");
    for (int i = 0; i < img.count(); i++) {
        QWebElement element = img.at(i);

        QString src = element.attribute("src");
        QString alt = element.attribute("alt");
        if (alt.isEmpty()) {
            if (src.indexOf("/") == -1) {
                alt = src;
            }
            else {
                int pos = src.lastIndexOf("/");
                alt = src.mid(pos);
                alt.remove("/");
            }
        }
        if (src.isEmpty() || alt.isEmpty()) {
            continue;
        }
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeImages);
        item->setText(0, alt);
        item->setText(1, src);
        ui->treeImages->addTopLevelItem(item);
    }

    //DATABASES
    const QList<QWebDatabase> &databases = frame->securityOrigin().databases();

    int counter = 0;
    foreach(const QWebDatabase & b, databases) {
        QListWidgetItem* item = new QListWidgetItem(ui->databaseList);
        item->setText(b.displayName());
        item->setData(Qt::UserRole + 10, counter);

        ++counter;
    }

    if (counter == 0) {
        QListWidgetItem* item = new QListWidgetItem(ui->databaseList);
        item->setText(tr("No databases are used by this page."));
        item->setFlags(item->flags() & Qt::ItemIsSelectable);
    }

    //SECURITY
    if (cert.isValid()) {
        ui->securityLabel->setText(tr("<b>Connection is Encrypted.</b>"));
        ui->certLabel->setText(tr("<b>Your connection to this page is secured with this certificate: </b>"));
        m_certWidget = new CertificateInfoWidget(cert);
        ui->certFrame->addWidget(m_certWidget);

    }
    else {
        ui->securityLabel->setText(tr("<b>Connection Not Encrypted.</b>"));
        ui->certLabel->setText(tr("<b>Your connection to this page is not secured!</b>"));
    }

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
    connect(ui->secDetailsButton, SIGNAL(clicked()), this, SLOT(securityDetailsClicked()));
    connect(ui->saveButton, SIGNAL(clicked(QAbstractButton*)), this, SLOT(downloadImage()));

    connect(ui->databaseList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(databaseItemChanged(QListWidgetItem*)));
    connect(ui->treeImages, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(showImagePreview(QTreeWidgetItem*)));
    connect(ui->treeImages, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(imagesCustomContextMenuRequested(const QPoint &)));

    ui->treeImages->setContextMenuPolicy(Qt::CustomContextMenu);
}

void SiteInfo::imagesCustomContextMenuRequested(const QPoint &p)
{
    QTreeWidgetItem* item = ui->treeImages->itemAt(p);
    if (!item) {
        return;
    }

    QMenu menu;
    menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy Image Location"), this, SLOT(copyActionData()))->setData(item->text(1));
    menu.addAction(tr("Copy Image Name"), this, SLOT(copyActionData()))->setData(item->text(0));
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme("document-save"), tr("Save Image to Disk"), this, SLOT(downloadImage()));
    menu.exec(ui->treeImages->viewport()->mapToGlobal(p));
}

void SiteInfo::databaseItemChanged(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    int id = item->data(Qt::UserRole + 10).toInt();
    const QList<QWebDatabase> &list = m_view->page()->mainFrame()->securityOrigin().databases();

    if (id > list.count() - 1) {
        qDebug("database is shit");
        return;
    }

    const QWebDatabase &db = list.at(id);

    ui->databaseName->setText(QString("%1 (%2)").arg(db.displayName(), db.name()));
    ui->databasePath->setText(db.fileName());
    ui->databaseSize->setText(DownloadItem::fileSizeToString(db.size()));
}

void SiteInfo::copyActionData()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        qApp->clipboard()->setText(action->data().toString());
    }
}

void SiteInfo::downloadImage()
{
    QTreeWidgetItem* item = ui->treeImages->currentItem();
    if (!item) {
        return;
    }

    if (m_activePixmap.isNull()) {
        QMessageBox::warning(this, tr("Error!"), tr("This preview is not available!"));
        return;
    }

    QString imageFileName = qz_getFileNameFromUrl(QUrl(item->text(1)));

    QString filePath = QFileDialog::getSaveFileName(this, tr("Save image..."), QDir::homePath() + "/" + imageFileName);
    if (filePath.isEmpty()) {
        return;
    }

    if (!m_activePixmap.save(filePath)) {
        QMessageBox::critical(this, tr("Error!"), tr("Cannot write to file!"));
        return;
    }
}

void SiteInfo::showImagePreview(QTreeWidgetItem* item)
{
    if (!item) {
        return;
    }
    QUrl imageUrl = QUrl::fromEncoded(item->text(1).toUtf8());
    if (imageUrl.isRelative()) {
        imageUrl = m_baseUrl.resolved(imageUrl);
    }
    QGraphicsScene* scene = new QGraphicsScene(ui->mediaPreview);

    if (imageUrl.scheme() == "data") {
        QByteArray encodedUrl = item->text(1).toUtf8();
        QByteArray imageData = encodedUrl.mid(encodedUrl.indexOf(",") + 1);
        m_activePixmap = qz_pixmapFromByteArray(imageData);
    }
    else if (imageUrl.scheme() == "file") {
        m_activePixmap = QPixmap(imageUrl.toLocalFile());
    }
    else if (imageUrl.scheme() == "qrc") {
        m_activePixmap = QPixmap(imageUrl.toString().mid(3)); // Remove qrc from url
    }
    else {
        QIODevice* cacheData = mApp->networkCache()->data(imageUrl);
        if (!cacheData) {
            m_activePixmap = QPixmap();
        }
        else {
            m_activePixmap.loadFromData(cacheData->readAll());
        }
    }

    if (m_activePixmap.isNull()) {
        scene->addText(tr("Preview not available"));
    }
    else {
        scene->addPixmap(m_activePixmap);
    }

    ui->mediaPreview->setScene(scene);
}

void SiteInfo::securityDetailsClicked()
{
    ui->listWidget->setCurrentRow(3);
}

SiteInfo::~SiteInfo()
{
    delete ui;
    if (m_certWidget) {
        delete m_certWidget;
    }
}
