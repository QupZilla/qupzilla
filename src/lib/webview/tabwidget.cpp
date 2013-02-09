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
#include "tabwidget.h"
#include "tabbar.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "qupzilla.h"
#include "iconprovider.h"
#include "mainapplication.h"
#include "webtab.h"
#include "clickablelabel.h"
#include "closedtabsmanager.h"
#include "progressbar.h"
#include "navigationbar.h"
#include "locationbar.h"
#include "websearchbar.h"
#include "settings.h"
#include "qzsettings.h"

#include <QMovie>
#include <QMenu>
#include <QMimeData>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QWebHistory>
#include <QClipboard>
#include <QFile>

AddTabButton::AddTabButton(TabWidget* tabWidget, TabBar* tabBar)
    : ToolButton(tabWidget)
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
        QString selectionClipboard = QApplication::clipboard()->text(QClipboard::Selection);
        QUrl guessedUrl = WebView::guessUrlFromString(selectionClipboard);

        if (!guessedUrl.isEmpty()) {
            m_tabWidget->addView(guessedUrl, Qz::NT_SelectedNewEmptyTab);
        }
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

    foreach(const QUrl & url, mime->urls()) {
        m_tabWidget->addView(url, Qz::NT_SelectedNewEmptyTab);
    }
}

TabWidget::TabWidget(QupZilla* mainClass, QWidget* parent)
    : QTabWidget(parent)
    , p_QupZilla(mainClass)
    , m_lastTabIndex(0)
    , m_lastBackgroundTabIndex(-1)
    , m_isClosingToLastTabIndex(false)
    , m_isRestoringState(false)
    , m_closedTabsManager(new ClosedTabsManager)
    , m_locationBars(new QStackedWidget)
{
    setObjectName("tabwidget");
    m_tabBar = new TabBar(p_QupZilla, this);
    setTabBar(m_tabBar);

    connect(this, SIGNAL(currentChanged(int)), p_QupZilla, SLOT(refreshHistory()));

    connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(m_tabBar, SIGNAL(stopTab(int)), this, SLOT(stopTab(int)));
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(closeAllButCurrent(int)), this, SLOT(closeAllButCurrent(int)));
    connect(m_tabBar, SIGNAL(duplicateTab(int)), this, SLOT(duplicateTab(int)));
    connect(m_tabBar, SIGNAL(tabMoved(int, int)), this, SLOT(tabMoved(int, int)));

    connect(m_tabBar, SIGNAL(moveAddTabButton(int)), this, SLOT(moveAddTabButton(int)));
    connect(m_tabBar, SIGNAL(showButtons()), this, SLOT(showButtons()));
    connect(m_tabBar, SIGNAL(hideButtons()), this, SLOT(hideButtons()));

    m_buttonListTabs = new ToolButton(this);
    m_buttonListTabs->setObjectName("tabwidget-button-opentabs");
    m_menuTabs = new QMenu(this);
    m_buttonListTabs->setMenu(m_menuTabs);
    m_buttonListTabs->setPopupMode(QToolButton::InstantPopup);
    m_buttonListTabs->setToolTip(tr("List of tabs"));
    m_buttonListTabs->setAutoRaise(true);
    m_buttonListTabs->setFocusPolicy(Qt::NoFocus);

    m_buttonAddTab = new AddTabButton(this, m_tabBar);

    connect(m_buttonAddTab, SIGNAL(clicked()), p_QupZilla, SLOT(addTab()));
    connect(m_menuTabs, SIGNAL(aboutToShow()), this, SLOT(aboutToShowClosedTabsMenu()));

    loadSettings();
}

void TabWidget::loadSettings()
{
    Settings settings;
    settings.beginGroup("Browser-Tabs-Settings");
    m_hideTabBarWithOneTab = settings.value("hideTabsWithOneTab", false).toBool();
    m_dontQuitWithOneTab = settings.value("dontQuitWithOneTab", false).toBool();
    m_closedInsteadOpened = settings.value("closedInsteadOpenedTabs", false).toBool();
    m_newTabAfterActive = settings.value("newTabAfterActive", true).toBool();
    m_newEmptyTabAfterActive = settings.value("newEmptyTabAfterActive", false).toBool();
    settings.endGroup();

    settings.beginGroup("Web-URL-Settings");
    m_urlOnNewTab = settings.value("newTabUrl", "qupzilla:speeddial").toUrl();
    settings.endGroup();

    m_tabBar->loadSettings();
}

void TabWidget::resizeEvent(QResizeEvent* e)
{
    QPoint posit;
    posit.setY(0);
    //RTL Support
    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        posit.setX(0);
    }
    else {
        posit.setX(width() - m_buttonListTabs->width());
    }
    m_buttonListTabs->move(posit);

    QTabWidget::resizeEvent(e);
}

WebTab* TabWidget::weTab()
{
    return weTab(currentIndex());
}

WebTab* TabWidget::weTab(int index)
{
    return qobject_cast<WebTab*>(widget(index));
}

void TabWidget::showButtons()
{
    m_buttonListTabs->show();
    m_buttonAddTab->show();
}

void TabWidget::hideButtons()
{
    m_buttonListTabs->hide();
    m_buttonAddTab->hide();
}

void TabWidget::moveAddTabButton(int posX)
{
    int posY = (m_tabBar->height() - m_buttonAddTab->height()) / 2;
    //RTL Support
    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        posX = qMax(posX - m_buttonAddTab->width(), m_buttonListTabs->width());
    }
    else {
        posX = qMin(posX, m_tabBar->width() - m_buttonAddTab->width() - m_buttonListTabs->width());
    }
    m_buttonAddTab->move(posX, posY);
}

void TabWidget::aboutToShowTabsMenu()
{
    m_menuTabs->clear();
    WebTab* actTab = weTab();
    if (!actTab) {
        return;
    }
    for (int i = 0; i < count(); i++) {
        WebTab* tab = weTab(i);
        if (!tab) {
            continue;
        }
        QAction* action = new QAction(this);
        if (tab == actTab) {
            action->setIcon(QIcon(":/icons/menu/dot.png"));
        }
        else {
            action->setIcon(tab->icon());
        }
        if (tab->title().isEmpty()) {
            if (tab->isLoading()) {
                action->setText(tr("Loading..."));
                action->setIcon(QIcon(":/icons/other/progress.gif"));
            }
            else {
                action->setText(tr("No Named Page"));
            }
        }
        else {
            QString title = tab->title();
            title.replace(QLatin1Char('&'), QLatin1String("&&"));
            if (title.length() > 40) {
                title.truncate(40);
                title += QLatin1String("..");
            }
            action->setText(title);
        }
        action->setData(i);
        connect(action, SIGNAL(triggered()), this, SLOT(actionChangeIndex()));

        m_menuTabs->addAction(action);
    }
    m_menuTabs->addSeparator();
    m_menuTabs->addAction(tr("Currently you have %1 opened tabs").arg(count()))->setEnabled(false);
}

void TabWidget::actionChangeIndex()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        setCurrentIndex(action->data().toInt());
    }
}

int TabWidget::addView(const QUrl &url, const Qz::NewTabPositionFlags &openFlags, bool selectLine)
{
    return addView(QNetworkRequest(url), openFlags, selectLine);
}

int TabWidget::addView(const QNetworkRequest &req, const Qz::NewTabPositionFlags &openFlags, bool selectLine)
{
    return addView(req, tr("New tab"), openFlags, selectLine);
}

int TabWidget::addView(const QUrl &url, const QString &title, const Qz::NewTabPositionFlags &openFlags, bool selectLine, int position)
{
    return addView(QNetworkRequest(url), title, openFlags, selectLine, position);
}

int TabWidget::addView(QNetworkRequest req, const QString &title, const Qz::NewTabPositionFlags &openFlags, bool selectLine, int position)
{
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

    LocationBar* locBar = new LocationBar(p_QupZilla);
    m_locationBars->addWidget(locBar);
    int index;

    if (position == -1) {
        index = addTab(new WebTab(p_QupZilla, locBar), QString());
    }
    else {
        index = insertTab(position, new WebTab(p_QupZilla, locBar), QString());
    }

    TabbedWebView* webView = weTab(index)->view();
    locBar->setWebView(webView);

    setTabText(index, title);
    setTabIcon(index, qIconProvider->emptyWebIcon());

    if (openFlags & Qz::NT_SelectedTab) {
        setCurrentIndex(index);
    }
    else {
        m_lastBackgroundTabIndex = index;
    }

    connect(webView, SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    connect(webView, SIGNAL(changed()), mApp, SLOT(setStateChanged()));
    connect(webView, SIGNAL(ipChanged(QString)), p_QupZilla->ipLabel(), SLOT(setText(QString)));

    if (url.isValid()) {
        req.setUrl(url);
        webView->load(req);
    }

    if (selectLine) {
        p_QupZilla->locationBar()->setFocus();
    }

    if (openFlags & Qz::NT_SelectedTab || openFlags & Qz::NT_NotSelectedTab) {
        m_isClosingToLastTabIndex = true;
    }

    if (openFlags & Qz::NT_NotSelectedTab) {
        WebTab* currentWebTab = weTab();
        // Workarounding invalid QWebPage::viewportSize() until QWebView is shown
        // Fixes invalid scrolling to anchor(#) links
        if (currentWebTab && currentWebTab->view()) {
            TabbedWebView* currentView = currentWebTab->view();
            webView->resize(currentView->size());
            webView->page()->setViewportSize(currentView->page()->viewportSize());
        }
    }

    return index;
}

void TabWidget::closeTab(int index, bool force)
{
    if (index == -1) {
        index = currentIndex();
    }

    WebTab* webTab = weTab(index);
    if (!webTab || !validIndex(index)) {
        return;
    }

    TabbedWebView* webView = webTab->view();
    WebPage* webPage = webView->page();

    if (!force && webView->url().toString() == QLatin1String("qupzilla:restore") && mApp->restoreManager()) {
        // Don't close restore page!
        return;
    }

    if (!force && count() == 1) {
        if (m_dontQuitWithOneTab && mApp->windowCount() == 1) {
            webView->load(m_urlOnNewTab);
            return;
        }
        else {
            p_QupZilla->close();
            return;
        }
    }

    if (webTab->isPinned()) {
        emit pinnedTabClosed();
    }

    m_locationBars->removeWidget(webView->webTab()->locationBar());
    disconnect(webView, SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    disconnect(webView, SIGNAL(changed()), mApp, SLOT(setStateChanged()));
    disconnect(webView, SIGNAL(ipChanged(QString)), p_QupZilla->ipLabel(), SLOT(setText(QString)));

    //Save last tab url and history
    m_closedTabsManager->saveView(webTab, index);

    if (m_isClosingToLastTabIndex && m_lastTabIndex < count() && index == currentIndex()) {
        setCurrentIndex(m_lastTabIndex);
    }

    m_lastBackgroundTabIndex = -1;

    webPage->disconnectObjects();
    webView->disconnectObjects();
    webTab->disconnectObjects();

    webTab->deleteLater();
}

void TabWidget::currentTabChanged(int index)
{
    if (!validIndex(index) || m_isRestoringState) {
        return;
    }

    m_isClosingToLastTabIndex = m_lastBackgroundTabIndex == index;
    m_lastBackgroundTabIndex = -1;
    m_lastTabIndex = index;

    WebTab* webTab = weTab(index);
    LocationBar* locBar = webTab->locationBar();

    if (locBar && m_locationBars->indexOf(locBar) != -1) {
        m_locationBars->setCurrentWidget(locBar);
    }

    webTab->setCurrentTab();
    p_QupZilla->currentTabChanged();
    showNavigationBar(p_QupZilla->navigationContainer());
}

void TabWidget::tabMoved(int before, int after)
{
    Q_UNUSED(before)
    Q_UNUSED(after)

    m_isClosingToLastTabIndex = false;
    m_lastBackgroundTabIndex = -1;
    m_lastTabIndex = before;
}

void TabWidget::tabInserted(int index)
{
    Q_UNUSED(index)

    tabBar()->setVisible(!(count() == 1 && m_hideTabBarWithOneTab));
}

void TabWidget::tabRemoved(int index)
{
    Q_UNUSED(index)

    tabBar()->setVisible(!(count() == 1 && m_hideTabBarWithOneTab));
}

void TabWidget::startTabAnimation(int index)
{
    if (!validIndex(index)) {
        return;
    }

    QLabel* label = qobject_cast<QLabel*>(m_tabBar->tabButton(index, QTabBar::LeftSide));
    if (!label) {
        label = new QLabel();
        m_tabBar->setTabButton(index, QTabBar::LeftSide, label);
    }

    if (label->movie()) {
        label->movie()->start();
        return;
    }

    QMovie* movie = new QMovie(":icons/other/progress.gif", QByteArray(), label);
    movie->setSpeed(70);
    movie->start();

    label->setMovie(movie);
}

void TabWidget::stopTabAnimation(int index)
{
    if (!validIndex(index)) {
        return;
    }

    QLabel* label = qobject_cast<QLabel*>(m_tabBar->tabButton(index, QTabBar::LeftSide));

    if (label && label->movie()) {
        label->movie()->stop();
    }
}

void TabWidget::setCurrentIndex(int index)
{
    m_lastTabIndex = currentIndex();

    QTabWidget::setCurrentIndex(index);
}

void TabWidget::setTabIcon(int index, const QIcon &icon)
{
    if (!validIndex(index)) {
        return;
    }

    QLabel* label = qobject_cast<QLabel*>(m_tabBar->tabButton(index, QTabBar::LeftSide));
    if (!label) {
        label = new QLabel();
        label->resize(16, 16);
        m_tabBar->setTabButton(index, QTabBar::LeftSide, label);
    }

    label->setPixmap(icon.pixmap(16, 16));
}

void TabWidget::setTabText(int index, const QString &text)
{
    if (!validIndex(index)) {
        return;
    }

    QString newtext = text;
    newtext.replace(QLatin1Char('&'), QLatin1String("&&")); // Avoid Alt+letter shortcuts

    if (WebTab* webTab = weTab(index)) {
        if (webTab->isPinned()) {
            newtext.clear();
        }
    }

    setTabToolTip(index, text);
    QTabWidget::setTabText(index, newtext);
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

void TabWidget::showTabBar()
{
    if (count() == 1 && m_hideTabBarWithOneTab) {
        tabBar()->hide();
    }
    else {
        tabBar()->show();
    }
}

int TabWidget::lastTabIndex() const
{
    return m_lastTabIndex;
}

void TabWidget::showNavigationBar(QWidget* bar)
{
    WebTab* tab = weTab();

    if (tab) {
        tab->showNavigationBar(bar);
    }
}

TabBar* TabWidget::getTabBar() const
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

    foreach(WebTab * tab, allTabs(false)) {
        int tabIndex = tab->tabIndex();
        if (akt == widget(tabIndex)) {
            continue;
        }
        closeTab(tabIndex);
    }
}

int TabWidget::duplicateTab(int index)
{
    if (!validIndex(index)) {
        return -1;
    }

    WebTab* webTab = weTab(index);

    const QUrl &url = webTab->url();
    const QString &title = webTab->title();
    const QByteArray &history = webTab->historyData();

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
        tab = m_closedTabsManager->getTabAt(action->data().toInt());
    }
    else {
        tab = m_closedTabsManager->getFirstClosedTab();
    }

    int index = addView(QUrl(), tab.title, Qz::NT_CleanSelectedTab, false, tab.position);
    WebTab* webTab = weTab(index);
    webTab->p_restoreTab(tab.url, tab.history);
}

void TabWidget::restoreAllClosedTabs()
{
    if (!m_closedTabsManager->isClosedTabAvailable()) {
        return;
    }

    const QList<ClosedTabsManager::Tab> &closedTabs = m_closedTabsManager->allClosedTabs();

    foreach(const ClosedTabsManager::Tab & tab, closedTabs) {
        int index = addView(QUrl(), tab.title, Qz::NT_CleanSelectedTab);
        WebTab* webTab = weTab(index);
        webTab->p_restoreTab(tab.url, tab.history);
    }

    m_closedTabsManager->clearList();
}

void TabWidget::clearClosedTabsList()
{
    m_closedTabsManager->clearList();
}

bool TabWidget::canRestoreTab() const
{
    return m_closedTabsManager->isClosedTabAvailable();
}

QStackedWidget* TabWidget::locationBars() const
{
    return m_locationBars;
}

ToolButton* TabWidget::buttonListTabs() const
{
    return m_buttonListTabs;
}

AddTabButton* TabWidget::buttonAddTab() const
{
    return m_buttonAddTab;
}

void TabWidget::aboutToShowClosedTabsMenu()
{
    if (!m_closedInsteadOpened) {
        aboutToShowTabsMenu();
    }
    else {
        m_menuTabs->clear();
        int i = 0;
        foreach(const ClosedTabsManager::Tab & tab, closedTabsManager()->allClosedTabs()) {
            QString title = tab.title;
            if (title.length() > 40) {
                title.truncate(40);
                title += "..";
            }
            m_menuTabs->addAction(_iconForUrl(tab.url), title, this, SLOT(restoreClosedTab()))->setData(i);
            i++;
        }
        m_menuTabs->addSeparator();
        if (i == 0) {
            m_menuTabs->addAction(tr("Empty"))->setEnabled(false);
        }
        else {
            m_menuTabs->addAction(tr("Restore All Closed Tabs"), this, SLOT(restoreAllClosedTabs()));
            m_menuTabs->addAction(tr("Clear list"), this, SLOT(clearClosedTabsList()));
        }
    }
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
    if (mApp->isPrivateSession()) {
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

    QFile file(mApp->currentProfilePath() + "pinnedtabs.dat");
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
}

void TabWidget::restorePinnedTabs()
{
    if (mApp->isPrivateSession()) {
        return;
    }

    QFile file(mApp->currentProfilePath() + "pinnedtabs.dat");
    file.open(QIODevice::ReadOnly);
    QByteArray sd = file.readAll();
    file.close();

    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd()) {
        return;
    }

    int version;
    stream >> version;
    if (version != Qz::sessionVersion) {
        return;
    }

    QStringList pinnedTabs;
    stream >> pinnedTabs;
    QList<QByteArray> tabHistory;
    stream >> tabHistory;

    m_isRestoringState = true;

    for (int i = 0; i < pinnedTabs.count(); ++i) {
        QUrl url = QUrl::fromEncoded(pinnedTabs.at(i).toUtf8());

        QByteArray historyState = tabHistory.value(i);
        int addedIndex;

        if (!historyState.isEmpty()) {
            addedIndex = addView(QUrl(), Qz::NT_CleanSelectedTab);

            weTab(addedIndex)->p_restoreTab(url, historyState);
        }
        else {
            addedIndex = addView(url);
        }

        WebTab* webTab = weTab(addedIndex);

        if (webTab) {
            webTab->setPinned(true);
            emit pinnedTabAdded();
        }

        m_tabBar->updatePinnedTabCloseButton(addedIndex);
//        m_tabBar->moveTab(addedIndex, i);
    }

    m_isRestoringState = false;
}

QByteArray TabWidget::saveState()
{
    QList<WebTab::SavedTab> tabList;

    for (int i = 0; i < count(); ++i) {
        WebTab* webTab = weTab(i);
        if (!webTab || webTab->isPinned()) {
            continue;
        }

        WebTab::SavedTab tab(webTab);
        tabList.append(tab);
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << tabList.count();

    foreach(const WebTab::SavedTab & tab, tabList) {
        stream << tab;
    }

    stream << currentIndex();

    return data;
}

bool TabWidget::restoreState(const QList<WebTab::SavedTab> &tabs, int currentTab)
{
    m_isRestoringState = true;

    for (int i = 0; i < tabs.size(); ++i) {
        WebTab::SavedTab tab = tabs.at(i);

        int index = addView(QUrl(), Qz::NT_CleanSelectedTab);
        weTab(index)->restoreTab(tab);
    }

    m_isRestoringState = false;

    setCurrentIndex(currentTab);
    currentTabChanged(currentTab);

    return true;
}

void TabWidget::closeRecoveryTab()
{
    foreach(WebTab * tab, allTabs(false)) {
        if (tab->url().toString() == QLatin1String("qupzilla:restore")) {
            closeTab(tab->tabIndex(), true);
        }
    }
}

void TabWidget::disconnectObjects()
{
    disconnect(this);
    disconnect(mApp);
    disconnect(p_QupZilla);
    disconnect(p_QupZilla->ipLabel());
}

TabWidget::~TabWidget()
{
    delete m_closedTabsManager;
}
