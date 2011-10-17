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
#include "squeezelabelv2.h"

ClickToFlash::ClickToFlash(const QUrl &pluginUrl, const QStringList &argumentNames, const QStringList &argumentValues, QWidget* parent)
        : QWidget(parent)
        , m_argumentNames(argumentNames)
        , m_argumentValues(argumentValues)
        , m_toolButton(0)
        , m_layout1(0)
        , m_layout2(0)
        , m_frame(0)
        , m_url(pluginUrl)
{
    //AdBlock
    AdBlockManager* manager = AdBlockManager::instance();
    if (manager->isEnabled()) {
        QString urlString = pluginUrl.toEncoded();
        AdBlockSubscription* subscription = manager->subscription();
        if (!subscription->allow(urlString) && subscription->block(urlString)) {
            QTimer::singleShot(200, this, SLOT(hideAdBlocked()));
            return;
        }
    }

    m_layout1 = new QHBoxLayout(this);
    m_frame = new QFrame(this);
    m_frame->setObjectName("click2flash-frame");
    m_frame->setContentsMargins(0,0,0,0);
    m_layout2 = new QHBoxLayout(m_frame);
    m_toolButton = new QToolButton(this);
    m_toolButton->setObjectName("click2flash-toolbutton");

    m_toolButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_toolButton->setCursor(Qt::PointingHandCursor);
    m_layout2->addWidget(m_toolButton);
    m_layout1->addWidget(m_frame);
    m_layout1->setContentsMargins(0,0,0,0);
    m_layout2->setContentsMargins(0,0,0,0);

    connect(m_toolButton, SIGNAL(clicked()), this, SLOT(load()));
    setMinimumSize(27,27);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));
}

void ClickToFlash::customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    menu.addAction(tr("Object blocked by ClickToFlash"));
    menu.addAction(tr("Show more informations about object"), this, SLOT(showInfo()));
    menu.addSeparator();
    menu.addAction(tr("Delete object"), this, SLOT(hideAdBlocked()));
    menu.addAction(tr("Add %1 to whitelist").arg(m_url.host()), this, SLOT(toWhitelist()));
    menu.actions().at(0)->setEnabled(false);
    menu.exec(mapToGlobal(pos));
}

void ClickToFlash::toWhitelist()
{
    mApp->plugins()->c2f_addWhitelist(m_url.host());
    load();
}

void ClickToFlash::hideAdBlocked()
{
    findElement();
    if (!m_element.isNull())
        m_element.setAttribute("style", "display:none;");
    else
        hide();

    //deleteLater(); //Well, it should be there, but therefore it sometimes crashes
}

void ClickToFlash::findElement()
{
    if (!m_toolButton)
        return;

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

    QList<QWebFrame*> frames;
    frames.append(view->page()->frameAt(view->mapFromGlobal(m_toolButton->mapToGlobal(m_toolButton->pos()))));
    m_mainFrame = view->page()->mainFrame();
    frames.append(m_mainFrame);

    while (!frames.isEmpty()) {
        QWebFrame* frame = frames.takeFirst();
        if (!frame)
            continue;
        QWebElement docElement = frame->documentElement();

        QWebElementCollection elements;
        elements.append(docElement.findAll(QLatin1String("embed")));
        elements.append(docElement.findAll(QLatin1String("object")));

        QWebElement element;
        foreach (element, elements) {
            if (!checkElement(element) && !checkUrlOnElement(element))
                continue;
            m_element = element;
            return;
        }
        frames += frame->childFrames();
    }
}

void ClickToFlash::load()
{
    findElement();
    if (m_element.isNull()) {
        qWarning("Click2Flash: Cannot find Flash object.");
    } else {
        QWebElement substitute = m_element.clone();
        substitute.setAttribute(QLatin1String("type"), "application/futuresplash");
        m_element.replace(substitute);
    }
}

bool ClickToFlash::checkUrlOnElement(QWebElement el)
{
    QString checkString = QUrl(el.attribute("src")).toString(QUrl::RemoveQuery);
    if (checkString.isEmpty())
        checkString = QUrl(el.attribute("data")).toString(QUrl::RemoveQuery);
    if (checkString.isEmpty())
        checkString = QUrl(el.attribute("value")).toString(QUrl::RemoveQuery);

    if (m_url.toEncoded().contains(checkString.toAscii()))
        return true;
    return false;
}

bool ClickToFlash::checkElement(QWebElement el)
{
    if (m_argumentNames == el.attributeNames()) {
        foreach (QString name, m_argumentNames) {
            if (m_argumentValues.indexOf(el.attribute(name)) == -1)
                return false;
        }
        return true;
    }
    return false;
}

void ClickToFlash::showInfo()
{
    QWidget* widg = new QWidget();
    widg->setAttribute(Qt::WA_DeleteOnClose);
    widg->setWindowTitle(tr("Flash Object"));
    QFormLayout* lay = new QFormLayout(widg);

    lay->addRow(new QLabel(tr("<b>Attribute Name</b>")), new QLabel(tr("<b>Value</b>")));

    int i = 0;
    foreach (QString name, m_argumentNames) {
        QString value = m_argumentValues.at(i);
        lay->addRow(new SqueezeLabelV2(name), new SqueezeLabelV2(value));

        i++;
    }

    if (i == 0)
        lay->addRow(new QLabel(tr("No more informations available.")));

    widg->setMaximumHeight(500);
    widg->show();
}

ClickToFlash::~ClickToFlash()
{
    if (m_toolButton)
        delete m_toolButton;
    if (m_layout1)
        delete m_layout1;
    if (m_layout2)
        delete m_layout2;
    if (m_frame)
        delete m_frame;
}
