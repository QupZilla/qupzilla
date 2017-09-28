/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#include "iconprovider.h"
#include "searchtoolbar.h"

#include <QVBoxLayout>
#include <QWebEngineHistory>
#include <QLabel>
#include <QTimer>
#include <QSplitter>

static const int savedTabVersion = 3;

WebTab::SavedTab::SavedTab()
    : isPinned(false)
    , zoomLevel(qzSettings->defaultZoomLevel)
{
}

WebTab::SavedTab::SavedTab(WebTab* webTab)
{
    if (webTab->url().toString() == QL1S("qupzilla:restore")) {
        return;
    }

    title = webTab->title();
    url = webTab->url();
    icon = webTab->icon(true);
    history = webTab->historyData();
    isPinned = webTab->isPinned();
    zoomLevel = webTab->zoomLevel();
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
    zoomLevel = qzSettings->defaultZoomLevel;
}

QDataStream &operator <<(QDataStream &stream, const WebTab::SavedTab &tab)
{
    stream << savedTabVersion;
    stream << tab.title;
    stream << tab.url;
    stream << tab.icon.pixmap(16);
    stream << tab.history;
    stream << tab.isPinned;
    stream << tab.zoomLevel;

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

    if (version >= 3)
        stream >> tab.zoomLevel;

    tab.icon = QIcon(pixmap);

    return stream;
}

WebTab::WebTab(BrowserWindow* window)
    : QWidget()
    , m_window(window)
    , m_tabBar(0)
    , m_isPinned(false)
{
    setObjectName(QSL("webtab"));

    m_webView = new TabbedWebView(this);
    m_webView->setBrowserWindow(m_window);
    m_webView->setWebPage(new WebPage);
    m_webView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    m_locationBar = new LocationBar(m_window);
    m_locationBar->setWebView(m_webView);

    m_tabIcon = new TabIcon(this);
    m_tabIcon->setWebTab(this);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_webView);

    QWidget *viewWidget = new QWidget(this);
    viewWidget->setLayout(m_layout);

    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->addWidget(viewWidget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_splitter);
    setLayout(layout);

    m_notificationWidget = new QWidget(this);
    m_notificationWidget->setAutoFillBackground(true);
    QPalette pal = m_notificationWidget->palette();
    pal.setColor(QPalette::Background, pal.window().color().darker(110));
    m_notificationWidget->setPalette(pal);

    QVBoxLayout *nlayout = new QVBoxLayout(m_notificationWidget);
    nlayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    nlayout->setContentsMargins(0, 0, 0, 0);
    nlayout->setSpacing(1);

    connect(m_webView, SIGNAL(showNotification(QWidget*)), this, SLOT(showNotification(QWidget*)));
    connect(m_webView, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(m_webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
    connect(m_webView, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged(QString)));

    // Workaround QTabBar not immediately noticing resizing of tab buttons
    connect(m_tabIcon, &TabIcon::resized, this, [this]() {
        if (m_tabBar) {
            m_tabBar->setTabButton(tabIndex(), m_tabBar->iconButtonPosition(), m_tabIcon);
        }
    });
}

TabbedWebView* WebTab::webView() const
{
    return m_webView;
}

bool WebTab::haveInspector() const
{
    return m_splitter->count() > 1 && m_splitter->widget(1)->inherits("WebInspector");
}

void WebTab::showWebInspector(bool inspectElement)
{
    if (!WebInspector::isEnabled() || haveInspector())
        return;

    WebInspector *inspector = new WebInspector(this);
    inspector->setView(m_webView);
    if (inspectElement)
        inspector->inspectElement();

    m_splitter->addWidget(inspector);
}

void WebTab::toggleWebInspector()
{
    if (!haveInspector())
        showWebInspector();
    else
        delete m_splitter->widget(1);
}

void WebTab::showSearchToolBar()
{
    const int index = 1;

    SearchToolBar *toolBar = nullptr;

    if (m_layout->count() == 1) {
        toolBar = new SearchToolBar(m_webView, this);
        m_layout->insertWidget(index, toolBar);
    } else if (m_layout->count() == 2) {
        Q_ASSERT(qobject_cast<SearchToolBar*>(m_layout->itemAt(index)->widget()));
        toolBar = static_cast<SearchToolBar*>(m_layout->itemAt(index)->widget());
    }

    Q_ASSERT(toolBar);
    toolBar->focusSearchLine();
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

QIcon WebTab::icon(bool allowNull) const
{
    if (isRestored()) {
        return m_webView->icon(allowNull);
    }

    if (allowNull || !m_savedTab.icon.isNull()) {
        return m_savedTab.icon;
    }

    return IconProvider::emptyWebIcon();
}

QWebEngineHistory* WebTab::history() const
{
    return m_webView->history();
}

int WebTab::zoomLevel() const
{
    return m_webView->zoomLevel();
}

void WebTab::setZoomLevel(int level)
{
    m_webView->setZoomLevel(level);
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
    m_tabBar->setTabText(tabIndex(), title());
    m_tabBar->setTabButton(tabIndex(), m_tabBar->iconButtonPosition(), m_tabIcon);
    m_tabIcon->updateIcon();
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

bool WebTab::isMuted() const
{
    return m_webView->page()->isAudioMuted();
}

void WebTab::setMuted(bool muted)
{
    m_webView->page()->setAudioMuted(muted);
}

void WebTab::toggleMuted()
{
    bool muted = isMuted();
    setMuted(!muted);
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
        m_tabIcon->updateIcon();

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
        // This is called only on restore session and restoring tabs immediately
        // crashes QtWebEngine, waiting after initialization is complete fixes it
        QTimer::singleShot(1000, this, [=]() {
            p_restoreTab(tab);
        });
    }
}

void WebTab::p_restoreTab(const QUrl &url, const QByteArray &history, int zoomLevel)
{
    m_webView->load(url);

    // Restoring history of internal pages crashes QtWebEngine 5.8
    static const QStringList blacklistedSchemes = {
        QSL("view-source"),
        QSL("chrome")
    };

    if (!blacklistedSchemes.contains(url.scheme())) {
        QDataStream stream(history);
        stream >> *m_webView->history();
    }

    m_webView->setZoomLevel(zoomLevel);
    m_webView->setFocus();
}

void WebTab::p_restoreTab(const WebTab::SavedTab &tab)
{
    p_restoreTab(tab.url, tab.history, tab.zoomLevel);
}

void WebTab::showNotification(QWidget* notif)
{
    m_notificationWidget->setParent(nullptr);
    m_notificationWidget->setParent(this);
    m_notificationWidget->setFixedWidth(width());
    m_notificationWidget->layout()->addWidget(notif);
    m_notificationWidget->show();
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

void WebTab::tabActivated()
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

void WebTab::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    m_notificationWidget->setFixedWidth(width());
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

    m_window->tabWidget()->pinUnPinTab(tabIndex(), title());
}
