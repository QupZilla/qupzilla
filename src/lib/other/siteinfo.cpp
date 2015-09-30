/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "listitemdelegate.h"
#include "webview.h"
#include "webpage.h"
#include "mainapplication.h"
#include "downloaditem.h"
#include "certificateinfowidget.h"
#include "qztools.h"
#include "iconprovider.h"
#include "scripts.h"
#include "networkmanager.h"

#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QNetworkDiskCache>
#include <QClipboard>
#include <QTimer>

QString SiteInfo::showCertInfo(const QString &string)
{
    if (string.isEmpty()) {
        return tr("<not set in certificate>");
    }
    return string;
}

SiteInfo::SiteInfo(WebView* view)
    : QWidget()
    , ui(new Ui::SiteInfo)
    , m_certWidget(0)
    , m_view(view)
    , m_imageReply(Q_NULLPTR)
    , m_baseUrl(view->url())
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->treeTags->setLayoutDirection(Qt::LeftToRight);
    QzTools::centerWidgetOnScreen(this);

    ListItemDelegate* delegate = new ListItemDelegate(24, ui->listWidget);
    delegate->setUpdateParentHeight(true);
    delegate->setUniformItemSizes(true);
    ui->listWidget->setItemDelegate(delegate);

    ui->listWidget->item(0)->setIcon(QIcon::fromTheme("document-properties", QIcon(":/icons/preferences/document-properties.png")));
    ui->listWidget->item(1)->setIcon(QIcon::fromTheme("applications-graphics", QIcon(":/icons/preferences/applications-graphics.png")));
    ui->listWidget->item(0)->setSelected(true);

    // General
    ui->heading->setText(QString("<b>%1</b>:").arg(m_view->title()));
    ui->siteAddress->setText(m_view->url().toString());

    if (m_view->url().scheme() == QL1S("https"))
        ui->securityLabel->setText(tr("<b>Connection is Encrypted.</b>"));
    else
        ui->securityLabel->setText(tr("<b>Connection Not Encrypted.</b>"));

    m_view->page()->runJavaScript(QSL("document.charset"), [this](const QVariant &res) {
        ui->encodingLabel->setText(res.toString());
    });

    // Meta
    m_view->page()->runJavaScript(Scripts::getAllMetaAttributes(), [this](const QVariant &res) {
        const QVariantList &list = res.toList();
        Q_FOREACH (const QVariant &val, list) {
            const QVariantMap &meta = val.toMap();
            QString content = meta.value(QSL("content")).toString();
            QString name = meta.value(QSL("name")).toString();

            if (name.isEmpty())
                name = meta.value(QSL("httpequiv")).toString();

            if (content.isEmpty() || name.isEmpty())
                continue;

            QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeTags);
            item->setText(0, name);
            item->setText(1, content);
            ui->treeTags->addTopLevelItem(item);
        }
    });

    // Images
    m_view->page()->runJavaScript(Scripts::getAllImages(), [this](const QVariant &res) {
        const QVariantList &list = res.toList();
        Q_FOREACH (const QVariant &val, list) {
            const QVariantMap &img = val.toMap();
            QString src = img.value(QSL("src")).toString();
            QString alt = img.value(QSL("alt")).toString();
            if (alt.isEmpty()) {
                if (src.indexOf(QLatin1Char('/')) == -1) {
                    alt = src;
                }
                else {
                    int pos = src.lastIndexOf(QLatin1Char('/'));
                    alt = src.mid(pos);
                    alt.remove(QLatin1Char('/'));
                }
            }

            if (src.isEmpty() || alt.isEmpty())
                continue;

            QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeImages);
            item->setText(0, alt);
            item->setText(1, src);
            ui->treeImages->addTopLevelItem(item);
        }
    });

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
    connect(ui->treeImages, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(showImagePreview(QTreeWidgetItem*)));
    connect(ui->treeImages, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(imagesCustomContextMenuRequested(QPoint)));

    ui->treeImages->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeImages->sortByColumn(-1);

    ui->treeTags->sortByColumn(-1);

    QzTools::setWmClass("Site Info", this);
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

void SiteInfo::copyActionData()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        qApp->clipboard()->setText(action->data().toString());
    }
}

void SiteInfo::showLoadingText()
{
    delete ui->mediaPreview->scene();
    QGraphicsScene* scene = new QGraphicsScene(ui->mediaPreview);

    scene->addText(tr("Loading..."));

    ui->mediaPreview->setScene(scene);
}

void SiteInfo::showPixmap(const QPixmap &pixmap)
{
    delete ui->mediaPreview->scene();
    QGraphicsScene* scene = new QGraphicsScene(ui->mediaPreview);

    if (pixmap.isNull())
        scene->addText(tr("Preview not available"));
    else
        scene->addPixmap(pixmap);

    ui->mediaPreview->setScene(scene);
}

void SiteInfo::showImagePreview(QTreeWidgetItem *item)
{
    if (!item) {
        return;
    }
    QUrl imageUrl = QUrl::fromEncoded(item->text(1).toUtf8());
    if (imageUrl.isRelative()) {
        imageUrl = m_baseUrl.resolved(imageUrl);
    }

    QPixmap pixmap;
    bool loading = false;

    if (imageUrl.scheme() == QLatin1String("data")) {
        QByteArray encodedUrl = item->text(1).toUtf8();
        QByteArray imageData = encodedUrl.mid(encodedUrl.indexOf(',') + 1);
        pixmap = QzTools::pixmapFromByteArray(imageData);
    }
    else if (imageUrl.scheme() == QLatin1String("file")) {
        pixmap = QPixmap(imageUrl.toLocalFile());
    }
    else if (imageUrl.scheme() == QLatin1String("qrc")) {
        pixmap = QPixmap(imageUrl.toString().mid(3)); // Remove qrc from url
    }
    else {
        delete m_imageReply;
        m_imageReply = mApp->networkManager()->get(QNetworkRequest(imageUrl));
        connect(m_imageReply, &QNetworkReply::finished, this, [this]() {
            if (m_imageReply->error() != QNetworkReply::NoError)
                return;

            const QByteArray &data = m_imageReply->readAll();
            showPixmap(QPixmap::fromImage(QImage::fromData(data)));
        });

        loading = true;
        showLoadingText();
    }

    if (!loading)
        showPixmap(pixmap);
}

SiteInfo::~SiteInfo()
{
    delete ui;
    delete m_certWidget;
}
