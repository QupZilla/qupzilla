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
#include "tabbedwebview.h"
#include "browserwindow.h"
#include "webpage.h"
#include "tabwidget.h"
#include "networkmanager.h"
#include "mainapplication.h"
#include "tabbar.h"
#include "webtab.h"
#include "statusbarmessage.h"
#include "progressbar.h"
#include "navigationbar.h"
#include "iconprovider.h"
#include "searchenginesmanager.h"
#include "enhancedmenu.h"
#include "adblockicon.h"
#include "locationbar.h"

#include <QMovie>
#include <QStatusBar>
#include <QHostInfo>
#include <QWebFrame>
#include <QContextMenuEvent>

TabbedWebView::TabbedWebView(WebTab* webTab)
    : WebView(webTab)
    , m_window(0)
    , m_webTab(webTab)
    , m_menu(new Menu(this))
{
    m_menu->setCloseOnMiddleClick(true);

    connect(this, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(slotLoadProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished()));
    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged(QUrl)));
}

void TabbedWebView::setWebPage(WebPage* page)
{
    page->setWebView(this);
    page->setParent(this);
    setPage(page);

    connect(page, SIGNAL(linkHovered(QString,QString,QString)), this, SLOT(linkHovered(QString,QString,QString)));
}

BrowserWindow* TabbedWebView::browserWindow() const
{
    return m_window;
}

void TabbedWebView::setBrowserWindow(BrowserWindow* window)
{
    if (m_window) {
        disconnect(this, SIGNAL(statusBarMessage(QString)), m_window->statusBar(), SLOT(showMessage(QString)));
    }

    m_window = window;

    if (m_window) {
        connect(this, SIGNAL(statusBarMessage(QString)), m_window->statusBar(), SLOT(showMessage(QString)));
    }
}

void TabbedWebView::inspectElement()
{
    m_webTab->showWebInspector();
    triggerPageAction(QWebPage::InspectElement);
}

WebTab* TabbedWebView::webTab() const
{
    return m_webTab;
}

QString TabbedWebView::getIp() const
{
    return m_currentIp;
}

void TabbedWebView::urlChanged(const QUrl &url)
{
    if (m_webTab->isCurrentTab() && m_window) {
        m_window->navigationBar()->refreshHistory();
    }

    if (lastUrl() != url) {
        emit changed();
    }
}

void TabbedWebView::slotLoadProgress(int prog)
{
    Q_UNUSED(prog)

    if (m_webTab->isCurrentTab() && m_window) {
        m_window->updateLoadingActions();
    }
}

void TabbedWebView::userLoadAction(const LoadRequest &req)
{
    QNetworkRequest request(req.networkRequest());
    request.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));

    LoadRequest r = req;
    r.setNetworkRequest(request);

    load(r);
}

void TabbedWebView::slotLoadStarted()
{
    m_currentIp.clear();
}

void TabbedWebView::slotLoadFinished()
{
    QHostInfo::lookupHost(url().host(), this, SLOT(setIp(QHostInfo)));

    if (m_webTab->isCurrentTab() && m_window) {
        m_window->updateLoadingActions();
    }
}

void TabbedWebView::setIp(const QHostInfo &info)
{
    if (info.addresses().isEmpty()) {
        return;
    }

    m_currentIp = QString("%1 (%2)").arg(info.hostName(), info.addresses().at(0).toString());

    if (m_webTab->isCurrentTab()) {
        emit ipChanged(m_currentIp);
    }
}

void TabbedWebView::linkHovered(const QString &link, const QString &title, const QString &content)
{
    Q_UNUSED(title)
    Q_UNUSED(content)

    if (m_webTab->isCurrentTab() && m_window) {
        if (link.isEmpty()) {
            m_window->statusBarMessage()->clearMessage();
        }
        else {
            // QUrl::fromEncoded(link.toUtf8());
            // Don't decode link from percent encoding (to show all utf8 chars), as it doesn't
            // works correctly in all cases
            // See #1095
            m_window->statusBarMessage()->showMessage(link);
        }
    }
}

int TabbedWebView::tabIndex() const
{
    return m_webTab->tabIndex();
}

QWidget* TabbedWebView::overlayWidget()
{
    return m_webTab;
}

void TabbedWebView::contextMenuEvent(QContextMenuEvent* event)
{
    m_menu->clear();

    const QWebHitTestResult hitTest = page()->mainFrame()->hitTestContent(event->pos());
    const QUrl pageUrl = page()->url();

    createContextMenu(m_menu, hitTest, event->pos());

    if (!hitTest.isContentEditable() && !hitTest.isContentSelected() && pageUrl.scheme() != QLatin1String("qupzilla") && m_window) {
        m_menu->addAction(m_window->adBlockIcon()->menuAction());
    }

    if (pageUrl.scheme() != QLatin1String("qupzilla")) {
        m_menu->addSeparator();
        m_menu->addAction(tr("Inspect Element"), this, SLOT(inspectElement()));
    }

    if (!m_menu->isEmpty()) {
        // Prevent choosing first option with double rightclick
        const QPoint pos = event->globalPos();
        QPoint p(pos.x(), pos.y() + 1);

        m_menu->popup(p);
        return;
    }

    WebView::contextMenuEvent(event);
}

void TabbedWebView::closeView()
{
    emit wantsCloseTab(tabIndex());
}

void TabbedWebView::openNewTab()
{
    if (m_window) {
        m_window->tabWidget()->addView(QUrl());
    }
}

void TabbedWebView::loadInNewTab(const LoadRequest &req, Qz::NewTabPositionFlags position)
{
    if (m_window) {
        int index = m_window->tabWidget()->addView(QUrl(), position);
        m_window->weView(index)->webTab()->locationBar()->showUrl(req.url());
        m_window->weView(index)->load(req);
    }
}

void TabbedWebView::setAsCurrentTab()
{
    if (m_window) {
        m_window->tabWidget()->setCurrentWidget(m_webTab);
    }
}

void TabbedWebView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_window && m_window->isFullScreen()) {
        if (m_window->fullScreenNavigationVisible()) {
            m_window->hideNavigationWithFullScreen();
        }
        else if (event->y() < 5) {
            m_window->showNavigationWithFullScreen();
        }
    }

    WebView::mouseMoveEvent(event);
}
