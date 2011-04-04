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
/* ============================================================
*
* Copyright (C) 2009 by Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2010 by Matthieu Gicquel <matgic78@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */

#include "clicktoflash.h"
#include "clickablelabel.h"
#include "mainapplication.h"
#include "pluginproxy.h"
#include "adblockmanager.h"
#include "adblocksubscription.h"

ClickToFlash::ClickToFlash(const QUrl &pluginUrl, QWidget* parent)
        : QWidget(parent)
        , m_url(pluginUrl)
{
    //AdBlock
    AdBlockManager* manager = AdBlockManager::instance();
    if (manager->isEnabled()) {
        QString urlString = pluginUrl.toEncoded();
        AdBlockSubscription* subscription = manager->subscription();
        if (!subscription->allow(urlString) && subscription->block(urlString)) {
            QWidget* parent = parentWidget();
            QWebView* view = 0;
            while (parent) {
                if (QWebView* aView = qobject_cast<QWebView*>(parent)) {
                    view = aView;
                    break;
                }
                parent = parent->parentWidget();
            }
            if (!view)
                return;

            const QString selector = "%1[type=\"application/x-shockwave-flash\"]";

            QList<QWebFrame*> frames;
            frames.append(view->page()->mainFrame());
            while (!frames.isEmpty()) {
                QWebFrame* frame = frames.takeFirst();
                QWebElement docElement = frame->documentElement();

                QWebElementCollection elements;
                elements.append(docElement.findAll(selector.arg("object")));
                elements.append(docElement.findAll(selector.arg("embed")));

                foreach(QWebElement element, elements) {
                    if (checkElement(element)) {
                        element.setAttribute("style", "visible:none;");
                        deleteLater();
                        return;
                    }
                }
                frames += frame->childFrames();
            }
        }
    }

    QHBoxLayout* horizontalLayout;
    QFrame* frame;
    QHBoxLayout* horizontalLayout_2;
    QToolButton* toolButton;

    horizontalLayout = new QHBoxLayout(this);
    frame = new QFrame(this);
    frame->setStyleSheet("QFrame { border: 1px solid #e8e8e8; }");
    frame->setContentsMargins(0,0,0,0);
    horizontalLayout_2 = new QHBoxLayout(frame);
    toolButton = new QToolButton(frame);
    toolButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolButton->setStyleSheet("QToolButton { background: url(:/icons/other/flash.png) no-repeat;\n"
        "background-position: center; border: none;}\n"
        "QToolButton:hover { background: url(:/icons/other/flashstart.png) no-repeat; \n"
        "background-position: center;  border:none;}");
    toolButton->setCursor(Qt::PointingHandCursor);
    horizontalLayout_2->addWidget(toolButton);
    horizontalLayout->addWidget(frame);
    horizontalLayout->setContentsMargins(0,0,0,0);
    horizontalLayout_2->setContentsMargins(0,0,0,0);

    connect(toolButton, SIGNAL(clicked(bool)), this, SLOT(load()));
    setMinimumSize(27,27);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));
}

void ClickToFlash::customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    menu.addAction(tr("Flash blocked by ClickToFlash"));
    menu.addSeparator();
    menu.addAction(tr("Add %1 to whitelist").arg(m_url.host()), this, SLOT(toWhitelist()));
    menu.actions().at(0)->setEnabled(false);
    menu.exec(mapToGlobal(pos));
}

void ClickToFlash::toWhitelist()
{
    mApp->plugins()->c2f_addWhitelist(m_url.host());
    load();
}

void ClickToFlash::load()
{    
    QWidget* parent = parentWidget();
    QWebView* view = 0;
    while (parent) {
        if (QWebView* aView = qobject_cast<QWebView*>(parent)) {
            view = aView;
            break;
        }
        parent = parent->parentWidget();
    }
    if (!view)
        return;

    const QString selector = "%1[type=\"application/x-shockwave-flash\"]";
    hide();

    QList<QWebFrame*> frames;
    frames.append(view->page()->mainFrame());
    while (!frames.isEmpty()) {
        QWebFrame* frame = frames.takeFirst();
        QWebElement docElement = frame->documentElement();

        QWebElementCollection elements;
        elements.append(docElement.findAll(selector.arg("object")));
        elements.append(docElement.findAll(selector.arg("embed")));

        foreach(QWebElement element, elements) {
            if (checkElement(element)) {
                QWebElement substitute = element.clone();
                emit signalLoadClickToFlash(true);
                element.replace(substitute);
                deleteLater();
                return;
            }
        }
        frames += frame->childFrames();
    }
}


bool ClickToFlash::checkElement(QWebElement el)
{
    QString checkString;
    QString urlString;
    checkString = QUrl(el.attribute("src")).toString(QUrl::RemoveQuery);
    urlString = m_url.toString(QUrl::RemoveQuery);

    if (urlString.contains(checkString))
        return true;
    QWebElementCollection collec = el.findAll("*");
    int i = 0;
    while (i < collec.count()) {
        QWebElement el = collec.at(i);
        checkString = QUrl(el.attribute("src")).toString(QUrl::RemoveQuery);
        urlString = m_url.toString(QUrl::RemoveQuery);
        if (urlString.contains(checkString))
            return true;
        i++;
    }
    return false;
}
