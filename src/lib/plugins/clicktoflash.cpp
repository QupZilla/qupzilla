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
#include "squeezelabelv2.h"
#include "webpage.h"
#include "qztools.h"
#include "qupzilla.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QFormLayout>
#include <QMenu>
#include <QTimer>
#include <QWebView>
#include <QNetworkRequest>
#include <QWebHitTestResult>

QUrl ClickToFlash::acceptedUrl;
QStringList ClickToFlash::acceptedArgNames;
QStringList ClickToFlash::acceptedArgValues;

ClickToFlash::ClickToFlash(const QUrl &pluginUrl, const QStringList &argumentNames, const QStringList &argumentValues, WebPage* parentPage)
    : QWidget()
    , m_argumentNames(argumentNames)
    , m_argumentValues(argumentValues)
    , m_toolButton(0)
    , m_layout1(0)
    , m_layout2(0)
    , m_frame(0)
    , m_url(pluginUrl)
    , m_page(parentPage)
{
    m_layout1 = new QHBoxLayout(this);
    m_frame = new QFrame(this);
    m_frame->setObjectName("click2flash-frame");
    m_frame->setContentsMargins(0, 0, 0, 0);
    m_layout2 = new QHBoxLayout(m_frame);
    m_toolButton = new QToolButton(this);
    m_toolButton->setObjectName("click2flash-toolbutton");

    m_toolButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_toolButton->setCursor(Qt::PointingHandCursor);
    m_layout2->addWidget(m_toolButton);
    m_layout1->addWidget(m_frame);
    m_layout1->setContentsMargins(0, 0, 0, 0);
    m_layout2->setContentsMargins(0, 0, 0, 0);

    connect(m_toolButton, SIGNAL(clicked()), this, SLOT(load()));
    setMinimumSize(27, 27);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));

    QTimer::singleShot(0, this, SLOT(ensurePluginVisible()));
}

bool ClickToFlash::isAlreadyAccepted(const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues)
{
    return (url == acceptedUrl &&
            argumentNames == acceptedArgNames &&
            argumentValues == acceptedArgValues);
}

void ClickToFlash::ensurePluginVisible()
{
    // Well, kind of a dirty workaround, but it works.
    // I don't know any other method how to show our plugin
    // and adjust it on the proper position in page

    // Scheduling adjustingPage rather than actually changing zoomFactor
    // right now, as it is CPU intensive when there is lot of click2flash
    // objects on page

    m_page->scheduleAdjustPage();
}

void ClickToFlash::customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    menu.addAction(tr("Object blocked by ClickToFlash"));
    menu.addAction(tr("Show more information about object"), this, SLOT(showInfo()));
    menu.addSeparator();
    menu.addAction(tr("Delete object"), this, SLOT(hideObject()));
    menu.addAction(tr("Add %1 to whitelist").arg(m_url.host()), this, SLOT(toWhitelist()));
    menu.actions().at(0)->setEnabled(false);
    menu.exec(mapToGlobal(pos));
}

void ClickToFlash::toWhitelist()
{
    mApp->plugins()->c2f_addWhitelist(m_url.host());
    load();
}

void ClickToFlash::hideObject()
{
    findElement();
    if (!m_element.isNull()) {
        m_element.setStyleProperty("visibility", "hidden");
    }
    else {
        hide();
    }

    //deleteLater(); //Well, it should be there, but therefore it sometimes crashes
}

void ClickToFlash::findElement()
{
    if (!m_toolButton) {
        return;
    }

    QWidget* parent = parentWidget();
    QWebView* view = 0;
    while (parent) {
        if (QWebView* aView = qobject_cast<QWebView*>(parent)) {
            view = aView;
            break;
        }
        parent = parent->parentWidget();
    }
    if (!view) {
        return;
    }

    QPoint objectPos = view->mapFromGlobal(m_toolButton->mapToGlobal(m_toolButton->pos()));
    QWebFrame* objectFrame = view->page()->frameAt(objectPos);
    QWebHitTestResult hitResult;
    QWebElement hitElement;

    if (objectFrame) {
        hitResult = objectFrame->hitTestContent(objectPos);
        hitElement = hitResult.element();
    }

    if (!hitElement.isNull() && (hitElement.tagName().compare("embed", Qt::CaseInsensitive) == 0 ||
                                 hitElement.tagName().compare("object", Qt::CaseInsensitive) == 0)) {
        m_element = hitElement;
        return;
    }

    // HitTestResult failed, trying to find element by src
    // attribute in elements at all frames on page (less accurate)

    QList<QWebFrame*> frames;
    frames.append(objectFrame);
    frames.append(view->page()->mainFrame());

    while (!frames.isEmpty()) {
        QWebFrame* frame = frames.takeFirst();
        if (!frame) {
            continue;
        }
        QWebElement docElement = frame->documentElement();

        QWebElementCollection elements;
        elements.append(docElement.findAll(QLatin1String("embed")));
        elements.append(docElement.findAll(QLatin1String("object")));

        foreach (const QWebElement &element, elements) {
            if (!checkElement(element) && !checkUrlOnElement(element)) {
                continue;
            }
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
    }
    else {
        /*
           Old code caused sometimes flashing of the whole browser window and then somehow
           ruined rendering of opacity effects, etc..

                QWebElement substitute = m_element.clone();
                substitute.setAttribute(QLatin1String("type"), "application/futuresplash");
                m_element.replace(substitute);

           So asynchronous JavaScript code is used to remove element from page and then substitute
           it with unblocked Flash. The JavaScript code is:

                var qz_c2f_clone = this.cloneNode(true);
                var qz_c2f_parentNode = this.parentNode;
                var qz_c2f_substituteElement = document.createElement(this.tagName);

                qz_c2f_substituteElement.width = this.width;
                qz_c2f_substituteElement.height = this.height;
                qz_c2f_substituteElement.type = "application/futuresplash";

                this.parentNode.replaceChild(qz_c2f_substituteElement, this);

                setTimeout(function(){
                    qz_c2f_parentNode.replaceChild(qz_c2f_clone, qz_c2f_substituteElement);
                }, 250);
        */

        acceptedUrl = m_url;
        acceptedArgNames = m_argumentNames;
        acceptedArgValues = m_argumentValues;

        QString js = "var qz_c2f_clone=this.cloneNode(true);var qz_c2f_parentNode=this.parentNode;"
                     "var qz_c2f_substituteElement=document.createElement(this.tagName);"
                     "qz_c2f_substituteElement.width=this.width;qz_c2f_substituteElement.height=this.height;"
                     "qz_c2f_substituteElement.type=\"application/futuresplash\";"
                     "this.parentNode.replaceChild(qz_c2f_substituteElement,this);setTimeout(function(){"
                     "qz_c2f_parentNode.replaceChild(qz_c2f_clone,qz_c2f_substituteElement);},250);";

        m_element.evaluateJavaScript(js);
    }
}

bool ClickToFlash::checkUrlOnElement(QWebElement el)
{
    QString checkString = el.attribute("src");
    if (checkString.isEmpty()) {
        checkString = el.attribute("data");
    }
    if (checkString.isEmpty()) {
        checkString = el.attribute("value");
    }

    checkString = m_page->url().resolved(QUrl(checkString)).toString(QUrl::RemoveQuery);

    return m_url.toEncoded().contains(checkString.toUtf8());
}

bool ClickToFlash::checkElement(QWebElement el)
{
    if (m_argumentNames == el.attributeNames()) {
        foreach (const QString &name, m_argumentNames) {
            if (m_argumentValues.indexOf(el.attribute(name)) == -1) {
                return false;
            }
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
    QLabel* attrib = new QLabel(tr("<b>Attribute Name</b>"));
    QLabel* value = new QLabel(tr("<b>Value</b>"));
    if (isRightToLeft()) {
        widg->setLayoutDirection(Qt::LeftToRight);
        attrib->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    lay->addRow(attrib, value);

    int i = 0;
    foreach (const QString &name, m_argumentNames) {
        QString value = m_argumentValues.at(i);
        SqueezeLabelV2* valueLabel = new SqueezeLabelV2(value);
        valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);

        lay->addRow(new SqueezeLabelV2(name), valueLabel);

        i++;
    }

    if (i == 0) {
        lay->addRow(new QLabel(tr("No more information available.")));
    }

    widg->setMaximumHeight(500);
    QzTools::centerWidgetToParent(widg, m_page->view());
    widg->show();
}
