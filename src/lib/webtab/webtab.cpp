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
#include "webtab.h"
#include "browserwindow.h"
#include "tabbedwebview.h"
#include "webinspector.h"
#include "webpage.h"
#include "tabbar.h"
#include "tabicon.h"
#include "tabwidget.h"
#include "locationbar.h"
#include "qztools.h"
#include "qzsettings.h"
#include "mainapplication.h"

#include <QVBoxLayout>
#include <QWebHistory>
#include <QWebFrame>
#include <QLabel>
#include <QTimer>
#include <QSplitter>

bool WebTab::s_pinningTab = false;
static const int savedTabVersion = 2;

WebTab::SavedTab::SavedTab()
    : isPinned(false)
{
}

WebTab::SavedTab::SavedTab(WebTab* webTab)
{
    title = webTab->title();
    url = webTab->url();
    icon = webTab->icon();
    history = webTab->historyData();
    isPinned = webTab->isPinned();
}

bool WebTab::SavedTab::isValid() const
{
    return !url.isEmpty();
}

void WebTab::SavedTab::clear()
{
    title.clear();
    url.clear();
    icon = QIcon();
    history.clear();
    isPinned = false;
}

QDataStream &operator <<(QDataStream &stream, const WebTab::SavedTab &tab)
{
    stream << savedTabVersion;
    stream << tab.title;
    stream << tab.url;
    stream << tab.icon.pixmap(16);
    stream << tab.history;
    stream << tab.isPinned;

    return stream;
}

QDataStream &operator >>(QDataStream &stream, WebTab::SavedTab &tab)
{
    int version;
    stream >> version;

    if (version < 1)
        return stream;

    QPixmap pixmap;
    stream >> tab.title;
    stream >> tab.url;
    stream >> pixmap;
    stream >> tab.history;

    if (version >= 2)
        stream >> tab.isPinned;

    tab.icon = QIcon(pixmap);

    return stream;
}

WebTab::WebTab(BrowserWindow* window)
    : QWidget()
    , m_window(window)
    , m_inspector(0)
    , m_tabBar(0)
    , m_isPinned(false)
{
    setObjectName(QSL("webtab"));

    // This fixes background of pages with dark themes
    setStyleSheet("#webtab {background-color:white;}");

    m_webView = new TabbedWebView(this);
    m_webView->setBrowserWindow(m_window);
    m_webView->setWebPage(new WebPage(this));
    m_webView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    m_locationBar = new LocationBar(m_window);
    m_locationBar->setWebView(m_webView);

    m_tabIcon = new TabIcon(this);
    m_tabIcon->setWebTab(this);

    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->addWidget(m_webView);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_splitter);
    setLayout(m_layout);

    connect(m_webView, SIGNAL(showNotification(QWidget*)), this, SLOT(showNotification(QWidget*)));
    connect(m_webView, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(m_webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
    connect(m_webView, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged(QString)));
}

TabbedWebView* WebTab::webView() const
{
    return m_webView;
}

void WebTab::showWebInspector()
{
    if (!m_inspector) {
        m_inspector = new WebInspector(this);
        m_inspector->setPage(m_webView->page());
        m_splitter->addWidget(m_inspector);
    }

    m_inspector->show();
}

void WebTab::toggleWebInspector()
{
    if (!m_inspector || m_inspector->isHidden())
        showWebInspector();
    else
        m_inspector->hideInspector();
}

QUrl WebTab::url() const
{
    if (isRestored()) {
        return m_webView->url();
    }
    else {
        return m_savedTab.url;
    }
}

QString WebTab::title() const
{
    if (isRestored()) {
        return m_webView->title();
    }
    else {
        return m_savedTab.title;
    }
}

QIcon WebTab::icon() const
{
    if (isRestored()) {
        return m_webView->icon();
    }
    else {
        return m_savedTab.icon;
    }
}

QWebHistory* WebTab::history() const
{
    return m_webView->history();
}

void WebTab::detach()
{
    Q_ASSERT(m_tabBar);

    // Remove icon from tab
    m_tabBar->setTabButton(tabIndex(), m_tabBar->iconButtonPosition(), 0);

    // Remove the tab from tabbar
    setParent(0);
    // Remove the locationbar from window
    m_locationBar->setParent(this);
    // Detach TabbedWebView
    m_webView->setBrowserWindow(0);

    // WebTab is now standalone widget
    m_window = 0;
    m_tabBar = 0;
}

void WebTab::attach(BrowserWindow* window)
{
    m_window = window;
    m_tabBar = m_window->tabWidget()->tabBar();

    m_webView->setBrowserWindow(m_window);
    m_tabBar->setTabButton(tabIndex(), m_tabBar->iconButtonPosition(), m_tabIcon);
    m_tabBar->setTabText(tabIndex(), title());
}

void WebTab::setHistoryData(const QByteArray &data)
{
    QDataStream historyStream(data);
    historyStream >> *m_webView->history();
}

QByteArray WebTab::historyData() const
{
    if (isRestored()) {
        QByteArray historyArray;
        QDataStream historyStream(&historyArray, QIODevice::WriteOnly);
        historyStream << *m_webView->history();
        return historyArray;
    }
    else {
        return m_savedTab.history;
    }
}

void WebTab::reload()
{
    m_webView->reload();
}

void WebTab::stop()
{
    m_webView->stop();
}

bool WebTab::isLoading() const
{
    return m_webView->isLoading();
}

bool WebTab::isPinned() const
{
    return m_isPinned;
}

void WebTab::setPinned(bool state)
{
    m_isPinned = state;
}

LocationBar* WebTab::locationBar() const
{
    return m_locationBar;
}

TabIcon* WebTab::tabIcon() const
{
    return m_tabIcon;
}

bool WebTab::isRestored() const
{
    return !m_savedTab.isValid();
}

void WebTab::restoreTab(const WebTab::SavedTab &tab)
{
    Q_ASSERT(m_tabBar);

    m_isPinned = tab.isPinned;

    if (!m_isPinned && qzSettings->loadTabsOnActivation) {
        m_savedTab = tab;
        int index = tabIndex();

        m_tabBar->setTabText(index, tab.title);
        m_locationBar->showUrl(tab.url);
        m_tabIcon->setIcon(tab.icon);

        if (!tab.url.isEmpty()) {
            QColor col = m_tabBar->palette().text().color();
            QColor newCol = col.lighter(250);

            // It won't work for black color because (V) = 0
            // It won't also work for white, as white won't get any lighter
            if (col == Qt::black || col == Qt::white) {
                newCol = Qt::gray;
            }

            m_tabBar->overrideTabTextColor(index, newCol);
        }
    }
    else {
        p_restoreTab(tab);
    }
}

void WebTab::p_restoreTab(const QUrl &url, const QByteArray &history)
{
    m_webView->load(url);

    QDataStream historyStream(history);
    historyStream >> *m_webView->history();
}

void WebTab::p_restoreTab(const WebTab::SavedTab &tab)
{
    p_restoreTab(tab.url, tab.history);
}

QPixmap WebTab::renderTabPreview()
{
    WebPage* page = m_webView->page();
    const QSize oldSize = page->viewportSize();
    const QPoint originalScrollPosition = page->mainFrame()->scrollPosition();

    // Hack to ensure rendering the same preview before and after the page was shown for the first time
    // This can occur eg. with opening background tabs
    TabbedWebView* currentWebView = m_window ? m_window->weView() : 0;
    if (currentWebView) {
        page->setViewportSize(currentWebView->size());
    }

    const int previewWidth = 230;
    const int previewHeight = 150;
    const int scrollBarExtent = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const int pageWidth = qMin(page->mainFrame()->contentsSize().width(), 1280);
    const int pageHeight = (pageWidth / 23 * 15);
    const qreal scalingFactor = 2 * static_cast<qreal>(previewWidth) / pageWidth;

    page->setViewportSize(QSize(pageWidth, pageHeight));

    QPixmap pageImage((2 * previewWidth) - scrollBarExtent, (2 * previewHeight) - scrollBarExtent);
    pageImage.fill(Qt::transparent);

    QPainter p(&pageImage);
    p.scale(scalingFactor, scalingFactor);
    m_webView->page()->mainFrame()->render(&p, QWebFrame::ContentsLayer);
    p.end();

    page->setViewportSize(oldSize);

    // Restore also scrollbar positions, to prevent messing scrolling to anchor links
    page->mainFrame()->setScrollBarValue(Qt::Vertical, originalScrollPosition.y());
    page->mainFrame()->setScrollBarValue(Qt::Horizontal, originalScrollPosition.x());

    return pageImage.scaled(previewWidth, previewHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void WebTab::showNotification(QWidget* notif)
{
    const int notifPos = 0;

    if (m_layout->count() > notifPos + 1) {
        delete m_layout->itemAt(notifPos)->widget();
    }

    m_layout->insertWidget(notifPos, notif);
    notif->show();
}

void WebTab::loadStarted()
{
    if (m_tabBar && m_webView->isTitleEmpty()) {
        m_tabBar->setTabText(tabIndex(), tr("Loading..."));
    }
}

void WebTab::loadFinished()
{
    titleChanged(m_webView->title());
}

void WebTab::titleChanged(const QString &title)
{
    if (!m_tabBar || !m_window || title.isEmpty()) {
        return;
    }

    if (isCurrentTab()) {
        m_window->setWindowTitle(tr("%1 - QupZilla").arg(title));
    }

    m_tabBar->setTabText(tabIndex(), title);
}

void WebTab::slotRestore()
{
    Q_ASSERT(m_tabBar);

    p_restoreTab(m_savedTab);
    m_savedTab.clear();

    m_tabBar->restoreTabTextColor(tabIndex());
}

void WebTab::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (!isRestored() && !s_pinningTab) {
        // When session is being restored, restore the tab immediately
        if (mApp->isRestoring()) {
            slotRestore();
        }
        else {
            QTimer::singleShot(0, this, SLOT(slotRestore()));
        }
    }
}

bool WebTab::isCurrentTab() const
{
    return m_tabBar && tabIndex() == m_tabBar->currentIndex();
}

int WebTab::tabIndex() const
{
    Q_ASSERT(m_tabBar);

    return m_tabBar->tabWidget()->indexOf(const_cast<WebTab*>(this));
}

void WebTab::togglePinned()
{
    Q_ASSERT(m_tabBar);
    Q_ASSERT(m_window);

    m_isPinned = !m_isPinned;

    // Workaround bug in TabStackedWidget when pinning tab, other tabs may be accidentaly
    // shown and restored state even when they won't be switched to by user.
    s_pinningTab = true;
    m_window->tabWidget()->pinUnPinTab(tabIndex(), title());
    s_pinningTab = false;
}
