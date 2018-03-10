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

static const int savedTabVersion = 6;

WebTab::SavedTab::SavedTab()
    : isPinned(false)
    , zoomLevel(qzSettings->defaultZoomLevel)
    , parentTab(-1)
{
}

WebTab::SavedTab::SavedTab(WebTab* webTab)
{
    title = webTab->title();
    url = webTab->url();
    icon = webTab->icon(true);
    history = webTab->historyData();
    isPinned = webTab->isPinned();
    zoomLevel = webTab->zoomLevel();
    parentTab = webTab->parentTab() ? webTab->parentTab()->tabIndex() : -1;

    const auto children = webTab->childTabs();
    childTabs.reserve(children.count());
    for (WebTab *child : children) {
        childTabs.append(child->tabIndex());
    }

    sessionData = webTab->sessionData();
}

bool WebTab::SavedTab::isValid() const
{
    return !url.isEmpty() || !history.isEmpty();
}

void WebTab::SavedTab::clear()
{
    title.clear();
    url.clear();
    icon = QIcon();
    history.clear();
    isPinned = false;
    zoomLevel = qzSettings->defaultZoomLevel;
    parentTab = -1;
    childTabs.clear();
    sessionData.clear();
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
    stream << tab.parentTab;
    stream << tab.childTabs;
    stream << tab.sessionData;

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

    if (version >= 4)
        stream >> tab.parentTab;

    if (version >= 5)
        stream >> tab.childTabs;

    if (version >= 6)
        stream >> tab.sessionData;

    tab.icon = QIcon(pixmap);

    return stream;
}

WebTab::WebTab(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QSL("webtab"));

    m_webView = new TabbedWebView(this);
    m_webView->setPage(new WebPage);
    m_webView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setFocusProxy(m_webView);

    m_locationBar = new LocationBar(this);
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
    connect(m_webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
    connect(m_webView, &TabbedWebView::titleChanged, this, &WebTab::titleWasChanged);
    connect(m_webView, &TabbedWebView::titleChanged, this, &WebTab::titleChanged);
    connect(m_webView, &TabbedWebView::iconChanged, this, &WebTab::iconChanged);
    connect(m_webView, &TabbedWebView::backgroundActivityChanged, this, &WebTab::backgroundActivityChanged);
    connect(m_webView, &TabbedWebView::loadStarted, this, std::bind(&WebTab::loadingChanged, this, true));
    connect(m_webView, &TabbedWebView::loadFinished, this, std::bind(&WebTab::loadingChanged, this, false));

    auto pageChanged = [this](WebPage *page) {
        connect(page, &WebPage::audioMutedChanged, this, &WebTab::playingChanged);
        connect(page, &WebPage::recentlyAudibleChanged, this, &WebTab::mutedChanged);
    };
    pageChanged(m_webView->page());
    connect(m_webView, &TabbedWebView::pageChanged, this, pageChanged);

    // Workaround QTabBar not immediately noticing resizing of tab buttons
    connect(m_tabIcon, &TabIcon::resized, this, [this]() {
        if (m_tabBar) {
            m_tabBar->setTabButton(tabIndex(), m_tabBar->iconButtonPosition(), m_tabIcon);
        }
    });
}

BrowserWindow *WebTab::browserWindow() const
{
    return m_window;
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

void WebTab::showSearchToolBar(const QString &searchText)
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
    if (!searchText.isEmpty()) {
        toolBar->setText(searchText);
    }
    toolBar->focusSearchLine();
}

QUrl WebTab::url() const
{
    if (isRestored()) {
        if (m_webView->url().isEmpty() && m_webView->isLoading()) {
            return m_webView->page()->requestedUrl();
        }
        return m_webView->url();
    }
    else {
        return m_savedTab.url;
    }
}

QString WebTab::title(bool allowEmpty) const
{
    if (isRestored()) {
        return m_webView->title(allowEmpty);
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
    Q_ASSERT(m_window);
    Q_ASSERT(m_tabBar);

    // Remove from tab tree
    removeFromTabTree();

    // Remove icon from tab
    m_tabBar->setTabButton(tabIndex(), m_tabBar->iconButtonPosition(), nullptr);
    m_tabIcon->setParent(this);

    // Remove the tab from tabbar
    m_window->tabWidget()->removeTab(tabIndex());
    setParent(nullptr);
    // Remove the locationbar from window
    m_locationBar->setParent(this);
    // Detach TabbedWebView
    m_webView->setBrowserWindow(nullptr);

    if (m_isCurrentTab) {
        m_isCurrentTab = false;
        emit currentTabChanged(m_isCurrentTab);
    }
    m_tabBar->disconnect(this);

    // WebTab is now standalone widget
    m_window = nullptr;
    m_tabBar = nullptr;
}

void WebTab::attach(BrowserWindow* window)
{
    m_window = window;
    m_tabBar = m_window->tabWidget()->tabBar();

    m_webView->setBrowserWindow(m_window);
    m_locationBar->setBrowserWindow(m_window);
    m_tabBar->setTabText(tabIndex(), title());
    m_tabBar->setTabButton(tabIndex(), m_tabBar->iconButtonPosition(), m_tabIcon);
    m_tabIcon->updateIcon();

    auto currentChanged = [this](int index) {
        const bool wasCurrent = m_isCurrentTab;
        m_isCurrentTab = index == tabIndex();
        if (wasCurrent != m_isCurrentTab) {
            emit currentTabChanged(m_isCurrentTab);
        }
    };

    currentChanged(m_tabBar->currentIndex());
    connect(m_tabBar, &TabBar::currentChanged, this, currentChanged);
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

void WebTab::stop()
{
    m_webView->stop();
}

void WebTab::reload()
{
    m_webView->reload();
}

void WebTab::load(const LoadRequest &request)
{
    if (!isRestored()) {
        tabActivated();
        QTimer::singleShot(0, this, std::bind(&WebTab::load, this, request));
    } else {
        m_webView->load(request);
    }
}

void WebTab::unload()
{
    m_savedTab = SavedTab(this);
    emit restoredChanged(isRestored());
    m_webView->setPage(new WebPage);
    m_webView->setFocus();
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
    if (m_isPinned == state) {
        return;
    }

    if (state) {
        removeFromTabTree();
    }

    m_isPinned = state;
    emit pinnedChanged(m_isPinned);
}

bool WebTab::isMuted() const
{
    return m_webView->page()->isAudioMuted();
}

bool WebTab::isPlaying() const
{
    return m_webView->page()->recentlyAudible();
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

bool WebTab::backgroundActivity() const
{
    return m_webView->backgroundActivity();
}

LocationBar* WebTab::locationBar() const
{
    return m_locationBar;
}

TabIcon* WebTab::tabIcon() const
{
    return m_tabIcon;
}

WebTab *WebTab::parentTab() const
{
    return m_parentTab;
}

void WebTab::setParentTab(WebTab *tab)
{
    if (m_isPinned || m_parentTab == tab) {
        return;
    }

    if (tab && tab->isPinned()) {
        return;
    }

    if (m_parentTab) {
        const int index = m_parentTab->m_childTabs.indexOf(this);
        if (index >= 0) {
            m_parentTab->m_childTabs.removeAt(index);
            emit m_parentTab->childTabRemoved(this, index);
        }
    }

    m_parentTab = tab;

    if (tab) {
        m_parentTab = nullptr;
        tab->addChildTab(this);
    } else {
        emit parentTabChanged(m_parentTab);
    }
}

void WebTab::addChildTab(WebTab *tab, int index)
{
    if (m_isPinned || !tab || tab->isPinned()) {
        return;
    }

    WebTab *oldParent = tab->m_parentTab;
    tab->m_parentTab = this;
    if (oldParent) {
        const int index = oldParent->m_childTabs.indexOf(tab);
        if (index >= 0) {
            oldParent->m_childTabs.removeAt(index);
            emit oldParent->childTabRemoved(tab, index);
        }
    }

    if (index < 0 || index > m_childTabs.size()) {
        index = 0;
        if (addChildBehavior() == AppendChild) {
            index = m_childTabs.size();
        } else if (addChildBehavior() == PrependChild) {
            index = 0;
        }
    }

    m_childTabs.insert(index, tab);
    emit childTabAdded(tab, index);

    emit tab->parentTabChanged(this);
}

QVector<WebTab*> WebTab::childTabs() const
{
    return m_childTabs;
}

QHash<QString, QVariant> WebTab::sessionData() const
{
    return m_sessionData;
}

void WebTab::setSessionData(const QString &key, const QVariant &value)
{
    m_sessionData[key] = value;
}

bool WebTab::isRestored() const
{
    return !m_savedTab.isValid();
}

void WebTab::restoreTab(const WebTab::SavedTab &tab)
{
    Q_ASSERT(m_tabBar);

    setPinned(tab.isPinned);
    m_sessionData = tab.sessionData;

    if (!isPinned() && qzSettings->loadTabsOnActivation) {
        m_savedTab = tab;
        emit restoredChanged(isRestored());
        int index = tabIndex();

        m_tabBar->setTabText(index, tab.title);
        m_locationBar->showUrl(tab.url);
        m_tabIcon->updateIcon();
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
    m_notificationWidget->setParent(this);
    m_notificationWidget->raise();
    m_notificationWidget->setFixedWidth(width());
    m_notificationWidget->layout()->addWidget(notif);
    m_notificationWidget->show();
    notif->show();
}

void WebTab::loadFinished()
{
    titleWasChanged(m_webView->title());
}

void WebTab::titleWasChanged(const QString &title)
{
    if (!m_tabBar || !m_window || title.isEmpty()) {
        return;
    }

    if (m_isCurrentTab) {
        m_window->setWindowTitle(tr("%1 - QupZilla").arg(title));
    }

    m_tabBar->setTabText(tabIndex(), title);
}

void WebTab::tabActivated()
{
    if (isRestored()) {
        return;
    }

    QTimer::singleShot(0, this, [this]() {
        if (isRestored()) {
            return;
        }
        p_restoreTab(m_savedTab);
        m_savedTab.clear();
        emit restoredChanged(isRestored());
    });
}

static WebTab::AddChildBehavior s_addChildBehavior = WebTab::AppendChild;

// static
WebTab::AddChildBehavior WebTab::addChildBehavior()
{
    return s_addChildBehavior;
}

// static
void WebTab::setAddChildBehavior(AddChildBehavior behavior)
{
    s_addChildBehavior = behavior;
}

void WebTab::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    m_notificationWidget->setFixedWidth(width());
}

void WebTab::removeFromTabTree()
{
    WebTab *parentTab = m_parentTab;
    const int parentIndex = parentTab ? parentTab->m_childTabs.indexOf(this) : -1;

    setParentTab(nullptr);

    int i = 0;
    while (!m_childTabs.isEmpty()) {
        WebTab *child = m_childTabs.at(0);
        child->setParentTab(nullptr);
        if (parentTab) {
            parentTab->addChildTab(child, parentIndex + i++);
        }
    }
}

bool WebTab::isCurrentTab() const
{
    return m_isCurrentTab;
}

void WebTab::makeCurrentTab()
{
    if (m_tabBar) {
        m_tabBar->tabWidget()->setCurrentIndex(tabIndex());
    }
}

void WebTab::closeTab()
{
    if (m_tabBar) {
        m_tabBar->tabWidget()->closeTab(tabIndex());
    }
}

void WebTab::moveTab(int to)
{
    if (m_tabBar) {
        m_tabBar->tabWidget()->moveTab(tabIndex(), to);
    }
}

int WebTab::tabIndex() const
{
    return m_tabBar ? m_tabBar->tabWidget()->indexOf(const_cast<WebTab*>(this)) : -1;
}

void WebTab::togglePinned()
{
    Q_ASSERT(m_tabBar);
    Q_ASSERT(m_window);

    setPinned(!isPinned());
    m_window->tabWidget()->pinUnPinTab(tabIndex(), title());
}
