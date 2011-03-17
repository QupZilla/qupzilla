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
#include "rsswidget.h"
#include "ui_rsswidget.h"
#include "mainapplication.h"
#include "webview.h"
#include "rssmanager.h"
#include "rssnotification.h"

RSSWidget::RSSWidget(WebView* view, QList<QPair<QString, QString> > availableRss, QWidget* parent)
    :QMenu(parent)
    ,ui(new Ui::RSSWidget)
    ,m_avRss(availableRss)
    ,m_view(view)
{
    ui->setupUi(this);
    for (int i = 0; i < m_avRss.count(); i++) {
        QPair<QString, QString> rss = m_avRss.at(i);
        QPushButton* button = new QPushButton(this);
        button->setText(tr("Add"));
        button->setWhatsThis(rss.second);
        button->setToolTip(rss.first);
        QLabel* label = new QLabel(this);
        label->setText(rss.first);
        ui->gridLayout->addWidget(label, i, 0);
        ui->gridLayout->addWidget(button, i, 1);
        connect(button, SIGNAL(clicked()), this, SLOT(addRss()));
    }
}

void RSSWidget::showAt(QWidget* _parent)
{
    QPoint p = _parent->mapToGlobal(QPoint(0, 0));
    move( (p.x()+_parent->width() ) - width(), p.y() + _parent->height());
    show();
}

void RSSWidget::addRss()
{
    if(!m_view)
        return;
    if (QPushButton* button = qobject_cast<QPushButton*>(sender())) {
        QUrl url = QUrl(button->whatsThis());
        QString urlString = button->whatsThis();
        if(url.host().isEmpty()) {
            if(!urlString.startsWith("/"))
                urlString="/"+urlString;
            urlString = m_view->url().host()+urlString;
            QUrl temp(urlString);
            if(temp.scheme().isEmpty())
                urlString="http://"+urlString;
            temp = QUrl(urlString);
            if(temp.scheme().isEmpty() || temp.host().isEmpty())
                return;
        }
        if (!url.isValid())
            return;

        QString title;
        if (button->toolTip().isEmpty())
            title = m_view->url().host();
        else
            title = button->toolTip();

        if (mApp->rssManager()->addRssFeed(urlString, title)) {
            RSSNotification* notif = new RSSNotification(title, m_view);
            m_view->addNotification(notif);
            close();
        }
    }
}

RSSWidget::~RSSWidget()
{
    delete ui;
}
