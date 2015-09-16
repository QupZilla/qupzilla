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
#include "rssmanager.h"
#include "ui_rssmanager.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include "mainapplication.h"
#include "treewidget.h"
#include "iconprovider.h"
#include "browsinglibrary.h"
#include "qztools.h"
#include "followredirectreply.h"
#include "networkmanager.h"
#include "qzsettings.h"

#include <QMenu>
#include <QLabel>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QWebSettings>
#include <QMessageBox>
#include <QNetworkReply>
#include <QBuffer>
#include <QSqlQuery>

RSSManager::RSSManager(BrowserWindow* window, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RSSManager)
    , m_window(window)
{
    ui->setupUi(this);
#ifdef Q_OS_MAC
    ui->tabWidget->setDocumentMode(false);
#endif
    ui->tabWidget->setElideMode(Qt::ElideRight);
    m_networkManager = mApp->networkManager();

    m_reloadButton = new QToolButton(this);
    m_reloadButton->setAutoRaise(true);
    m_reloadButton->setToolTip(tr("Reload"));
    m_reloadButton->setIcon(QIcon::fromTheme(QSL("view-refresh")));

    ui->tabWidget->setCornerWidget(m_reloadButton);

    connect(m_reloadButton, SIGNAL(clicked()), this, SLOT(reloadFeeds()));
    connect(ui->add, SIGNAL(clicked()), this, SLOT(addFeed()));
    connect(ui->deletebutton, SIGNAL(clicked()), this, SLOT(deleteFeed()));
    connect(ui->edit, SIGNAL(clicked()), this, SLOT(editFeed()));
}

BrowserWindow* RSSManager::getQupZilla()
{
    if (!m_window) {
        m_window = mApp->getWindow();
    }
    return m_window.data();
}

void RSSManager::deleteAllTabs()
{
    while (ui->tabWidget->count() > 0) {
        QWidget* w = ui->tabWidget->widget(0);
        ui->tabWidget->removeTab(0);
        delete w;
    }
}

void RSSManager::setMainWindow(BrowserWindow* window)
{
    if (window) {
        m_window = window;
    }
}

void RSSManager::refreshTable()
{
    QSqlQuery query;
    ui->tabWidget->setUpdatesEnabled(false);
    deleteAllTabs();

    query.exec("SELECT address, title, icon FROM rss");
    int i = 0;
    while (query.next()) {
        QUrl address = query.value(0).toUrl();
        QString title = query.value(1).toString();
        QIcon icon = QPixmap::fromImage(QImage::fromData(query.value(2).toByteArray()));
        TreeWidget* tree = new TreeWidget();
        tree->setHeaderLabel(tr("News"));
        tree->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));

        ui->tabWidget->addTab(tree, title);
        ui->tabWidget->setTabToolTip(i, address.toString());
        connect(tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(loadFeed(QTreeWidgetItem*)));
        connect(tree, SIGNAL(itemMiddleButtonClicked(QTreeWidgetItem*)), this, SLOT(controlLoadFeed(QTreeWidgetItem*)));
        connect(tree, SIGNAL(itemControlClicked(QTreeWidgetItem*)), this, SLOT(controlLoadFeed(QTreeWidgetItem*)));
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, tr("Loading..."));
        tree->addTopLevelItem(item);

        ui->tabWidget->setTabIcon(i, icon);
        beginToLoadSlot(address);
        i++;
    }
    if (i > 0) {
        ui->deletebutton->setEnabled(true);
        m_reloadButton->setEnabled(true);
        ui->edit->setEnabled(true);
    }
    else {
        ui->deletebutton->setEnabled(false);
        m_reloadButton->setEnabled(false);
        ui->edit->setEnabled(false);

        QFrame* frame = new QFrame();
        frame->setObjectName("rssmanager-frame");
        QVBoxLayout* verticalLayout = new QVBoxLayout(frame);
        QLabel* label_2 = new QLabel(frame);
        label_2->setPixmap(QPixmap(":/icons/menu/rss.png"));
        label_2->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
        verticalLayout->addWidget(label_2);
        QLabel* label = new QLabel(frame);
        label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        label->setText(tr("You don't have any RSS Feeds.<br/>\nPlease add some with RSS icon in navigation bar on site which offers feeds."));
        verticalLayout->addWidget(label);
        ui->tabWidget->addTab(frame, tr("Empty"));
    }
    ui->tabWidget->setUpdatesEnabled(true);
}

void RSSManager::reloadFeeds()
{
    TreeWidget* treeWidget = qobject_cast<TreeWidget*>(ui->tabWidget->widget(ui->tabWidget->currentIndex()));
    if (!treeWidget) {
        return;
    }
    treeWidget->clear();
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, tr("Loading..."));
    treeWidget->addTopLevelItem(item);

    beginToLoadSlot(QUrl(ui->tabWidget->tabToolTip(ui->tabWidget->currentIndex())));
}

void RSSManager::addFeed()
{
    QUrl url = QUrl(QInputDialog::getText(this, tr("Add new feed"), tr("Please enter URL of new feed:")));

    if (url.isEmpty() || !url.isValid()) {
        return;
    }

    addRssFeed(url, tr("New feed"), IconProvider::iconForUrl(url));
    refreshTable();
}

void RSSManager::deleteFeed()
{
    QString url = ui->tabWidget->tabToolTip(ui->tabWidget->currentIndex());
    if (url.isEmpty()) {
        return;
    }
    QSqlQuery query;
    query.prepare("DELETE FROM rss WHERE address=?");
    query.addBindValue(url);
    query.exec();

    ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
    if (ui->tabWidget->count() == 0) {
        refreshTable();
    }
}

void RSSManager::editFeed()
{
    QString url = ui->tabWidget->tabToolTip(ui->tabWidget->currentIndex());
    if (url.isEmpty()) {
        return;
    }

    QDialog dialog(this);
    QFormLayout* layout = new QFormLayout(&dialog);
    QLabel* label = new QLabel(&dialog);
    QLineEdit* editUrl = new QLineEdit(&dialog);
    QLineEdit* editTitle = new QLineEdit(&dialog);
    QDialogButtonBox* box = new QDialogButtonBox(&dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), &dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), &dialog, SLOT(accept()));

    label->setText(tr("Fill title and URL of a feed: "));
    layout->addRow(label);
    layout->addRow(new QLabel(tr("Feed title: ")), editTitle);
    layout->addRow(new QLabel(tr("Feed URL: ")), editUrl);
    layout->addRow(box);

    editUrl->setText(ui->tabWidget->tabToolTip(ui->tabWidget->currentIndex()));
    editTitle->setText(ui->tabWidget->tabText(ui->tabWidget->currentIndex()));

    dialog.setWindowTitle(tr("Edit RSS Feed"));
    dialog.setMinimumSize(400, 100);
    dialog.exec();
    if (dialog.result() == QDialog::Rejected) {
        return;
    }

    QString address = editUrl->text();
    QString title = editTitle->text();

    if (address.isEmpty() || title.isEmpty()) {
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE rss SET address=?, title=? WHERE address=?");
    query.bindValue(0, QUrl(address));
    query.bindValue(1, title);
    query.bindValue(2, url);
    query.exec();

    refreshTable();
}

void RSSManager::customContextMenuRequested(const QPoint &position)
{
    TreeWidget* treeWidget = qobject_cast<TreeWidget*>(ui->tabWidget->widget(ui->tabWidget->currentIndex()));
    if (!treeWidget) {
        return;
    }

    if (!treeWidget->itemAt(position)) {
        return;
    }

    QString link = treeWidget->itemAt(position)->toolTip(0);
    if (link.isEmpty()) {
        return;
    }

    QMenu menu;
    menu.addAction(tr("Open link in current tab"), getQupZilla(), SLOT(loadActionUrl()))->setData(link);
    menu.addAction(tr("Open link in new tab"), this, SLOT(loadFeedInNewTab()))->setData(link);
    menu.addAction(tr("Open link in &private window"), mApp, SLOT(startPrivateBrowsing()))->setData(link);

    //Prevent choosing first option with double rightclick
    QPoint pos = treeWidget->viewport()->mapToGlobal(position);
    QPoint p(pos.x(), pos.y() + 1);
    menu.exec(p);
}

void RSSManager::loadFeed(QTreeWidgetItem* item)
{
    if (!item) {
        return;
    }
    if (item->toolTip(0).isEmpty()) {
        return;
    }
    getQupZilla()->loadAddress(QUrl(item->toolTip(0)));
}

void RSSManager::controlLoadFeed(QTreeWidgetItem* item)
{
    if (!item || item->toolTip(0).isEmpty()) {
        return;
    }

    getQupZilla()->tabWidget()->addView(QUrl(item->toolTip(0)), qzSettings->newTabPosition);
}

void RSSManager::loadFeedInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        getQupZilla()->tabWidget()->addView(action->data().toUrl(), qzSettings->newTabPosition);
    }
}

void RSSManager::beginToLoadSlot(const QUrl &url)
{
    FollowRedirectReply* reply = new FollowRedirectReply(url, m_networkManager);
    connect(reply, SIGNAL(finished()), this, SLOT(finished()));

    QPair<FollowRedirectReply*, QUrl> pair;
    pair.first = reply;
    pair.second = url;
    m_replies.append(pair);
}

void RSSManager::finished()
{
    FollowRedirectReply* reply = qobject_cast<FollowRedirectReply*> (sender());
    if (!reply) {
        return;
    }

    QString replyUrl;
    for (int i = 0; i < m_replies.count(); i++) {
        QPair<FollowRedirectReply*, QUrl> pair = m_replies.at(i);
        if (pair.first == reply) {
            replyUrl = pair.second.toString();
            break;
        }
    }

    if (replyUrl.isEmpty()) {
        return;
    }

    QString currentTag;
    QString linkString;
    QString titleString;

    QXmlStreamReader xml;
    xml.addData(reply->readAll());

    reply->deleteLater();

    int tabIndex = -1;
    for (int i = 0; i < ui->tabWidget->count(); i++) {
        if (replyUrl == ui->tabWidget->tabToolTip(i)) {
            tabIndex = i;
            break;
        }
    }

    if (tabIndex == -1) {
        return;
    }

    TreeWidget* treeWidget = qobject_cast<TreeWidget*>(ui->tabWidget->widget(tabIndex));
    if (!treeWidget) {
        return;
    }
    treeWidget->clear();

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("item")) {
                linkString = xml.attributes().value("rss:about").toString();
            }
            currentTag = xml.qualifiedName().toString();
        }
        else if (xml.isEndElement()) {
            if (xml.qualifiedName() == QLatin1String("item")) {
                QTreeWidgetItem* item = new QTreeWidgetItem;
                item->setText(0, titleString);
                item->setIcon(0, QIcon(":/icons/other/feed.png"));
                item->setToolTip(0, linkString);
                treeWidget->addTopLevelItem(item);

                titleString.clear();
                linkString.clear();
            }
        }
        else if (xml.isCharacters() && !xml.isWhitespace()) {
            if (currentTag == QLatin1String("title")) {
                titleString = xml.text().toString();
            }
            else if (currentTag == QLatin1String("link")) {
                linkString += xml.text().toString();
            }
        }
    }

    if (treeWidget->topLevelItemCount() == 0) {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, tr("Error in fetching feed"));
        treeWidget->addTopLevelItem(item);
    }
}

bool RSSManager::addRssFeed(const QUrl &url, const QString &title, const QIcon &icon)
{
    if (url.isEmpty()) {
        return false;
    }
    QSqlQuery query;
    query.prepare("SELECT id FROM rss WHERE address=?");
    query.addBindValue(url);
    query.exec();

    if (!query.next()) {
        QImage image = icon.pixmap(16, 16).toImage();

        if (image == IconProvider::emptyWebImage()) {
            image.load(":icons/menu/rss.png");
        }

        query.prepare("INSERT INTO rss (address, title, icon) VALUES(?,?,?)");
        query.bindValue(0, url);
        query.bindValue(1, title);
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        query.bindValue(2, buffer.data());
        query.exec();
        return true;
    }

    QMessageBox::warning(getQupZilla(), tr("RSS feed duplicated"), tr("You already have this feed."));
    return false;
}

RSSManager::~RSSManager()
{
    delete ui;
}
