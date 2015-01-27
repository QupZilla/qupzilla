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
#include "rssnotification.h"
#include "ui_rssnotification.h"
#include "mainapplication.h"
#include "browsinglibrary.h"
#include "iconprovider.h"
#include "processinfo.h"
#include "rssmanager.h"
#include "settings.h"
#include "webview.h"
#include "qztools.h"

#include <QMessageBox>
#include <QClipboard>
#include <QFile>

#if QTWEBENGINE_DISABLED

RSSNotification::RSSNotification(const QString &title, const QUrl &url, WebView* parent)
    : AnimatedWidget(AnimatedWidget::Down, 300, parent)
    , ui(new Ui::RSSNotification)
    , m_title(title)
    , m_url(url)
    , m_view(parent)
{
    setAutoFillBackground(true);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(widget());
    ui->closeButton->setIcon(IconProvider::standardIcon(QStyle::SP_DialogCloseButton));
    ui->label->setText(tr("RSS feed <b>\"%1\"</b>").arg(title));

    RssApp bloglines;
    bloglines.type = WebApplication;
    bloglines.title = "Bloglines";
    bloglines.icon = QIcon(":/icons/sites/bloglines.png");
    bloglines.address = "http://www.bloglines.com/sub?url=";

    RssApp myaol;
    myaol.type = WebApplication;
    myaol.title = "My AOL";
    myaol.icon = QIcon(":/icons/sites/aol.png");
    myaol.address = "http://feeds.my.aol.com/add.jsp?url=";

    RssApp netvibes;
    netvibes.type = WebApplication;
    netvibes.title = "Netvibes";
    netvibes.icon = QIcon(":/icons/sites/netvibes.png");
    netvibes.address = "http://www.netvibes.com/subscribe.php?url=";

    RssApp yahoo;
    yahoo.type = WebApplication;
    yahoo.title = "Yahoo!";
    yahoo.icon = QIcon(":/icons/sites/yahoo.png");
    yahoo.address = "http://add.my.yahoo.com/rss?url=";

    m_rssApps << bloglines << myaol << netvibes << yahoo;

#ifdef Q_OS_UNIX
    const QString akregatorBin = QzTools::resolveFromPath("akregator");
    const QString lifereaBin = QzTools::resolveFromPath("liferea");
    const QString lifereaAddFeedBin = QzTools::resolveFromPath("liferea-add-feed");

    if (!akregatorBin.isEmpty()) {
        RssApp akregator;
        akregator.type = DesktopApplication;
        akregator.title = "Akregator";
        akregator.icon = QIcon(":/icons/sites/akregator.png");
        akregator.executable = akregatorBin;
        akregator.arguments = "-a";

        m_rssApps << akregator;
    }

    if (!lifereaBin.isEmpty() && !lifereaAddFeedBin.isEmpty()) {
        RssApp liferea;
        liferea.type = DesktopApplication;
        liferea.title = "Liferea";
        liferea.icon = QIcon(":/icons/sites/liferea.png");
        liferea.executable = lifereaAddFeedBin;

        m_rssApps << liferea;
    }
#endif

    foreach (const RssApp &app, m_rssApps) {
        ui->comboBox->addItem(app.icon, app.title, QVariant(app.type));
    }

    ui->comboBox->addItem(QIcon(":/icons/qupzilla.png"), tr("Internal Reader"), QVariant(Internal));
    ui->comboBox->addItem(tr("Other..."), QVariant(Other));

    Settings settings;
    settings.beginGroup("RSS");
    ui->comboBox->setCurrentIndex(settings.value("LastAddOptionIndex", ui->comboBox->count() - 2).toInt());
    settings.endGroup();

    connect(ui->add, SIGNAL(clicked()), this, SLOT(addRss()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(hide()));

    startAnimation();
}

void RSSNotification::hide()
{
    m_view->setFocus();
    AnimatedWidget::hide();
}

void RSSNotification::addRss()
{
    bool success = false;
    int index = ui->comboBox->currentIndex();

    switch (ui->comboBox->itemData(index).toInt()) {
    case WebApplication: {
        const RssApp app = m_rssApps.at(index);
        const QUrl url = QUrl::fromEncoded(QString(app.address + QUrl::toPercentEncoding(m_url.toString())).toLatin1());

        m_view->openUrlInNewTab(url, Qz::NT_CleanSelectedTab);
        success = true;
        break;
    }

    case DesktopApplication: {
        const RssApp app = m_rssApps.at(index);
        if (app.title == QLatin1String("Liferea")) {
            if (!ProcessInfo("liferea").isRunning()) {
                QMessageBox::warning(this, tr("Liferea not running"), tr("Liferea must be running in order to add new feed."));
                success = false;
                break;
            }
        }
        const QString arguments = QString("%1 %2").arg(app.arguments, QString::fromUtf8(m_url.toEncoded()));
        success = QzTools::startExternalProcess(app.executable, arguments);
        break;
    }

    case Other: {
        QApplication::clipboard()->setText(m_url.toEncoded());
        const QString message = tr("To add this RSS feed into other application, please use this information:<br/><br/>"
                                   "<b>Title: </b>%1<br/><b>Url: </b>%2<br/><br/>"
                                   "Url address of this feed has been copied into your clipboard.").arg(m_title, m_url.toString());
        QMessageBox::information(0, tr("Add feed into other application"), message);
        success = true;
        break;
    }

    case Internal:
        success = mApp->rssManager()->addRssFeed(m_url, m_title, m_view->icon());
        if (success) {
            mApp->browsingLibrary()->showRSS(mApp->windows().at(0));
        }
        break;

    default:
        break;
    }

    if (success) {
        Settings settings;
        settings.beginGroup("RSS");
        settings.setValue("LastAddOptionIndex", index);
        settings.endGroup();

        hide();
    }
}

RSSNotification::~RSSNotification()
{
    delete ui;
}

#endif
