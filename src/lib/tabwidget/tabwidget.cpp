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
#include "tabwidget.h"
#include "tabbar.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "browserwindow.h"
#include "mainapplication.h"
#include "webtab.h"
#include "clickablelabel.h"
#include "closedtabsmanager.h"
#include "locationbar.h"
#include "settings.h"
#include "datapaths.h"
#include "qzsettings.h"
#include "qztools.h"
#include "tabicon.h"
#include "pluginproxy.h"

#include <QFile>
#include <QTimer>
#include <QMimeData>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QWebEngineHistory>
#include <QClipboard>

AddTabButton::AddTabButton(TabWidget* tabWidget, TabBar* tabBar)
    : ToolButton(tabBar)
    , m_tabBar(tabBar)
    , m_tabWidget(tabWidget)
{
    setObjectName("tabwidget-button-addtab");
    setAutoRaise(true);
    setFocusPolicy(Qt::NoFocus);
    setAcceptDrops(true);
    setToolTip(TabWidget::tr("New Tab"));
}

void AddTabButton::wheelEvent(QWheelEvent* event)
{
    m_tabBar->wheelEvent(event);
}

void AddTabButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton && rect().contains(event->pos())) {
        m_tabWidget->addTabFromClipboard();
    }

    ToolButton::mouseReleaseEvent(event);
}

void MenuTabs::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        QAction* action = actionAt(event->pos());
        if (action && action->isEnabled()) {
            WebTab* tab = qobject_cast<WebTab*>(qvariant_cast<QWidget*>(action->data()));
            if (tab) {
                emit closeTab(tab->tabIndex());
                action->setEnabled(false);
                event->accept();
            }
        }
    }
    QMenu::mouseReleaseEvent(event);
}

TabWidget::TabWidget(BrowserWindow *window, QWidget *parent)
    : TabStackedWidget(parent)
    , m_window(window)
    , m_locationBars(new QStackedWidget)
    , m_closedTabsManager(new ClosedTabsManager)
{
    setObjectName(QSL("tabwidget"));

    m_tabBar = new TabBar(m_window, this);
    setTabBar(m_tabBar);

    connect(this, SIGNAL(currentChanged(int)), m_window, SLOT(refreshHistory()));
    connect(this, &TabWidget::changed, mApp, &MainApplication::changeOccurred);
    connect(this, &TabStackedWidget::pinStateChanged, this, &TabWidget::changed);

    connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(requestCloseTab(int)));
    connect(m_tabBar, &TabBar::tabMoved, this, &TabWidget::tabWasMoved);

    connect(m_tabBar, SIGNAL(moveAddTabButton(int)), this, SLOT(moveAddTabButton(int)));

    connect(mApp, SIGNAL(settingsReloaded()), this, SLOT(loadSettings()));

    m_menuTabs = new MenuTabs(this);
    connect(m_menuTabs, SIGNAL(closeTab(int)), this, SLOT(requestCloseTab(int)));

    m_menuClosedTabs = new QMenu(this);

    // AddTab button displayed next to last tab
    m_buttonAddTab = new AddTabButton(this, m_tabBar);
    m_buttonAddTab->setProperty("outside-tabbar", false);
    connect(m_buttonAddTab, SIGNAL(clicked()), m_window, SLOT(addTab()));

    // AddTab button displayed outside tabbar (as corner widget)
    m_buttonAddTab2 = new AddTabButton(this, m_tabBar);
    m_buttonAddTab2->setProperty("outside-tabbar", true);
    m_buttonAddTab2->hide();
    connect(m_buttonAddTab2, SIGNAL(clicked()), m_window, SLOT(addTab()));

    // ClosedTabs button displayed as a permanent corner widget
    m_buttonClosedTabs = new ToolButton(m_tabBar);
    m_buttonClosedTabs->setObjectName("tabwidget-button-closedtabs");
    m_buttonClosedTabs->setMenu(m_menuClosedTabs);
    m_buttonClosedTabs->setPopupMode(QToolButton::InstantPopup);
    m_buttonClosedTabs->setToolTip(tr("Closed tabs"));
    m_buttonClosedTabs->setAutoRaise(true);
    m_buttonClosedTabs->setFocusPolicy(Qt::NoFocus);
    m_buttonClosedTabs->setShowMenuInside(true);
    connect(m_buttonClosedTabs, SIGNAL(aboutToShowMenu()), this, SLOT(aboutToShowClosedTabsMenu()));

    // ListTabs button is showed only when tabbar overflows
    m_buttonListTabs = new ToolButton(m_tabBar);
    m_buttonListTabs->setObjectName("tabwidget-button-opentabs");
    m_buttonListTabs->setMenu(m_menuTabs);
    m_buttonListTabs->setPopupMode(QToolButton::InstantPopup);
    m_buttonListTabs->setToolTip(tr("List of tabs"));
    m_buttonListTabs->setAutoRaise(true);
    m_buttonListTabs->setFocusPolicy(Qt::NoFocus);
    m_buttonListTabs->setShowMenuInside(true);
    m_buttonListTabs->hide();
    connect(m_buttonListTabs, SIGNAL(aboutToShowMenu()), this, SLOT(aboutToShowTabsMenu()));

    m_tabBar->addCornerWidget(m_buttonAddTab2, Qt::TopRightCorner);
    m_tabBar->addCornerWidget(m_buttonClosedTabs, Qt::TopRightCorner);
    m_tabBar->addCornerWidget(m_buttonListTabs, Qt::TopRightCorner);
    connect(m_tabBar, SIGNAL(overFlowChanged(bool)), this, SLOT(tabBarOverFlowChanged(bool)));

    loadSettings();
}

BrowserWindow *TabWidget::browserWindow() const
{
    return m_window;
}

void TabWidget::loadSettings()
{
    Settings settings;
    settings.beginGroup("Browser-Tabs-Settings");
    m_dontCloseWithOneTab = settings.value("dontCloseWithOneTab", false).toBool();
    m_showClosedTabsButton = settings.value("showClosedTabsButton", false).toBool();
    m_newTabAfterActive = settings.value("newTabAfterActive", true).toBool();
    m_newEmptyTabAfterActive = settings.value("newEmptyTabAfterActive", false).toBool();
    settings.endGroup();

    settings.beginGroup("Web-URL-Settings");
    m_urlOnNewTab = settings.value("newTabUrl", "qupzilla:speeddial").toUrl();
    settings.endGroup();

    m_tabBar->loadSettings();

    updateClosedTabsButton();
}

WebTab* TabWidget::weTab() const
{
    return weTab(currentIndex());
}

WebTab* TabWidget::weTab(int index) const
{
    return qobject_cast<WebTab*>(widget(index));
}

TabIcon* TabWidget::tabIcon(int index) const
{
    return weTab(index)->tabIcon();
}

bool TabWidget::validIndex(int index) const
{
    return index >= 0 && index < count();
}

void TabWidget::updateClosedTabsButton()
{
    m_buttonClosedTabs->setVisible(m_showClosedTabsButton && canRestoreTab());
}

void TabWidget::keyPressEvent(QKeyEvent *event)
{
    if (mApp->plugins()->processKeyPress(Qz::ON_TabWidget, this, event)) {
        return;
    }

    TabStackedWidget::keyPressEvent(event);
}

void TabWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (mApp->plugins()->processKeyRelease(Qz::ON_TabWidget, this, event)) {
        return;
    }

    TabStackedWidget::keyReleaseEvent(event);
}

bool TabWidget::isCurrentTabFresh() const
{
    return m_currentTabFresh;
}

void TabWidget::setCurrentTabFresh(bool currentTabFresh)
{
    m_currentTabFresh = currentTabFresh;
}

void TabWidget::tabBarOverFlowChanged(bool overflowed)
{
    // Show buttons inside tabbar
    m_buttonAddTab->setVisible(!overflowed);

    // Show buttons displayed outside tabbar (corner widgets)
    m_buttonAddTab2->setVisible(overflowed);
    m_buttonListTabs->setVisible(overflowed);
}

void TabWidget::moveAddTabButton(int posX)
{
    int posY = (m_tabBar->height() - m_buttonAddTab->height()) / 2;

    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        posX = qMax(posX - m_buttonAddTab->width(), 0);
    }
    else {
        posX = qMin(posX, m_tabBar->width() - m_buttonAddTab->width());
    }

    m_buttonAddTab->move(posX, posY);
}

void TabWidget::aboutToShowTabsMenu()
{
    m_menuTabs->clear();

    for (int i = 0; i < count(); i++) {
        WebTab* tab = weTab(i);
        if (!tab || tab->isPinned()) {
            continue;
        }

        QAction* action = new QAction(this);
        action->setIcon(tab->icon());

        if (i == currentIndex()) {
            QFont f = action->font();
            f.setBold(true);
            action->setFont(f);
        }

        QString title = tab->title();
        title.replace(QLatin1Char('&'), QLatin1String("&&"));
        action->setText(QzTools::truncatedText(title, 40));

        action->setData(QVariant::fromValue(qobject_cast<QWidget*>(tab)));
        connect(action, SIGNAL(triggered()), this, SLOT(actionChangeIndex()));
        m_menuTabs->addAction(action);
    }
}

void TabWidget::aboutToShowClosedTabsMenu()
{
    m_menuClosedTabs->clear();

    const auto closedTabs = closedTabsManager()->closedTabs();
    for (int i = 0; i < closedTabs.count(); ++i) {
        const ClosedTabsManager::Tab tab = closedTabs.at(i);
        const QString title = QzTools::truncatedText(tab.tabState.title, 40);
        m_menuClosedTabs->addAction(tab.tabState.icon, title, this, SLOT(restoreClosedTab()))->setData(i);
    }

    if (m_menuClosedTabs->isEmpty()) {
        m_menuClosedTabs->addAction(tr("Empty"))->setEnabled(false);
    }
    else {
        m_menuClosedTabs->addSeparator();
        m_menuClosedTabs->addAction(tr("Restore All Closed Tabs"), this, SLOT(restoreAllClosedTabs()));
        m_menuClosedTabs->addAction(tr("Clear list"), this, SLOT(clearClosedTabsList()));
    }
}

void TabWidget::actionChangeIndex()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        WebTab* tab = qobject_cast<WebTab*>(qvariant_cast<QWidget*>(action->data()));
        if (tab) {
            m_tabBar->ensureVisible(tab->tabIndex());
            setCurrentIndex(tab->tabIndex());
        }
    }
}

int TabWidget::addView(const LoadRequest &req, const Qz::NewTabPositionFlags &openFlags, bool selectLine, bool pinned)
{
    return addView(req, QString(), openFlags, selectLine, -1, pinned);
}

int TabWidget::addView(const LoadRequest &req, const QString &title, const Qz::NewTabPositionFlags &openFlags, bool selectLine, int position, bool pinned)
{
    QUrl url = req.url();
    m_currentTabFresh = false;

    if (url.isEmpty() && !(openFlags & Qz::NT_CleanTab)) {
        url = m_urlOnNewTab;
    }

    bool openAfterActive = m_newTabAfterActive && !(openFlags & Qz::NT_TabAtTheEnd);

    if (openFlags == Qz::NT_SelectedNewEmptyTab && m_newEmptyTabAfterActive) {
        openAfterActive = true;
    }

    if (openAfterActive && position == -1) {
        // If we are opening newBgTab from pinned tab, make sure it won't be
        // opened between other pinned tabs
        if (openFlags & Qz::NT_NotSelectedTab && m_lastBackgroundTab) {
            position = m_lastBackgroundTab->tabIndex() + 1;
        }
        else {
            position = qMax(currentIndex() + 1, m_tabBar->pinnedTabsCount());
        }
    }

    WebTab* webTab = new WebTab(m_window);
    webTab->setPinned(pinned);
    webTab->locationBar()->showUrl(url);
    m_locationBars->addWidget(webTab->locationBar());

    int index = insertTab(position == -1 ? count() : position, webTab, QString(), pinned);
    webTab->attach(m_window);

    if (!title.isEmpty()) {
        m_tabBar->setTabText(index, title);
    }

    if (openFlags & Qz::NT_SelectedTab) {
        setCurrentIndex(index);
    } else {
        m_lastBackgroundTab = webTab;
    }

    connect(webTab->webView(), SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    connect(webTab->webView(), SIGNAL(urlChanged(QUrl)), this, SIGNAL(changed()));
    connect(webTab->webView(), SIGNAL(ipChanged(QString)), m_window->ipLabel(), SLOT(setText(QString)));
    connect(webTab->webView(), &WebView::urlChanged, this, [this](const QUrl &url) {
        if (url != m_urlOnNewTab)
            m_currentTabFresh = false;
    });

    if (url.isValid() && url != req.url()) {
        LoadRequest r(req);
        r.setUrl(url);
        webTab->webView()->load(r);
    }
    else if (req.url().isValid()) {
        webTab->webView()->load(req);
    }

    if (selectLine && m_window->locationBar()->text().isEmpty()) {
        m_window->locationBar()->setFocus();
    }

    // Make sure user notice opening new background tabs
    if (!(openFlags & Qz::NT_SelectedTab)) {
        m_tabBar->ensureVisible(index);
    }

    emit changed();
    emit tabInserted(index);

    return index;
}

int TabWidget::addView(WebTab *tab, const Qz::NewTabPositionFlags &openFlags)
{
    return insertView(count() + 1, tab, openFlags);
}

int TabWidget::insertView(int index, WebTab *tab, const Qz::NewTabPositionFlags &openFlags)
{
    m_locationBars->addWidget(tab->locationBar());
    int newIndex = insertTab(index, tab, QString(), tab->isPinned());
    tab->attach(m_window);

    if (openFlags.testFlag(Qz::NT_SelectedTab)) {
        setCurrentIndex(newIndex);
    } else {
        m_lastBackgroundTab = tab;
    }

    connect(tab->webView(), SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    connect(tab->webView(), SIGNAL(urlChanged(QUrl)), this, SIGNAL(changed()));
    connect(tab->webView(), SIGNAL(ipChanged(QString)), m_window->ipLabel(), SLOT(setText(QString)));

    // Make sure user notice opening new background tabs
    if (!(openFlags & Qz::NT_SelectedTab)) {
        m_tabBar->ensureVisible(index);
    }

    emit changed();
    emit tabInserted(newIndex);

    return newIndex;
}

void TabWidget::addTabFromClipboard()
{
    QString selectionClipboard = QApplication::clipboard()->text(QClipboard::Selection);
    QUrl guessedUrl = QUrl::fromUserInput(selectionClipboard);

    if (!guessedUrl.isEmpty()) {
        addView(guessedUrl, Qz::NT_SelectedNewEmptyTab);
    }
}

void TabWidget::closeTab(int index)
{
    if (index == -1)
        index = currentIndex();

    WebTab *webTab = weTab(index);
    if (!webTab || !validIndex(index))
        return;

    // This is already handled in requestCloseTab
    if (count() <= 1) {
        requestCloseTab(index);
        return;
    }

    m_closedTabsManager->saveTab(webTab);

    TabbedWebView *webView = webTab->webView();
    m_locationBars->removeWidget(webView->webTab()->locationBar());
    disconnect(webView, SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    disconnect(webView, SIGNAL(urlChanged(QUrl)), this, SIGNAL(changed()));
    disconnect(webView, SIGNAL(ipChanged(QString)), m_window->ipLabel(), SLOT(setText(QString)));

    m_lastBackgroundTab = nullptr;

    webTab->detach();
    webTab->deleteLater();

    updateClosedTabsButton();

    emit changed();
    emit tabRemoved(index);
}

void TabWidget::requestCloseTab(int index)
{
    if (index == -1)
        index = currentIndex();

    WebTab *webTab = weTab(index);
    if (!webTab || !validIndex(index))
        return;

    TabbedWebView *webView = webTab->webView();

    // This would close last tab, so we close the window instead
    if (count() <= 1) {
        // If we are not closing window upon closing last tab, let's just load new-tab-url
        if (m_dontCloseWithOneTab) {
            // We don't want to accumulate more than one closed tab, if user tries
            // to close the last tab multiple times
            if (webView->url() != m_urlOnNewTab) {
                m_closedTabsManager->saveTab(webTab);
            }
            webView->zoomReset();
            webView->load(m_urlOnNewTab);
            return;
        }
        m_window->close();
        return;
    }

    webView->triggerPageAction(QWebEnginePage::RequestClose);
}

void TabWidget::currentTabChanged(int index)
{
    if (!validIndex(index))
        return;

    m_lastBackgroundTab = nullptr;
    m_currentTabFresh = false;

    WebTab* webTab = weTab(index);
    webTab->tabActivated();

    LocationBar* locBar = webTab->locationBar();

    if (locBar && m_locationBars->indexOf(locBar) != -1) {
        m_locationBars->setCurrentWidget(locBar);
    }

    m_window->currentTabChanged();

    emit changed();
}

void TabWidget::tabWasMoved(int before, int after)
{
    m_lastBackgroundTab = nullptr;

    emit changed();
    emit tabMoved(before, after);
}

void TabWidget::setCurrentIndex(int index)
{
    TabStackedWidget::setCurrentIndex(index);
}

void TabWidget::nextTab()
{
    setCurrentIndex((currentIndex() + 1) % count());
}

void TabWidget::previousTab()
{
    setCurrentIndex(currentIndex() == 0 ? count() - 1 : currentIndex() - 1);
}

int TabWidget::normalTabsCount() const
{
    return m_tabBar->normalTabsCount();
}

int TabWidget::pinnedTabsCount() const
{
    return m_tabBar->pinnedTabsCount();
}

void TabWidget::reloadTab(int index)
{
    if (!validIndex(index)) {
        return;
    }

    weTab(index)->reload();
}

WebTab *TabWidget::webTab(int index) const
{
    return index < 0 ? weTab() : weTab(index);
}

int TabWidget::extraReservedWidth() const
{
    return m_buttonAddTab->width();
}

TabBar* TabWidget::tabBar() const
{
    return m_tabBar;
}

ClosedTabsManager* TabWidget::closedTabsManager() const
{
    return m_closedTabsManager;
}

void TabWidget::reloadAllTabs()
{
    for (int i = 0; i < count(); i++) {
        reloadTab(i);
    }
}

void TabWidget::stopTab(int index)
{
    if (!validIndex(index)) {
        return;
    }

    weTab(index)->stop();
}

void TabWidget::closeAllButCurrent(int index)
{
    if (!validIndex(index)) {
        return;
    }

    WebTab* akt = weTab(index);

    foreach (WebTab* tab, allTabs(false)) {
        int tabIndex = tab->tabIndex();
        if (akt == widget(tabIndex)) {
            continue;
        }
        requestCloseTab(tabIndex);
    }
}

void TabWidget::closeToRight(int index)
{
    if (!validIndex(index)) {
        return;
    }

    foreach (WebTab* tab, allTabs(false)) {
        int tabIndex = tab->tabIndex();
        if (index >= tabIndex) {
            continue;
        }
        requestCloseTab(tabIndex);
    }
}


void TabWidget::closeToLeft(int index)
{
    if (!validIndex(index)) {
        return;
    }

    foreach (WebTab* tab, allTabs(false)) {
        int tabIndex = tab->tabIndex();
        if (index <= tabIndex) {
            continue;
        }
        requestCloseTab(tabIndex);
    }
}

int TabWidget::pinUnPinTab(int index, const QString &title)
{
    const int newIndex = TabStackedWidget::pinUnPinTab(index, title);
    if (index != newIndex) {
        emit tabMoved(index, newIndex);
    }
    return newIndex;
}

void TabWidget::detachTab(WebTab* tab)
{
    Q_ASSERT(tab);

    if (count() == 1 && mApp->windowCount() == 1) {
        return;
    }

    m_locationBars->removeWidget(tab->locationBar());
    disconnect(tab->webView(), SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    disconnect(tab->webView(), SIGNAL(urlChanged(QUrl)), this, SIGNAL(changed()));
    disconnect(tab->webView(), SIGNAL(ipChanged(QString)), m_window->ipLabel(), SLOT(setText(QString)));

    const int index = tab->tabIndex();

    tab->detach();
    tab->setPinned(false);

    emit tabRemoved(index);

    if (count() == 0) {
        m_window->close();
    }
}

void TabWidget::detachTab(int index)
{
    WebTab* tab = weTab(index);
    Q_ASSERT(tab);

    if (count() == 1 && mApp->windowCount() == 1) {
        return;
    }

    detachTab(tab);

    BrowserWindow* window = mApp->createWindow(Qz::BW_NewWindow);
    window->setStartTab(tab);
}

int TabWidget::duplicateTab(int index)
{
    if (!validIndex(index)) {
        return -1;
    }

    WebTab* webTab = weTab(index);

    int id = addView(QUrl(), webTab->title(), Qz::NT_CleanSelectedTab);
    weTab(id)->p_restoreTab(webTab->url(), webTab->historyData(), webTab->zoomLevel());

    return id;
}

void TabWidget::loadTab(int index)
{
    if (!validIndex(index)) {
        return;
    }

    weTab(index)->tabActivated();
}

void TabWidget::unloadTab(int index)
{
    if (!validIndex(index)) {
        return;
    }

    weTab(index)->unload();
}

void TabWidget::restoreClosedTab(QObject* obj)
{
    if (!obj) {
        obj = sender();
    }

    if (!m_closedTabsManager->isClosedTabAvailable()) {
        return;
    }

    ClosedTabsManager::Tab tab;

    QAction* action = qobject_cast<QAction*>(obj);
    if (action && action->data().toInt() != 0) {
        tab = m_closedTabsManager->takeTabAt(action->data().toInt());
    }
    else {
        tab = m_closedTabsManager->takeLastClosedTab();
    }

    if (tab.position < 0) {
        return;
    }

    int index = addView(QUrl(), tab.tabState.title, Qz::NT_CleanSelectedTab, false, tab.position);
    WebTab* webTab = weTab(index);
    webTab->p_restoreTab(tab.tabState);

    updateClosedTabsButton();
}

void TabWidget::restoreAllClosedTabs()
{
    if (!m_closedTabsManager->isClosedTabAvailable()) {
        return;
    }

    const auto closedTabs = m_closedTabsManager->closedTabs();
    for (const ClosedTabsManager::Tab &tab : closedTabs) {
        int index = addView(QUrl(), tab.tabState.title, Qz::NT_CleanSelectedTab);
        WebTab* webTab = weTab(index);
        webTab->p_restoreTab(tab.tabState);
    }

    clearClosedTabsList();
}

void TabWidget::clearClosedTabsList()
{
    m_closedTabsManager->clearClosedTabs();
    updateClosedTabsButton();
}

bool TabWidget::canRestoreTab() const
{
    return m_closedTabsManager->isClosedTabAvailable();
}

QStackedWidget* TabWidget::locationBars() const
{
    return m_locationBars;
}

ToolButton* TabWidget::buttonClosedTabs() const
{
    return m_buttonClosedTabs;
}

AddTabButton* TabWidget::buttonAddTab() const
{
    return m_buttonAddTab;
}

QList<WebTab*> TabWidget::allTabs(bool withPinned)
{
    QList<WebTab*> allTabs;

    for (int i = 0; i < count(); i++) {
        WebTab* tab = weTab(i);
        if (!tab || (!withPinned && tab->isPinned())) {
            continue;
        }
        allTabs.append(tab);
    }

    return allTabs;
}

bool TabWidget::restoreState(const QVector<WebTab::SavedTab> &tabs, int currentTab)
{
    if (tabs.isEmpty()) {
        return false;
    }

    QVector<QPair<WebTab*, QVector<int>>> childTabs;

    for (int i = 0; i < tabs.size(); ++i) {
        WebTab::SavedTab tab = tabs.at(i);
        WebTab *webTab = weTab(addView(QUrl(), Qz::NT_CleanSelectedTab, false, tab.isPinned));
        webTab->restoreTab(tab);
        if (!tab.childTabs.isEmpty()) {
            childTabs.append({webTab, tab.childTabs});
        }
    }

    for (const auto p : qAsConst(childTabs)) {
        const auto indices = p.second;
        for (int index : indices) {
            WebTab *t = weTab(index);
            if (t) {
                p.first->addChildTab(t);
            }
        }
    }

    setCurrentIndex(currentTab);
    QTimer::singleShot(0, m_tabBar, SLOT(ensureVisible(int,int)));

    weTab()->tabActivated();

    return true;
}

TabWidget::~TabWidget()
{
    delete m_closedTabsManager;
}
