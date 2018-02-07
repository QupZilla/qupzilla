/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "mainapplication.h"
#include "tabbar.h"
#include "webtab.h"
#include "statusbar.h"
#include "progressbar.h"
#include "navigationbar.h"
#include "iconprovider.h"
#include "searchenginesmanager.h"
#include "enhancedmenu.h"
#include "locationbar.h"
#include "webhittestresult.h"
#include "webinspector.h"

#include <QHostInfo>
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

void TabbedWebView::setPage(WebPage* page)
{
    WebView::setPage(page);

    connect(page, &WebPage::linkHovered, this, &TabbedWebView::linkHovered);
}

BrowserWindow* TabbedWebView::browserWindow() const
{
    return m_window;
}

void TabbedWebView::setBrowserWindow(BrowserWindow* window)
{
    m_window = window;
}

void TabbedWebView::inspectElement()
{
    if (m_webTab->haveInspector())
        triggerPageAction(QWebEnginePage::InspectElement);
    else
        m_webTab->showWebInspector(true);
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
    Q_UNUSED(url)

    if (m_webTab->isCurrentTab() && m_window) {
        m_window->navigationBar()->refreshHistory();
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
    load(req);
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

void TabbedWebView::linkHovered(const QString &link)
{
    if (m_webTab->isCurrentTab() && m_window) {
        if (link.isEmpty()) {
            m_window->statusBar()->clearMessage();
        }
        else {
            m_window->statusBar()->showMessage(link);
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

void TabbedWebView::closeView()
{
    emit wantsCloseTab(tabIndex());
}

void TabbedWebView::loadInNewTab(const LoadRequest &req, Qz::NewTabPositionFlags position)
{
    if (m_window) {
        int index = m_window->tabWidget()->addView(QUrl(), position);
        TabbedWebView *view = m_window->weView(index);
        webTab()->addChildTab(view->webTab());
        view->webTab()->locationBar()->showUrl(req.url());
        view->load(req);
    }
}

bool TabbedWebView::isFullScreen()
{
    return m_window && m_window->isFullScreen();
}

void TabbedWebView::requestFullScreen(bool enable)
{
    if (!m_window)
        return;

    m_window->toggleHtmlFullScreen(enable);
}

void TabbedWebView::setAsCurrentTab()
{
    if (m_window) {
        m_window->tabWidget()->setCurrentWidget(m_webTab);
    }
}

void TabbedWebView::_contextMenuEvent(QContextMenuEvent *event)
{
    m_menu->clear();

    WebHitTestResult hitTest = page()->hitTestContent(event->pos());
    createContextMenu(m_menu, hitTest);

    if (WebInspector::isEnabled()) {
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

    WebView::_contextMenuEvent(event);
}

void TabbedWebView::_mouseMoveEvent(QMouseEvent *event)
{
    if (m_window && m_window->isFullScreen()) {
        if (m_window->fullScreenNavigationVisible()) {
            m_window->hideNavigationWithFullScreen();
        }
        else if (event->y() < 5) {
            m_window->showNavigationWithFullScreen();
        }
    }

    WebView::_mouseMoveEvent(event);
}
