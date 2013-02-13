/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include <QProcess>
#include <QFile>

static bool startExternalProcess(const QString &program, const QStringList &arguments)
{
    if (!QProcess::startDetached(program, arguments)) {
        QString info = "<ul><li><b>" + RSSNotification::tr("Executable: ") + "</b>" + program + "</li><li><b>" + RSSNotification::tr("Arguments: ") + "</b>" + arguments.join(" ") + "</li></ul>";
        QMessageBox::critical(0, RSSNotification::tr("Cannot start external program"), RSSNotification::tr("Cannot start external program! %1").arg(info));

        return false;
    }

    return true;
}

RSSNotification::RSSNotification(const QString &title, const QUrl &url, WebView* parent)
    : AnimatedWidget(AnimatedWidget::Down, 300, parent)
    , ui(new Ui::RSSNotification)
    , m_title(title)
    , m_url(url)
    , m_view(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(widget());
    ui->closeButton->setIcon(qIconProvider->standardIcon(QStyle::SP_DialogCloseButton));
    ui->label->setText(tr("RSS feed <b>\"%1\"</b>").arg(title));

    m_rssApps << RssApp("Bloglines", "http://www.bloglines.com/sub?url=", QIcon(":/icons/sites/bloglines.png"))
              << RssApp("Google Reader", "http://www.google.com/ig/add?feedurl=", QIcon(":/icons/sites/google.png"))
              << RssApp("My AOL", "http://feeds.my.aol.com/add.jsp?url=", QIcon(":/icons/sites/aol.png"))
              << RssApp("Netvibes", "http://www.netvibes.com/subscribe.php?url=", QIcon(":/icons/sites/netvibes.png"))
              << RssApp("Yahoo!", "http://add.my.yahoo.com/rss?url=", QIcon(":/icons/sites/yahoo.png"));

#ifdef QZ_WS_X11
    const QString &akregatorBin = QzTools::resolveFromPath("akregator");
    const QString &lifereaBin = QzTools::resolveFromPath("liferea");
    const QString &lifereaAddFeedBin = QzTools::resolveFromPath("liferea-add-feed");

    if (!akregatorBin.isEmpty()) {
        m_rssApps << RssApp("Akregator", akregatorBin + " -a ", QIcon(":/icons/sites/akregator.png"), DesktopApplication);
    }

    if (!lifereaBin.isEmpty() && !lifereaAddFeedBin.isEmpty()) {
        m_rssApps << RssApp("Liferea", lifereaAddFeedBin + " ", QIcon(":/icons/sites/liferea.png"), DesktopApplication);
    }
#endif

    foreach(const RssApp & app, m_rssApps) {
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
        const QUrl &url = QUrl::fromEncoded(QString(app.address + QUrl::toPercentEncoding(m_url.toString())).toLatin1());

        m_view->openUrlInNewTab(url, Qz::NT_SelectedTab);
        success = true;
        break;
    }

    case DesktopApplication: {
        const RssApp app = m_rssApps.at(index);
        if (app.title == QLatin1String("Akregator")) {
            success = startExternalProcess("/usr/bin/akregator", QStringList() << "-a" << m_url.toEncoded());
        }
        else if (app.title == QLatin1String("Liferea")) {
            if (!ProcessInfo("liferea").isRunning()) {
                QMessageBox::warning(this, tr("Liferea not running"), tr("Liferea must be running in order to add new feed."));
                success = false;
            }
            else {
                success = startExternalProcess("/usr/bin/liferea-add-feed", QStringList(m_url.toEncoded()));
            }
        }
        break;
    }

    case Other: {
        QApplication::clipboard()->setText(m_url.toEncoded());
        const QString &message = tr("To add this RSS feed into other application, please use this information:<br/><br/>"
                                    "<b>Title: </b>%1<br/><b>Url: </b>%2<br/><br/>"
                                    "Url address of this feed has been copied into your clipboard.").arg(m_title, m_url.toString());
        QMessageBox::information(0, tr("Add feed into other application"), message);
        success = true;
        break;
    }

    case Internal:
        success = mApp->rssManager()->addRssFeed(m_url, m_title, m_view->icon());
        if (success) {
            mApp->browsingLibrary()->showRSS(mApp->mainWindows().at(0));
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
