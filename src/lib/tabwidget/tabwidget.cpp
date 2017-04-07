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

void AddTabButton::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* mime = event->mimeData();

    if (mime->hasUrls()) {
        event->acceptProposedAction();
        return;
    }

    ToolButton::dragEnterEvent(event);
}

void AddTabButton::dropEvent(QDropEvent* event)
{
    const QMimeData* mime = event->mimeData();

    if (!mime->hasUrls()) {
        ToolButton::dropEvent(event);
        return;
    }

    foreach (const QUrl &url, mime->urls()) {
        m_tabWidget->addView(url, Qz::NT_SelectedNewEmptyTab);
    }
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

TabWidget::TabWidget(BrowserWindow* window, QWidget* parent)
    : TabStackedWidget(parent)
    , m_window(window)
    , m_locationBars(new QStackedWidget)
    , m_closedTabsManager(new ClosedTabsManager)
    , m_lastTabIndex(-1)
    , m_lastBackgroundTabIndex(-1)
{
    setObjectName(QSL("tabwidget"));

    m_tabBar = new TabBar(m_window, this);
    setTabBar(m_tabBar);

    connect(this, SIGNAL(currentChanged(int)), m_window, SLOT(refreshHistory()));
    connect(this, &TabWidget::changed, mApp, &MainApplication::changeOccurred);
    connect(this, &TabStackedWidget::pinStateChanged, this, &TabWidget::changed);

    connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(requestCloseTab(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(m_tabBar, SIGNAL(stopTab(int)), this, SLOT(stopTab(int)));
    connect(m_tabBar, SIGNAL(closeAllButCurrent(int)), this, SLOT(closeAllButCurrent(int)));
    connect(m_tabBar, SIGNAL(closeToRight(int)), this, SLOT(closeToRight(int)));
    connect(m_tabBar, SIGNAL(closeToLeft(int)), this, SLOT(closeToLeft(int)));
    connect(m_tabBar, SIGNAL(duplicateTab(int)), this, SLOT(duplicateTab(int)));
    connect(m_tabBar, SIGNAL(detachTab(int)), this, SLOT(detachTab(int)));
    connect(m_tabBar, SIGNAL(tabMoved(int,int)), this, SLOT(tabMoved(int,int)));

    connect(m_tabBar, SIGNAL(moveAddTabButton(int)), this, SLOT(moveAddTabButton(int)));

    connect(mApp, SIGNAL(settingsReloaded()), this, SLOT(loadSettings()));

    m_menuTabs = new MenuTabs(this);
    connect(m_menuTabs, SIGNAL(closeTab(int)), this, SLOT(requestCloseTab(int)));

    m_menuClosedTabs = new QMenu(this);

    // AddTab button displayed next to last tab
    m_buttonAddTab = new AddTabButton(this, m_tabBar);
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

WebTab* TabWidget::weTab()
{
    return weTab(currentIndex());
}

WebTab* TabWidget::weTab(int index)
{
    return qobject_cast<WebTab*>(widget(index));
}

TabIcon* TabWidget::tabIcon(int index)
{
    return weTab(index)->tabIcon();
}

bool TabWidget::validIndex(int index) const
{
    return index >= 0 && index < count();
}

void TabWidget::updateClosedTabsButton()
{
    if (!m_showClosedTabsButton) {
        m_buttonClosedTabs->hide();
    }

    m_buttonClosedTabs->setEnabled(canRestoreTab());
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
    m_buttonClosedTabs->setVisible(m_showClosedTabsButton);
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

    int i = 0;
    const QLinkedList<ClosedTabsManager::Tab> closedTabs = closedTabsManager()->allClosedTabs();

    foreach (const ClosedTabsManager::Tab &tab, closedTabs) {
        const QString title = QzTools::truncatedText(tab.title, 40);
        QAction* act = m_menuClosedTabs->addAction(tab.icon, title, this, SLOT(restoreClosedTab()));
        act->setData(i++);
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
    m_lastTabIndex = currentIndex();
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
        if (openFlags & Qz::NT_NotSelectedTab && m_lastBackgroundTabIndex != -1) {
            position = m_lastBackgroundTabIndex + 1;
        }
        else {
            position = qMax(currentIndex() + 1, m_tabBar->pinnedTabsCount());
        }
    }

    WebTab* webTab = new WebTab(m_window);
    webTab->locationBar()->showUrl(url);
    m_locationBars->addWidget(webTab->locationBar());

    int index = insertTab(position == -1 ? count() : position, webTab, QString(), pinned);
    webTab->attach(m_window);

    if (!title.isEmpty()) {
        m_tabBar->setTabText(index, title);
    }

    if (openFlags & Qz::NT_SelectedTab) {
        setCurrentIndex(index);
    }
    else {
        m_lastBackgroundTabIndex = index;
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
    return index;
}

int TabWidget::addView(WebTab* tab)
{
    m_locationBars->addWidget(tab->locationBar());
    int index = addTab(tab, QString());
    tab->attach(m_window);

    connect(tab->webView(), SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    connect(tab->webView(), SIGNAL(urlChanged(QUrl)), this, SIGNAL(changed()));
    connect(tab->webView(), SIGNAL(ipChanged(QString)), m_window->ipLabel(), SLOT(setText(QString)));

    return index;
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

    TabbedWebView *webView = webTab->webView();

    // Save tab url and history
    if (webView->url().toString() != QL1S("qupzilla:restore"))
        m_closedTabsManager->saveTab(webTab, index);

    m_locationBars->removeWidget(webView->webTab()->locationBar());
    disconnect(webView, SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    disconnect(webView, SIGNAL(urlChanged(QUrl)), this, SIGNAL(changed()));
    disconnect(webView, SIGNAL(ipChanged(QString)), m_window->ipLabel(), SLOT(setText(QString)));

    m_lastBackgroundTabIndex = -1;

    if (m_menuTabs->isVisible()) {
        QAction* labelAction = m_menuTabs->actions().last();
        labelAction->setText(tr("Currently you have %n opened tab(s)", "", count() - 1));
    }

    removeTab(index);
    webTab->deleteLater();

    updateClosedTabsButton();

    emit changed();
}

void TabWidget::requestCloseTab(int index)
{
    if (index == -1)
        index = currentIndex();

    WebTab *webTab = weTab(index);
    if (!webTab || !validIndex(index))
        return;

    TabbedWebView *webView = webTab->webView();

    // Don't close restore page!
    if (webView->url().toString() == QL1S("qupzilla:restore") && mApp->restoreManager())
        return;

    // This would close last tab, so we close the window instead
    if (count() == 1) {
        // If we are not closing window upon closing last tab, let's just load new-tab-url
        if (m_dontCloseWithOneTab) {
            if (webView->url() == m_urlOnNewTab) {
                // We don't want to accumulate more than one closed tab, if user tries
                // to close the last tab multiple times
                m_closedTabsManager->takeLastClosedTab();
            }
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

    m_lastBackgroundTabIndex = -1;
    m_lastTabIndex = index;
    m_currentTabFresh = false;

    WebTab* webTab = weTab(index);
    LocationBar* locBar = webTab->locationBar();

    if (locBar && m_locationBars->indexOf(locBar) != -1) {
        m_locationBars->setCurrentWidget(locBar);
    }

    m_window->currentTabChanged();

    emit changed();
}

void TabWidget::tabMoved(int before, int after)
{
    Q_UNUSED(before)
    Q_UNUSED(after)

    m_lastBackgroundTabIndex = -1;
    m_lastTabIndex = before;

    emit changed();
}

void TabWidget::setCurrentIndex(int index)
{
    m_lastTabIndex = currentIndex();

    TabStackedWidget::setCurrentIndex(index);
}

void TabWidget::nextTab()
{
    QKeyEvent fakeEvent(QKeyEvent::KeyPress, Qt::Key_Tab, Qt::ControlModifier);
    keyPressEvent(&fakeEvent);
}

void TabWidget::previousTab()
{
    QKeyEvent fakeEvent(QKeyEvent::KeyPress, Qt::Key_Backtab, QFlags<Qt::KeyboardModifier>(Qt::ControlModifier + Qt::ShiftModifier));
    keyPressEvent(&fakeEvent);
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

int TabWidget::lastTabIndex() const
{
    return m_lastTabIndex;
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

void TabWidget::detachTab(int index)
{
    WebTab* tab = weTab(index);

    if (tab->isPinned() || count() == 1) {
        return;
    }

    m_locationBars->removeWidget(tab->locationBar());
    disconnect(tab->webView(), SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    disconnect(tab->webView(), SIGNAL(urlChanged(QUrl)), this, SIGNAL(changed()));
    disconnect(tab->webView(), SIGNAL(ipChanged(QString)), m_window->ipLabel(), SLOT(setText(QString)));

    tab->detach();

    BrowserWindow* window = mApp->createWindow(Qz::BW_NewWindow);
    window->setStartTab(tab);
}

int TabWidget::duplicateTab(int index)
{
    if (!validIndex(index)) {
        return -1;
    }

    WebTab* webTab = weTab(index);

    int id = addView(QUrl(), webTab->title(), Qz::NT_CleanNotSelectedTab);
    weTab(id)->p_restoreTab(webTab->url(), webTab->historyData(), webTab->zoomLevel());

    return id;
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

    int index = addView(QUrl(), tab.title, Qz::NT_CleanSelectedTab, false, tab.position);
    WebTab* webTab = weTab(index);
    webTab->p_restoreTab(tab.url, tab.history, tab.zoomLevel);

    updateClosedTabsButton();
}

void TabWidget::restoreAllClosedTabs()
{
    if (!m_closedTabsManager->isClosedTabAvailable()) {
        return;
    }

    const QLinkedList<ClosedTabsManager::Tab> &closedTabs = m_closedTabsManager->allClosedTabs();

    foreach (const ClosedTabsManager::Tab &tab, closedTabs) {
        int index = addView(QUrl(), tab.title, Qz::NT_CleanSelectedTab);
        WebTab* webTab = weTab(index);
        webTab->p_restoreTab(tab.url, tab.history, tab.zoomLevel);
    }

    clearClosedTabsList();
}

void TabWidget::clearClosedTabsList()
{
    m_closedTabsManager->clearList();
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

QByteArray TabWidget::saveState()
{
    int currentTabIndex = 0;
    QVector<WebTab::SavedTab> tabList;

    for (int i = 0; i < count(); ++i) {
        WebTab* webTab = weTab(i);
        if (!webTab)
            continue;

        WebTab::SavedTab tab(webTab);
        if (!tab.isValid())
            continue;

        tabList.append(tab);

        if (webTab->isCurrentTab())
            currentTabIndex = tabList.size() - 1;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << tabList.count();

    foreach (const WebTab::SavedTab &tab, tabList) {
        stream << tab;
    }

    stream << currentTabIndex;

    return data;
}

bool TabWidget::restoreState(const QVector<WebTab::SavedTab> &tabs, int currentTab)
{
    for (int i = 0; i < tabs.size(); ++i) {
        WebTab::SavedTab tab = tabs.at(i);

        int index = addView(QUrl(), Qz::NT_CleanSelectedTab, false, tab.isPinned);
        weTab(index)->restoreTab(tab);

        if (tab.isPinned)
            m_tabBar->updatePinnedTabCloseButton(index);
    }

    setCurrentIndex(currentTab);
    QTimer::singleShot(0, m_tabBar, SLOT(ensureVisible(int,int)));

    // WebTab is restoring state on showEvent
    weTab()->hide();
    weTab()->show();

    return true;
}

void TabWidget::closeRecoveryTab()
{
    foreach (WebTab* tab, allTabs(false)) {
        if (tab->url().toString() == QLatin1String("qupzilla:restore")) {
            closeTab(tab->tabIndex());
        }
    }
}

TabWidget::~TabWidget()
{
    delete m_closedTabsManager;
}
