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

TabbedWebView::TabbedWebView(BrowserWindow* window, WebTab* webTab)
    : WebView(webTab)
    , m_window(window)
    , m_webTab(webTab)
    , m_menu(new Menu(this))
{
    m_menu->setCloseOnMiddleClick(true);

    connect(this, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished()));

    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged(QUrl)));
    connect(this, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged()));

    connect(this, SIGNAL(statusBarMessage(QString)), m_window->statusBar(), SLOT(showMessage(QString)));
}

TabbedWebView::~TabbedWebView()
{
}

void TabbedWebView::setWebPage(WebPage* page)
{
    page->setWebView(this);
    page->setParent(this);
    setPage(page);

    connect(page, SIGNAL(linkHovered(QString,QString,QString)), this, SLOT(linkHovered(QString,QString,QString)));
}

void TabbedWebView::inspectElement()
{
    m_window->showWebInspector(false);
    triggerPageAction(QWebPage::InspectElement);
}

WebTab* TabbedWebView::webTab() const
{
    return m_webTab;
}

TabWidget* TabbedWebView::tabWidget() const
{
    return m_window->tabWidget();
}

QString TabbedWebView::getIp() const
{
    return m_currentIp;
}

void TabbedWebView::urlChanged(const QUrl &url)
{
    if (m_webTab->isCurrentTab()) {
        m_window->navigationBar()->refreshHistory();
    }

    if (lastUrl() != url) {
        emit changed();
    }
}

void TabbedWebView::loadProgress(int prog)
{
    Q_UNUSED(prog)

    if (m_webTab->isCurrentTab()) {
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
    if (title().isNull()) {
        m_webTab->setTabTitle(tr("Loading..."));
    }

    m_currentIp.clear();
}

void TabbedWebView::slotLoadFinished()
{
    QHostInfo::lookupHost(url().host(), this, SLOT(setIp(QHostInfo)));

    if (m_webTab->isCurrentTab()) {
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

void TabbedWebView::titleChanged()
{
    if (m_webTab->isCurrentTab()) {
        m_window->setWindowTitle(tr("%1 - QupZilla").arg(title()));
    }

    m_webTab->setTabTitle(title());
}

void TabbedWebView::linkHovered(const QString &link, const QString &title, const QString &content)
{
    Q_UNUSED(title)
    Q_UNUSED(content)

    if (m_webTab->isCurrentTab()) {
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
    return tabWidget()->indexOf(m_webTab);
}

BrowserWindow* TabbedWebView::mainWindow() const
{
    return m_window;
}

void TabbedWebView::moveToWindow(BrowserWindow* window)
{
    disconnect(this, SIGNAL(statusBarMessage(QString)), m_window->statusBar(), SLOT(showMessage(QString)));

    m_window = window;

    connect(this, SIGNAL(statusBarMessage(QString)), m_window->statusBar(), SLOT(showMessage(QString)));
}

QWidget* TabbedWebView::overlayForJsAlert()
{
    return m_webTab;
}

void TabbedWebView::contextMenuEvent(QContextMenuEvent* event)
{
    m_menu->clear();

    const QWebHitTestResult hitTest = page()->mainFrame()->hitTestContent(event->pos());

    createContextMenu(m_menu, hitTest, event->pos());

    if (!hitTest.isContentEditable() && !hitTest.isContentSelected()) {
        m_menu->addAction(m_window->adBlockIcon()->menuAction());
    }

    m_menu->addSeparator();
    m_menu->addAction(tr("Inspect Element"), this, SLOT(inspectElement()));

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
    tabWidget()->addView(QUrl());
}

void TabbedWebView::loadInNewTab(const LoadRequest &req, Qz::NewTabPositionFlags position)
{
    int index = tabWidget()->addView(QUrl(), position);
    m_window->weView(index)->webTab()->locationBar()->showUrl(req.url());
    m_window->weView(index)->load(req);
}

void TabbedWebView::setAsCurrentTab()
{
    tabWidget()->setCurrentWidget(m_webTab);
}

void TabbedWebView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_window->isFullScreen()) {
        if (m_window->fullScreenNavigationVisible()) {
            m_window->hideNavigationWithFullScreen();
        }
        else if (event->y() < 5) {
            m_window->showNavigationWithFullScreen();
        }
    }

    WebView::mouseMoveEvent(event);
}
