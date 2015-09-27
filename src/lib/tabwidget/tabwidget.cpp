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
#include "qtwin.h"

#include <QFile>
#include <QTimer>
#include <QMimeData>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QWebHistory>
#include <QWebFrame>
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
    connect(this, SIGNAL(changed()), mApp, SLOT(changeOcurred()));

    connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(m_tabBar, SIGNAL(stopTab(int)), this, SLOT(stopTab(int)));
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(closeAllButCurrent(int)), this, SLOT(closeAllButCurrent(int)));
    connect(m_tabBar, SIGNAL(duplicateTab(int)), this, SLOT(duplicateTab(int)));
    connect(m_tabBar, SIGNAL(detachTab(int)), this, SLOT(detachTab(int)));
    connect(m_tabBar, SIGNAL(tabMoved(int,int)), this, SLOT(tabMoved(int,int)));

    connect(m_tabBar, SIGNAL(moveAddTabButton(int)), this, SLOT(moveAddTabButton(int)));

    connect(mApp, SIGNAL(settingsReloaded()), this, SLOT(loadSettings()));

    m_menuTabs = new MenuTabs(this);
    connect(m_menuTabs, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));

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
        if (!tab) {
            continue;
        }

        QAction* action = new QAction(this);
        action->setIcon(i == currentIndex() ? QIcon(QSL(":/icons/menu/dot.png")) : tab->icon());

        QString title = tab->title();
        title.replace(QLatin1Char('&'), QLatin1String("&&"));
        action->setText(QzTools::truncatedText(title, 40));

        action->setData(QVariant::fromValue(qobject_cast<QWidget*>(tab)));
        connect(action, SIGNAL(triggered()), this, SLOT(actionChangeIndex()));
        m_menuTabs->addAction(action);
    }

    m_menuTabs->addSeparator();
    m_menuTabs->addAction(tr("Currently you have %n opened tab(s)", "", count()))->setEnabled(false);
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
#ifdef Q_OS_WIN
    if (m_window->isTransparentBackgroundAllowed()) {
        QtWin::extendFrameIntoClientArea(m_window);
    }
#endif
    QUrl url = req.url();
    m_lastTabIndex = currentIndex();

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
    connect(webTab->webView(), SIGNAL(changed()), this, SIGNAL(changed()));
    connect(webTab->webView(), SIGNAL(ipChanged(QString)), m_window->ipLabel(), SLOT(setText(QString)));

    if (url.isValid() && url != req.url()) {
        LoadRequest r(req);
        r.setUrl(url);
        webTab->webView()->load(r);
    }
    else {
        webTab->webView()->load(req);
    }

    if (selectLine && m_window->locationBar()->text().isEmpty()) {
        m_window->locationBar()->setFocus();
    }

    if (openFlags & Qz::NT_NotSelectedTab) {
        WebTab* currentWebTab = weTab();
        // Workarounding invalid QWebPage::viewportSize() until QWebView is shown
        // Fixes invalid scrolling to anchor(#) links
        if (currentWebTab && currentWebTab->webView()) {
            TabbedWebView* currentView = currentWebTab->webView();
            webTab->webView()->resize(currentView->size());
            webTab->webView()->page()->setViewportSize(currentView->page()->viewportSize());
        }
    }

    // Make sure user notice opening new background tabs
    if (!(openFlags & Qz::NT_SelectedTab)) {
        m_tabBar->ensureVisible(index);
    }

    emit changed();

#ifdef Q_OS_WIN
    QTimer::singleShot(0, m_window, SLOT(applyBlurToMainWindow()));
#endif
    return index;
}

int TabWidget::addView(WebTab* tab)
{
    m_locationBars->addWidget(tab->locationBar());
    int index = addTab(tab, QString());
    tab->attach(m_window);

    connect(tab->webView(), SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    connect(tab->webView(), SIGNAL(changed()), this, SIGNAL(changed()));
    connect(tab->webView(), SIGNAL(ipChanged(QString)), m_window->ipLabel(), SLOT(setText(QString)));

    return index;
}

void TabWidget::addTabFromClipboard()
{
    QString selectionClipboard = QApplication::clipboard()->text(QClipboard::Selection);
    QUrl guessedUrl = WebView::guessUrlFromString(selectionClipboard);

    if (!guessedUrl.isEmpty()) {
        addView(guessedUrl, Qz::NT_SelectedNewEmptyTab);
    }
}

void TabWidget::closeTab(int index, bool force)
{
    if (index == -1)
        index = currentIndex();

    WebTab* webTab = weTab(index);
    if (!webTab || !validIndex(index))
        return;

    TabbedWebView* webView = webTab->webView();
    bool isRestorePage = webView->url().toString() == QL1S("qupzilla:restore");

    // Don't close restore page!
    if (!force && isRestorePage && mApp->restoreManager())
        return;

    // window.onbeforeunload handling
    if (!webView->onBeforeUnload())
        return;

    // Save tab url and history
    if (!isRestorePage)
        m_closedTabsManager->saveTab(webTab, index);

    // This would close last tab, so we close the window instead
    if (!force && count() == 1) {
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

    m_locationBars->removeWidget(webView->webTab()->locationBar());
    disconnect(webView, SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    disconnect(webView, SIGNAL(changed()), this, SIGNAL(changed()));
    disconnect(webView, SIGNAL(ipChanged(QString)), m_window->ipLabel(), SLOT(setText(QString)));

    m_lastBackgroundTabIndex = -1;

    if (m_menuTabs->isVisible()) {
        QAction* labelAction = m_menuTabs->actions().last();
        labelAction->setText(tr("Currently you have %n opened tabs", "", count() - 1));
    }

    removeTab(index);
    webTab->deleteLater();

    updateClosedTabsButton();

    emit changed();
}

void TabWidget::currentTabChanged(int index)
{
    if (!validIndex(index))
        return;

    m_lastBackgroundTabIndex = -1;
    m_lastTabIndex = index;

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
        closeTab(tabIndex);
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
    disconnect(tab->webView(), SIGNAL(changed()), this, SIGNAL(changed()));
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

    const QUrl url = webTab->url();
    const QString title = webTab->title();
    const QByteArray history = webTab->historyData();

    QNetworkRequest req(url);
    req.setRawHeader("Referer", url.toEncoded());
    req.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));

    int id = addView(req, title, Qz::NT_CleanNotSelectedTab);
    weTab(id)->setHistoryData(history);

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
    webTab->p_restoreTab(tab.url, tab.history);

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
        webTab->p_restoreTab(tab.url, tab.history);
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

void TabWidget::savePinnedTabs()
{
    if (mApp->isPrivate()) {
        return;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << Qz::sessionVersion;

    QStringList tabs;
    QList<QByteArray> tabsHistory;
    for (int i = 0; i < count(); ++i) {
        WebTab* tab = weTab(i);
        if (!tab || !tab->isPinned()) {
            continue;
        }

        tabs.append(tab->url().toEncoded());
        tabsHistory.append(tab->historyData());
    }

    stream << tabs;
    stream << tabsHistory;

    QFile file(DataPaths::currentProfilePath() + "/pinnedtabs.dat");
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
}

void TabWidget::restorePinnedTabs()
{
    if (mApp->isPrivate()) {
        return;
    }

    QFile file(DataPaths::currentProfilePath() + "/pinnedtabs.dat");
    file.open(QIODevice::ReadOnly);
    QByteArray sd = file.readAll();
    file.close();

    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd()) {
        return;
    }

    int version;
    stream >> version;
    if (version != Qz::sessionVersion && version != Qz::sessionVersionQt5) {
        return;
    }

    QStringList pinnedTabs;
    stream >> pinnedTabs;
    QList<QByteArray> tabHistory;
    stream >> tabHistory;

    for (int i = 0; i < pinnedTabs.count(); ++i) {
        QUrl url = QUrl::fromEncoded(pinnedTabs.at(i).toUtf8());

        QByteArray historyState = tabHistory.value(i);
        int addedIndex;

        if (!historyState.isEmpty()) {
            addedIndex = addView(QUrl(), Qz::NT_CleanSelectedTab, false, true);

            weTab(addedIndex)->p_restoreTab(url, historyState);
        }
        else {
            addedIndex = addView(url, tr("New tab"), Qz::NT_SelectedTab, false, -1, true);
        }

        WebTab* webTab = weTab(addedIndex);

        if (webTab) {
            webTab->setPinned(true);
        }

        m_tabBar->updatePinnedTabCloseButton(addedIndex);
    }
}

QByteArray TabWidget::saveState()
{
    QVector<WebTab::SavedTab> tabList;

    for (int i = 0; i < count(); ++i) {
        WebTab* webTab = weTab(i);
        if (!webTab)
            continue;

        WebTab::SavedTab tab(webTab);
        tabList.append(tab);
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << tabList.count();

    foreach (const WebTab::SavedTab &tab, tabList) {
        stream << tab;
    }

    stream << currentIndex();

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

    // WebTab is restoring state on showEvent
    weTab()->hide();
    weTab()->show();

    return true;
}

void TabWidget::closeRecoveryTab()
{
    foreach (WebTab* tab, allTabs(false)) {
        if (tab->url().toString() == QLatin1String("qupzilla:restore")) {
            closeTab(tab->tabIndex(), true);
        }
    }
}

TabWidget::~TabWidget()
{
    delete m_closedTabsManager;
}
