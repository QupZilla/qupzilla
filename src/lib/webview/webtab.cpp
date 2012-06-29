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
#include "webtab.h"
#include "qupzilla.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "tabbar.h"
#include "tabwidget.h"
#include "locationbar.h"
#include "globalfunctions.h"
#include "websettings.h"

#include <QVBoxLayout>
#include <QWebHistory>
#include <QWebFrame>
#include <QLabel>
#include <QStyle>

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
    stream << tab.title;
    stream << tab.url;
    stream << tab.icon;
    stream << tab.history;

    return stream;
}

QDataStream &operator >>(QDataStream &stream, WebTab::SavedTab &tab)
{
    stream >> tab.title;
    stream >> tab.url;
    stream >> tab.icon;
    stream >> tab.history;

    return stream;
}

WebTab::WebTab(QupZilla* mainClass, LocationBar* locationBar)
    : QWidget()
    , p_QupZilla(mainClass)
    , m_locationBar(locationBar)
    , m_pinned(false)
    , m_inspectorVisible(false)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_view = new TabbedWebView(p_QupZilla, this);
    WebPage* page = new WebPage(p_QupZilla);
    m_view->setWebPage(page);
    m_layout->addWidget(m_view);

    setLayout(m_layout);
    setAutoFillBackground(true); // We don't want this transparent

    connect(m_view, SIGNAL(showNotification(QWidget*)), this, SLOT(showNotification(QWidget*)));
    connect(m_view, SIGNAL(iconChanged()), m_locationBar.data(), SLOT(siteIconChanged()));
    connect(m_view, SIGNAL(loadStarted()), m_locationBar.data(), SLOT(clearIcon()));
    connect(m_view, SIGNAL(loadFinished(bool)), m_locationBar.data(), SLOT(siteIconChanged()));
    connect(m_view, SIGNAL(urlChanged(QUrl)), m_locationBar.data(), SLOT(showUrl(QUrl)));
    connect(m_view, SIGNAL(rssChanged(bool)), m_locationBar.data(), SLOT(showRSSIcon(bool)));
    connect(m_view, SIGNAL(privacyChanged(bool)), m_locationBar.data(), SLOT(setPrivacy(bool)));
    connect(m_locationBar.data(), SIGNAL(loadUrl(QUrl)), m_view, SLOT(load(QUrl)));
}

TabbedWebView* WebTab::view() const
{
    return m_view;
}

void WebTab::setCurrentTab()
{
    if (!isRestored()) {
        p_restoreTab(m_savedTab);

        m_savedTab.clear();
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
    if (WebSettings::loadTabsOnActivation) {
        m_savedTab = tab;
        int index = tabIndex();

        m_view->tabWidget()->setTabIcon(index, tab.icon);
        m_view->tabWidget()->setTabText(index, tab.title);
        m_view->tabWidget()->setTabToolTip(index, tab.title);
        m_locationBar.data()->showUrl(tab.url);
    }
    else {
        p_restoreTab(tab);
    }
}

void WebTab::p_restoreTab(const QUrl &url, const QByteArray &history)
{
    QDataStream historyStream(history);
    historyStream >> *m_view->history();

    m_view->load(url);
}

void WebTab::p_restoreTab(const WebTab::SavedTab &tab)
{
    p_restoreTab(tab.url, tab.history);
}

QPixmap WebTab::renderTabPreview()
{
    WebPage* page = m_view->page();
    QSize oldSize = page->viewportSize();

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

    return pageImage.scaled(previewWidth, previewHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void WebTab::showNotification(QWidget* notif)
{
    if (m_layout->count() > 1) {
        delete m_layout->itemAt(0)->widget();
    }

    m_layout->insertWidget(0, notif);
    notif->show();
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

    if (m_pinned) { //Unpin tab
        m_pinned = false;
        tabWidget->setTabText(index, m_view->title());
        tabWidget->getTabBar()->updateCloseButton(index);
    }
    else {   // Pin tab
        m_pinned = true;
        tabWidget->setCurrentIndex(0); //             <<-- those 2 lines fixes
        tabWidget->getTabBar()->moveTab(index, 0);    // | weird behavior with bad
        tabWidget->setTabText(0, "");                 // | tabwidget update if we
        tabWidget->setCurrentIndex(0); //             <<-- are moving current tab
        tabWidget->getTabBar()->updateCloseButton(0);
    }
}

void WebTab::disconnectObjects()
{
    disconnect(this);
    disconnect(m_locationBar.data());
    disconnect(m_view);
}

WebTab::~WebTab()
{
    if (m_locationBar) {
        delete m_locationBar.data();
    }
}
