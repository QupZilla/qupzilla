/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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

TabbedWebView::TabbedWebView(QupZilla* mainClass, WebTab* webTab)
    : WebView(webTab)
    , p_QupZilla(mainClass)
    , m_tabWidget(p_QupZilla->tabWidget())
    , m_page(0)
    , m_webTab(webTab)
    , m_menu(new Menu(this))
    , m_mouseTrack(false)
    , m_navigationVisible(false)
    , m_hasRss(false)
    , m_rssChecked(false)
{
    connect(this, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(loadingProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished()));

    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged(QUrl)));
    connect(this, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged()));
    connect(this, SIGNAL(iconChanged()), this, SLOT(slotIconChanged()));

    connect(this, SIGNAL(statusBarMessage(QString)), p_QupZilla->statusBar(), SLOT(showMessage(QString)));

    connect(mApp->networkManager(), SIGNAL(wantsFocus(QUrl)), this, SLOT(getFocus(QUrl)));

    connect(p_QupZilla, SIGNAL(setWebViewMouseTracking(bool)), this, SLOT(trackMouse(bool)));

    // Tracking mouse also on tabs created in fullscreen
    trackMouse(p_QupZilla->isFullScreen());
}

void TabbedWebView::setWebPage(WebPage* page)
{
    if (m_page == page) {
        return;
    }

    if (m_page) {
        delete m_page;
        m_page = 0;
    }

    m_page = page;
    m_page->setWebView(this);
    m_page->setParent(this);
    setPage(m_page);

    connect(m_page, SIGNAL(linkHovered(QString, QString, QString)), this, SLOT(linkHovered(QString, QString, QString)));
}

void TabbedWebView::slotIconChanged()
{
    const QString &urlScheme = url().scheme();

    if (urlScheme == "file" || urlScheme == "qupzilla" || title().contains(tr("Failed loading page"))) {
        return;
    }

    mApp->iconProvider()->saveIcon(this);

    showIcon();
}

void TabbedWebView::inspectElement()
{
    p_QupZilla->showWebInspector(false);
    triggerPageAction(QWebPage::InspectElement);
}

WebPage* TabbedWebView::webPage() const
{
    return m_page;
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

void TabbedWebView::loadingProgress(int prog)
{
    if (prog > 60) {
        checkRss();
    }

    if (isCurrent()) {
        p_QupZilla->updateLoadingActions();
    }
}

void TabbedWebView::slotLoadStarted()
{
    m_rssChecked = false;
    emit rssChanged(false);

    animationLoading(tabIndex(), true);

    if (title().isNull()) {
        m_tabWidget->setTabText(tabIndex(), tr("Loading..."));
    }

    m_currentIp.clear();
}

void TabbedWebView::slotLoadFinished()
{
    QMovie* mov = animationLoading(tabIndex(), false)->movie();
    if (mov) {
        mov->stop();
    }

    showIcon();
    QHostInfo::lookupHost(url().host(), this, SLOT(setIp(QHostInfo)));

    if (isCurrent()) {
        p_QupZilla->updateLoadingActions();
    }
}

QLabel* TabbedWebView::animationLoading(int index, bool addMovie)
{
    if (index == -1) {
        return 0;
    }

    QLabel* loadingAnimation = qobject_cast<QLabel*>(m_tabWidget->getTabBar()->tabButton(index, QTabBar::LeftSide));
    if (!loadingAnimation) {
        loadingAnimation = new QLabel();
    }
    if (addMovie && !loadingAnimation->movie()) {
        QMovie* movie = new QMovie(":icons/other/progress.gif", QByteArray(), loadingAnimation);
        movie->setSpeed(70);
        loadingAnimation->setMovie(movie);
        movie->start();
    }
    else if (loadingAnimation->movie()) {
        loadingAnimation->movie()->stop();
    }

    m_tabWidget->getTabBar()->setTabButton(index, QTabBar::LeftSide, 0);
    m_tabWidget->getTabBar()->setTabButton(index, QTabBar::LeftSide, loadingAnimation);
    return loadingAnimation;
}

void TabbedWebView::stopAnimation()
{
    QMovie* mov = animationLoading(tabIndex(), false)->movie();
    if (mov) {
        mov->stop();
    }

    showIcon();
}

void TabbedWebView::setIp(const QHostInfo &info)
{
    if (info.addresses().isEmpty()) {
        return;
    }

    m_currentIp = info.hostName() + " (" + info.addresses().at(0).toString() + ")";

    if (isCurrent()) {
        emit ipChanged(m_currentIp);
    }
}

void TabbedWebView::titleChanged()
{
    const QString &t = title();
    m_tabWidget->setTabToolTip(tabIndex(), t);

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
    if (!icon_.isNull()) {
        animationLoading(tabIndex(), false)->setPixmap(icon_.pixmap(16, 16));
    }
    else {
        animationLoading(tabIndex(), false)->setPixmap(IconProvider::fromTheme("text-plain").pixmap(16, 16));
    }
}

void TabbedWebView::linkHovered(const QString &link, const QString &title, const QString &content)
{
    Q_UNUSED(title)
    Q_UNUSED(content)

    if (isCurrent()) {
        if (link != "") {
            p_QupZilla->statusBarMessage()->showMessage(link);
        }
        else {
            p_QupZilla->statusBarMessage()->clearMessage();
        }
    }
    m_hoveredLink = link;
}

// FIXME: Don't do this magic to get index of tab.
// Implement setTabIndex() and call it from TabWidget (when creating and also from
// tabMoved slot)
int TabbedWebView::tabIndex() const
{
    int i = 0;
    while (WebTab* wTab = qobject_cast<WebTab*>(m_tabWidget->widget(i))) {
        if (wTab && wTab->view() == this) {
            break;
        }
        i++;
    }
    return i;
}

QWidget* TabbedWebView::overlayForJsAlert()
{
    return m_webTab;
}

void TabbedWebView::closeView()
{
    emit wantsCloseTab(tabIndex());
}

void TabbedWebView::checkRss()
{
    if (m_rssChecked) {
        return;
    }

    m_rssChecked = true;
    QWebFrame* frame = page()->mainFrame();
    const QWebElementCollection &links = frame->findAllElements("link[type=\"application/rss+xml\"]");

    m_hasRss = links.count() != 0;
    emit rssChanged(m_hasRss);
}

void TabbedWebView::contextMenuEvent(QContextMenuEvent* event)
{
    m_menu->clear();

    const QWebHitTestResult &hitTest = page()->mainFrame()->hitTestContent(event->pos());

    createContextMenu(m_menu, hitTest, event->pos());
    m_menu->addAction(p_QupZilla->adBlockIcon()->menuAction());

    m_menu->addSeparator();
    m_menu->addAction(tr("Inspect Element"), this, SLOT(inspectElement()));

    if (!m_menu->isEmpty()) {
        //Prevent choosing first option with double rightclick
        const QPoint &pos = mapToGlobal(event->pos());
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

void TabbedWebView::openUrlInNewTab(const QUrl &url, Qz::NewTabPositionFlag position)
{
    m_tabWidget->addView(url, position);
}

void TabbedWebView::openNewTab()
{
    m_tabWidget->addView();
}

void TabbedWebView::getFocus(const QUrl &urla)
{
    if (urla == url()) {
        m_tabWidget->setCurrentWidget(m_webTab);
    }
}

void TabbedWebView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_mouseTrack) {
        if (m_navigationVisible) {
            m_navigationVisible = false;
            p_QupZilla->showNavigationWithFullscreen();
        }
        else if (event->y() < 5) {
            m_navigationVisible = true;
            p_QupZilla->showNavigationWithFullscreen();
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

void TabbedWebView::fakePageLoading(int progress)
{
    WebView::slotLoadStarted();
    slotLoadStarted();
    loadingProgress(progress);
}

TabbedWebView::~TabbedWebView()
{
}
