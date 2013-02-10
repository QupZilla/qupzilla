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
#include "tabbedwebview.h"
#include "qupzilla.h"
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

#include <QMovie>
#include <QStatusBar>
#include <QHostInfo>
#include <QWebFrame>
#include <QContextMenuEvent>

TabbedWebView::TabbedWebView(QupZilla* mainClass, WebTab* webTab)
    : WebView(webTab)
    , p_QupZilla(mainClass)
    , m_tabWidget(p_QupZilla->tabWidget())
    , m_webTab(webTab)
    , m_menu(new Menu(this))
    , m_mouseTrack(false)
{
    connect(this, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished()));

    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged(QUrl)));
    connect(this, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged()));
    connect(this, SIGNAL(iconChanged()), this, SLOT(slotIconChanged()));

    connect(this, SIGNAL(statusBarMessage(QString)), p_QupZilla->statusBar(), SLOT(showMessage(QString)));

    connect(p_QupZilla, SIGNAL(setWebViewMouseTracking(bool)), this, SLOT(trackMouse(bool)));

    // Tracking mouse also on tabs created in fullscreen
    trackMouse(p_QupZilla->isFullScreen());
}

void TabbedWebView::setWebPage(WebPage* page)
{
    page->setWebView(this);
    page->setParent(this);
    setPage(page);

    connect(page, SIGNAL(linkHovered(QString, QString, QString)), this, SLOT(linkHovered(QString, QString, QString)));
}

void TabbedWebView::slotIconChanged()
{
    const QString &urlScheme = url().scheme();

    if (urlScheme == QLatin1String("file") || urlScheme == QLatin1String("qupzilla")
            || title().contains(tr("Failed loading page"))) {
        return;
    }

    showIcon();
}

void TabbedWebView::inspectElement()
{
    p_QupZilla->showWebInspector(false);
    triggerPageAction(QWebPage::InspectElement);
}

WebTab* TabbedWebView::webTab() const
{
    return m_webTab;
}

TabWidget* TabbedWebView::tabWidget() const
{
    return m_tabWidget;
}

QString TabbedWebView::getIp() const
{
    return m_currentIp;
}

bool TabbedWebView::isCurrent()
{
    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_tabWidget->currentIndex()));
    if (!webTab) {
        return false;
    }

    return (webTab->view() == this);
}

void TabbedWebView::urlChanged(const QUrl &url)
{
    if (isCurrent()) {
        p_QupZilla->navigationBar()->refreshHistory();
    }

    if (lastUrl() != url) {
        emit changed();
    }
}

void TabbedWebView::loadProgress(int prog)
{
    Q_UNUSED(prog)

    if (isCurrent()) {
        p_QupZilla->updateLoadingActions();
    }
}

void TabbedWebView::userLoadAction(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));

    load(request);
}

void TabbedWebView::slotLoadStarted()
{
    m_tabWidget->startTabAnimation(tabIndex());

    if (title().isNull()) {
        m_tabWidget->setTabText(tabIndex(), tr("Loading..."));
    }

    m_currentIp.clear();
}

void TabbedWebView::slotLoadFinished()
{
    m_tabWidget->stopTabAnimation(tabIndex());

    showIcon();
    QHostInfo::lookupHost(url().host(), this, SLOT(setIp(QHostInfo)));

    if (isCurrent()) {
        p_QupZilla->updateLoadingActions();
    }
}

void TabbedWebView::setIp(const QHostInfo &info)
{
    if (info.addresses().isEmpty()) {
        return;
    }

    m_currentIp = QString("%1 (%2)").arg(info.hostName(), info.addresses().at(0).toString());

    if (isCurrent()) {
        emit ipChanged(m_currentIp);
    }
}

void TabbedWebView::titleChanged()
{
    const QString &t = title();

    if (isCurrent()) {
        p_QupZilla->setWindowTitle(tr("%1 - QupZilla").arg(t));
    }

    m_tabWidget->setTabText(tabIndex(), t);
}

void TabbedWebView::showIcon()
{
    if (isLoading()) {
        return;
    }

    QIcon icon_ = icon();
    if (icon_.isNull()) {
        icon_ = qIconProvider->emptyWebIcon();
    }

    m_tabWidget->setTabIcon(tabIndex(), icon_);
}

void TabbedWebView::linkHovered(const QString &link, const QString &title, const QString &content)
{
    Q_UNUSED(title)
    Q_UNUSED(content)

    if (isCurrent()) {
        if (link.isEmpty()) {
            p_QupZilla->statusBarMessage()->clearMessage();
        }
        else {
            p_QupZilla->statusBarMessage()->showMessage(link);
        }
    }
}

int TabbedWebView::tabIndex() const
{
    return m_tabWidget->indexOf(m_webTab);
}

QWidget* TabbedWebView::overlayForJsAlert()
{
    return m_webTab;
}

void TabbedWebView::closeView()
{
    emit wantsCloseTab(tabIndex());
}

void TabbedWebView::contextMenuEvent(QContextMenuEvent* event)
{
    m_menu->clear();

    const QWebHitTestResult &hitTest = page()->mainFrame()->hitTestContent(event->pos());

    createContextMenu(m_menu, hitTest, event->pos());

    if (!hitTest.isContentEditable() && !hitTest.isContentSelected()) {
        m_menu->addAction(p_QupZilla->adBlockIcon()->menuAction());
    }

    m_menu->addSeparator();
    m_menu->addAction(tr("Inspect Element"), this, SLOT(inspectElement()));

    if (!m_menu->isEmpty()) {
        // Prevent choosing first option with double rightclick
        const QPoint &pos = event->globalPos();
        QPoint p(pos.x(), pos.y() + 1);

        m_menu->popup(p);
        return;
    }

    WebView::contextMenuEvent(event);
}

void TabbedWebView::stop()
{
    triggerPageAction(QWebPage::Stop);
    slotLoadFinished();
}

void TabbedWebView::openUrlInNewTab(const QUrl &urla, Qz::NewTabPositionFlag position)
{
    QNetworkRequest req(urla);
    req.setRawHeader("Referer", url().toEncoded());
    req.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));

    m_tabWidget->addView(req, position);
}

void TabbedWebView::openNewTab()
{
    m_tabWidget->addView(QUrl());
}

void TabbedWebView::setAsCurrentTab()
{
    m_tabWidget->setCurrentWidget(m_webTab);
}

void TabbedWebView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_mouseTrack) {
        if (p_QupZilla->fullScreenNavigationVisible()) {
            p_QupZilla->hideNavigationWithFullScreen();
        }
        else if (event->y() < 5) {
            p_QupZilla->showNavigationWithFullScreen();
        }
    }

    WebView::mouseMoveEvent(event);
}

void TabbedWebView::disconnectObjects()
{
    disconnect(this);
    disconnect(p_QupZilla->statusBar());

    WebView::disconnectObjects();
}

TabbedWebView::~TabbedWebView()
{
}
