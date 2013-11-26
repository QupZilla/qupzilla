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
#include "webtab.h"
#include "qupzilla.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "tabbar.h"
#include "tabwidget.h"
#include "locationbar.h"
#include "qztools.h"
#include "qzsettings.h"
#include "mainapplication.h"

#include <QVBoxLayout>
#include <QWebHistory>
#include <QWebFrame>
#include <QLabel>
#include <QStyle>
#include <QTimer>

static const int savedTabVersion = 1;

WebTab::SavedTab::SavedTab(WebTab* webTab)
{
    title = webTab->title();
    url = webTab->url();
    icon = webTab->icon();
    history = webTab->historyData();
}

void WebTab::SavedTab::clear()
{
    title.clear();
    url.clear();
    icon = QIcon();
    history.clear();
}

QDataStream &operator <<(QDataStream &stream, const WebTab::SavedTab &tab)
{
    stream << savedTabVersion;
    stream << tab.title;
    stream << tab.url;
    stream << tab.icon.pixmap(16);
    stream << tab.history;

    return stream;
}

QDataStream &operator >>(QDataStream &stream, WebTab::SavedTab &tab)
{
    int version;
    stream >> version;

    // FIXME: HACK to ensure backwards compatibility
    if (version != savedTabVersion) {
        stream.device()->seek(stream.device()->pos() - sizeof(int));
        stream >> tab.title;
        stream >> tab.url;
        stream >> tab.icon;
        stream >> tab.history;
        return stream;
    }

    QPixmap pixmap;
    stream >> tab.title;
    stream >> tab.url;
    stream >> pixmap;
    stream >> tab.history;

    tab.icon = QIcon(pixmap);

    return stream;
}

WebTab::WebTab(QupZilla* mainClass, LocationBar* locationBar)
    : QWidget()
    , p_QupZilla(mainClass)
    , m_navigationContainer(0)
    , m_locationBar(locationBar)
    , m_pinned(false)
    , m_inspectorVisible(false)
{
    setObjectName("webtab");

    // This fixes background of pages with dark themes
    setStyleSheet("#webtab {background-color:white;}");

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_view = new TabbedWebView(p_QupZilla, this);
    m_view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    WebPage* page = new WebPage;
    m_view->setWebPage(page);
    m_layout->addWidget(m_view);

    setLayout(m_layout);

    connect(m_view, SIGNAL(showNotification(QWidget*)), this, SLOT(showNotification(QWidget*)));
    connect(m_view, SIGNAL(iconChanged()), m_locationBar.data(), SLOT(siteIconChanged()));
    connect(m_view, SIGNAL(loadStarted()), m_locationBar.data(), SLOT(clearIcon()));
    connect(m_view, SIGNAL(loadFinished(bool)), m_locationBar.data(), SLOT(siteIconChanged()));
    connect(m_view, SIGNAL(urlChanged(QUrl)), m_locationBar.data(), SLOT(showUrl(QUrl)));
    connect(m_view, SIGNAL(rssChanged(bool)), m_locationBar.data(), SLOT(showRSSIcon(bool)));
    connect(m_view, SIGNAL(privacyChanged(bool)), m_locationBar.data(), SLOT(setPrivacy(bool)));
    connect(m_locationBar.data(), SIGNAL(loadUrl(QUrl)), m_view, SLOT(userLoadAction(QUrl)));
}

TabbedWebView* WebTab::view() const
{
    return m_view;
}

void WebTab::setCurrentTab()
{
    if (!isRestored()) {
        // When session is being restored, restore the tab immediately
        if (mApp->isRestoring()) {
            slotRestore();
        }
        else {
            QTimer::singleShot(0, this, SLOT(slotRestore()));
        }
    }
}

QUrl WebTab::url() const
{
    if (isRestored()) {
        return m_view->url();
    }
    else {
        return m_savedTab.url;
    }
}

QString WebTab::title() const
{
    if (isRestored()) {
        return m_view->title();
    }
    else {
        return m_savedTab.title;
    }
}

QIcon WebTab::icon() const
{
    if (isRestored()) {
        return m_view->icon();
    }
    else {
        return m_savedTab.icon;
    }
}

QWebHistory* WebTab::history() const
{
    return m_view->history();
}

void WebTab::moveToWindow(QupZilla* window)
{
    p_QupZilla = window;

    m_view->moveToWindow(p_QupZilla);
}

void WebTab::setHistoryData(const QByteArray &data)
{
    QDataStream historyStream(data);
    historyStream >> *m_view->history();
}

QByteArray WebTab::historyData() const
{
    if (isRestored()) {
        QByteArray historyArray;
        QDataStream historyStream(&historyArray, QIODevice::WriteOnly);
        historyStream << *m_view->history();

        return historyArray;
    }
    else {
        return m_savedTab.history;
    }
}

void WebTab::reload()
{
    m_view->reload();
}

void WebTab::stop()
{
    m_view->stop();
}

bool WebTab::isLoading() const
{
    return m_view->isLoading();
}

bool WebTab::isPinned() const
{
    return m_pinned;
}

void WebTab::setPinned(bool state)
{
    m_pinned = state;
}

void WebTab::setLocationBar(LocationBar* bar)
{
    m_locationBar = bar;
}

LocationBar* WebTab::locationBar() const
{
    return m_locationBar.data();
}

bool WebTab::inspectorVisible() const
{
    return m_inspectorVisible;
}

void WebTab::setInspectorVisible(bool v)
{
    m_inspectorVisible = v;
}

WebTab::SavedTab WebTab::savedTab() const
{
    return m_savedTab;
}

bool WebTab::isRestored() const
{
    return m_savedTab.isEmpty();
}

void WebTab::restoreTab(const WebTab::SavedTab &tab)
{
    if (qzSettings->loadTabsOnActivation) {
        m_savedTab = tab;
        int index = tabIndex();

        m_view->tabWidget()->setTabIcon(index, tab.icon);
        m_view->tabWidget()->setTabText(index, tab.title);
        m_view->tabWidget()->setTabToolTip(index, tab.title);
        m_locationBar.data()->showUrl(tab.url);

        if (!tab.url.isEmpty()) {
            QColor col = m_view->tabWidget()->getTabBar()->palette().text().color();
            QColor newCol = col.lighter(250);

            // It won't work for black color because (V) = 0
            // It won't also work for white, as white won't get any lighter
            if (col == Qt::black || col == Qt::white) {
                newCol = Qt::gray;
            }

            m_view->tabWidget()->getTabBar()->overrideTabTextColor(index, newCol);
        }
    }
    else {
        p_restoreTab(tab);
    }
}

void WebTab::p_restoreTab(const QUrl &url, const QByteArray &history)
{
    m_view->load(url);

    QDataStream historyStream(history);
    historyStream >> *m_view->history();
}

void WebTab::p_restoreTab(const WebTab::SavedTab &tab)
{
    p_restoreTab(tab.url, tab.history);
}

QPixmap WebTab::renderTabPreview()
{
    TabbedWebView* currentWebView = p_QupZilla->weView();
    WebPage* page = m_view->page();
    const QSize oldSize = page->viewportSize();
    const QPoint originalScrollPosition = page->mainFrame()->scrollPosition();

    // Hack to ensure rendering the same preview before and after the page was shown for the first time
    // This can occur eg. with opening background tabs
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
    m_view->page()->mainFrame()->render(&p, QWebFrame::ContentsLayer);
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

void WebTab::slotRestore()
{
    p_restoreTab(m_savedTab);
    m_savedTab.clear();

    m_view->tabWidget()->getTabBar()->restoreTabTextColor(tabIndex());
}

int WebTab::tabIndex() const
{
    return m_view->tabIndex();
}

void WebTab::pinTab(int index)
{
    TabWidget* tabWidget = p_QupZilla->tabWidget();
    if (!tabWidget) {
        return;
    }

    m_pinned = !m_pinned;
    index = tabWidget->pinUnPinTab(index, m_view->title());
    tabWidget->setCurrentIndex(index);
}

void WebTab::disconnectObjects()
{
    disconnect(this);
    disconnect(m_locationBar.data());
    disconnect(m_view);
}

WebTab::~WebTab()
{
    delete m_locationBar.data();
}
