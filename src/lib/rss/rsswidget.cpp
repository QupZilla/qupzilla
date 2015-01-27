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
#include "rsswidget.h"
#include "ui_rsswidget.h"
#include "mainapplication.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "rssmanager.h"
#include "rssnotification.h"

#if QTWEBENGINE_DISABLED

#include <QToolTip>
#include <QPushButton>
#include <QWebEngineFrame>
#include <QSqlQuery>

RSSWidget::RSSWidget(WebView* view, QWidget* parent)
    : LocationBarPopup(parent)
    , ui(new Ui::RSSWidget)
    , m_view(view)
{
    ui->setupUi(this);

    QWebEngineFrame* frame = m_view->page()->mainFrame();
    QWebElementCollection links = frame->findAllElements("link[type=\"application/rss+xml\"]");

    // Make sure RSS feeds fit into a window, in case there is a lot of feeds from one page
    // See #906
    int cols = links.count() / 10 == 0 ? 1 : links.count() / 10;
    int row = 0;

    for (int i = 0; i < links.count(); i++) {
        QWebElement element = links.at(i);
        QString title = element.attribute("title");
        const QUrl url = QUrl::fromEncoded(element.attribute("href").toUtf8());
        if (url.isEmpty()) {
            continue;
        }

        if (title.isEmpty()) {
            title = tr("Untitled feed");
        }

        QPushButton* button = new QPushButton(this);
        button->setIcon(QIcon(":icons/other/feed.png"));
        button->setStyleSheet("text-align:left");
        button->setText(title);
        button->setProperty("rss-url", url);
        button->setProperty("rss-title", title);

        if (!isRssFeedAlreadyStored(url)) {
            button->setFlat(true);
            button->setToolTip(url.toString());
        }
        else {
            button->setFlat(false);
            button->setEnabled(false);
            button->setToolTip(tr("You already have this feed."));
        }

        int pos = i % cols > 0 ? (i % cols) * 2 : 0;

        ui->gridLayout->addWidget(button, row, pos);
        connect(button, SIGNAL(clicked()), this, SLOT(addRss()));

        if (i % cols == cols - 1) {
            row++;
        }
    }
}

void RSSWidget::addRss()
{
    if (!m_view) {
        return;
    }
    if (QPushButton* button = qobject_cast<QPushButton*>(sender())) {
        QUrl url = button->property("rss-url").toUrl();

        if (url.isRelative()) {
            url = m_view->page()->mainFrame()->baseUrl().resolved(url);
        }

        if (!url.isValid()) {
            return;
        }

        QString title = button->property("rss-title").toString();
        if (title.isEmpty()) {
            title = m_view->url().host();
        }

        RSSNotification* notif = new RSSNotification(title, url, m_view);
        m_view->addNotification(notif);
        close();
    }
}

bool RSSWidget::isRssFeedAlreadyStored(const QUrl &url)
{
    QUrl rurl = url;

    if (url.isRelative()) {
        rurl = m_view->page()->mainFrame()->baseUrl().resolved(url);
    }

    if (rurl.isEmpty()) {
        return false;
    }
    QSqlQuery query;
    query.prepare("SELECT id FROM rss WHERE address=?");
    query.addBindValue(rurl);
    query.exec();

    return query.next();
}

RSSWidget::~RSSWidget()
{
    delete ui;
}

#endif
