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
#include "webpage.h"
#include "qupzilla.h"
#include "tabwidget.h"
#include "tabbar.h"
#include "iconprovider.h"
#include "mainapplication.h"
#include "webtab.h"
#include "clickablelabel.h"
#include "closedtabsmanager.h"
#include "progressbar.h"
#include "navigationbar.h"
#include "toolbutton.h"
#include "locationbar.h"
#include "websearchbar.h"
#include "settings.h"

class NewTabButton : public QToolButton
{
public:
    explicit NewTabButton(QWidget* parent) : QToolButton(parent) {
#ifndef Q_WS_WIN
        setIcon(QIcon::fromTheme("list-add"));
        setIconSize(QSize(16, 16));
        setAutoRaise(true);
#endif
    }
    QSize sizeHint() const {
        QSize siz = QToolButton::sizeHint();
        siz.setWidth(26);
        return siz;
    }

#ifdef Q_WS_WIN
private:
    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        QStyleOptionTabV3 opt;
        opt.init(this);
        style()->drawControl(QStyle::CE_TabBarTab, &opt, &p, this);

        QPixmap pix(":/icons/other/list-add.png");
        QRect r = this->rect();
        r.setHeight(r.height() + 3);
        r.setWidth(r.width() + 3);
        style()->drawItemPixmap(&p, r, Qt::AlignCenter, pix);
    }
#endif
};

class TabListButton : public QToolButton
{
public:
    explicit TabListButton(QWidget* parent) : QToolButton(parent) {
    }

    QSize sizeHint() const {
        QSize siz = QToolButton::sizeHint();
        siz.setWidth(20);
        return siz;
    }

private:
    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        QStyleOptionToolButton opt;
        opt.init(this);
        if (isDown()) {
            opt.state |= QStyle::State_On;
        }
        if (opt.state & QStyle::State_MouseOver) {
            opt.activeSubControls = QStyle::SC_ToolButton;
        }
        if (!isChecked() && !isDown()) {
            opt.state |= QStyle::State_Raised;
        }
        opt.state |= QStyle::State_AutoRaise;

        style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
    }
};

TabWidget::TabWidget(QupZilla* mainClass, QWidget* parent)
    : QTabWidget(parent)
    , p_QupZilla(mainClass)
    , m_lastTabIndex(0)
    , m_isClosingToLastTabIndex(false)
    , m_closedTabsManager(new ClosedTabsManager(this))
    , m_locationBars(new QStackedWidget())
{
    setObjectName("tabwidget");
    m_tabBar = new TabBar(p_QupZilla, this);
    setTabBar(m_tabBar);

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
    connect(this, SIGNAL(currentChanged(int)), p_QupZilla, SLOT(refreshHistory()));

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(backTab(int)), this, SLOT(backTab(int)));
    connect(m_tabBar, SIGNAL(forwardTab(int)), this, SLOT(forwardTab(int)));
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

    m_buttonAddTab = new ToolButton(this);
    m_buttonAddTab->setObjectName("tabwidget-button-addtab");
    m_buttonAddTab->setAutoRaise(true);
    m_buttonAddTab->setToolTip(tr("New Tab"));
    m_buttonAddTab->setFocusPolicy(Qt::NoFocus);

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
    posit.setX(width() - m_buttonListTabs->width());
    m_buttonListTabs->move(posit);

    QTabWidget::resizeEvent(e);
}

TabbedWebView* TabWidget::weView()
{
    return weView(currentIndex());
}

TabbedWebView* TabWidget::weView(int index)
{
    WebTab* webTab = qobject_cast<WebTab*>(widget(index));

    if (!webTab) {
        return 0;
    }

    return webTab->view();
}

void TabWidget::createKeyPressEvent(QKeyEvent* event)
{
    QTabWidget::keyPressEvent(event);
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
    m_buttonAddTab->move(posX, posY);
}

void TabWidget::aboutToShowTabsMenu()
{
    m_menuTabs->clear();
    TabbedWebView* actView = weView();
    if (!actView) {
        return;
    }
    for (int i = 0; i < count(); i++) {
        TabbedWebView* view = weView(i);
        if (!view) {
            continue;
        }
        QAction* action = new QAction(this);
        if (view == actView) {
            action->setIcon(QIcon(":/icons/menu/dot.png"));
        }
        else {
            action->setIcon(view->icon());
        }
        if (view->title().isEmpty()) {
            if (view->isLoading()) {
                action->setText(tr("Loading..."));
                action->setIcon(QIcon(":/icons/other/progress.gif"));
            }
            else {
                action->setText(tr("No Named Page"));
            }
        }
        else {
            QString title = view->title();
            if (title.length() > 40) {
                title.truncate(40);
                title += "..";
            }
            action->setText(title);
        }
        action->setData(i);
        connect(action, SIGNAL(triggered()), this, SLOT(actionChangeIndex()));

        m_menuTabs->addAction(action);
    }
    m_menuTabs->addSeparator();
    m_menuTabs->addAction(tr("Actually you have %1 opened tabs").arg(count()))->setEnabled(false);
}

void TabWidget::actionChangeIndex()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        setCurrentIndex(action->data().toInt());
    }
}

int TabWidget::addView(const QUrl &url, const Qz::NewTabPositionFlags &openFlags, bool selectLine)
{
    return addView(url, tr("New tab"), openFlags, selectLine);
}

int TabWidget::addView(QUrl url, const QString &title, const Qz::NewTabPositionFlags &openFlags, bool selectLine, int position)
{
    m_lastTabIndex = currentIndex();

    if (url.isEmpty() && !(openFlags & Qz::NT_CleanTab)) {
        url = m_urlOnNewTab;
    }

    if (openFlags & Qz::NT_NotSelectedTab) {
        // If we are opening newBgTab from pinned tab, make sure it won't be
        // opened between other pinned tabs
        position = qMax(currentIndex() + 1, m_tabBar->pinnedTabsCount());
    }

    LocationBar* locBar = new LocationBar(p_QupZilla);
    m_locationBars->addWidget(locBar);
    int index;

    if (position == -1) {
        index = addTab(new WebTab(p_QupZilla, locBar), "");
    }
    else {
        index = insertTab(position, new WebTab(p_QupZilla, locBar), "");
    }

    TabbedWebView* webView = weView(index);
    locBar->setWebView(webView);

    setTabText(index, title);
    webView->animationLoading(index, true)->movie()->stop();
    webView->animationLoading(index, false)->setPixmap(_iconForUrl(url).pixmap(16, 16));

    if (openFlags & Qz::NT_SelectedTab) {
        setCurrentIndex(index);
    }

    if (count() == 1 && m_hideTabBarWithOneTab) {
        tabBar()->setVisible(false);
    }
    else {
        tabBar()->setVisible(true);
    }

    connect(webView, SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    connect(webView, SIGNAL(changed()), mApp, SLOT(setStateChanged()));
    connect(webView, SIGNAL(ipChanged(QString)), p_QupZilla->ipLabel(), SLOT(setText(QString)));

    if (url.isValid()) {
        webView->load(url);
    }

    if (selectLine) {
        p_QupZilla->locationBar()->setFocus();
    }

    if (openFlags & Qz::NT_SelectedTab) {
        m_isClosingToLastTabIndex = true;
        m_locationBars->setCurrentWidget(locBar);
    }

    return index;
}

void TabWidget::setTabText(int index, const QString &text)
{
    QString newtext = text;
    newtext.remove("&"); // Avoid Alt+letter shortcuts

    if (WebTab* webTab = qobject_cast<WebTab*>(p_QupZilla->tabWidget()->widget(index))) {
        if (webTab->isPinned()) {
            newtext = "";
        }
    }

    QTabWidget::setTabText(index, newtext);
}

void TabWidget::closeTab(int index)
{
    if (index == -1) {
        index = currentIndex();
    }

    TabbedWebView* webView = weView(index);
    WebPage* webPage = webView->webPage();
    WebTab* webTab = webView->webTab();

    if (!webView || !webPage || !webTab) {
        return;
    }

    if (count() == 1) {
        if (m_dontQuitWithOneTab) {
            webView->setUrl(m_urlOnNewTab);
            webPage->history()->clear();
            p_QupZilla->updateLoadingActions();
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
    m_closedTabsManager->saveView(webView, index);

    if (m_isClosingToLastTabIndex && m_lastTabIndex < count() && index == currentIndex()) {
        setCurrentIndex(m_lastTabIndex);
    }

    if (count() == 2 && m_hideTabBarWithOneTab) {
        tabBar()->setVisible(false);
    }

    webPage->disconnectObjects();
    webView->disconnectObjects();
    webTab->disconnectObjects();

    webTab->deleteLater();
}

void TabWidget::reloadTab(int index)
{
    weView(index)->reload();
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

void TabWidget::tabMoved(int before, int after)
{
    Q_UNUSED(before)
    Q_UNUSED(after)
    m_isClosingToLastTabIndex = false;
}

void TabWidget::currentTabChanged(int index)
{
    if (index < 0) {
        return;
    }

    m_isClosingToLastTabIndex = false;
    TabbedWebView* webView = weView();
    LocationBar* locBar = webView->webTab()->locationBar();

    if (m_locationBars->indexOf(locBar) != -1) {
        m_locationBars->setCurrentWidget(locBar);
    }

    p_QupZilla->currentTabChanged();
    m_tabBar->updateCloseButton(index);
}

void TabWidget::reloadAllTabs()
{
    for (int i = 0; i < count(); i++) {
        reloadTab(i);
    }
}

void TabWidget::stopTab(int index)
{
    weView(index)->stop();
}

void TabWidget::backTab(int index)
{
    weView(index)->back();
}

void TabWidget::forwardTab(int index)
{
    weView(index)->forward();
}

void TabWidget::closeAllButCurrent(int index)
{
    WebTab* akt = qobject_cast<WebTab*>(widget(index));

    foreach(WebTab * tab, allTabs(false)) {
        if (akt == widget(tab->view()->tabIndex())) {
            continue;
        }
        closeTab(tab->view()->tabIndex());
    }
}

int TabWidget::duplicateTab(int index)
{
    QUrl url = weView(index)->url();
    QByteArray history;
    QDataStream tabHistoryStream(&history, QIODevice::WriteOnly);
    tabHistoryStream << *weView(index)->history();

    int id = addView(url, tabText(index), Qz::NT_CleanNotSelectedTab);
    QDataStream historyStream(history);
    historyStream >> *weView(id)->history();

    return id;
}

void TabWidget::restoreClosedTab()
{
    if (!m_closedTabsManager->isClosedTabAvailable()) {
        return;
    }

    ClosedTabsManager::Tab tab;

    QAction* action = qobject_cast<QAction*>(sender());
    if (action && action->data().toInt() != 0) {
        tab = m_closedTabsManager->getTabAt(action->data().toInt());
    }
    else {
        tab = m_closedTabsManager->getFirstClosedTab();
    }

    int index = addView(QUrl(), tab.title, Qz::NT_CleanSelectedTab, false, tab.position);
    QDataStream historyStream(tab.history);
    historyStream >> *weView(index)->history();

    weView(index)->load(tab.url);
}

void TabWidget::restoreAllClosedTabs()
{
    if (!m_closedTabsManager->isClosedTabAvailable()) {
        return;
    }

    QList<ClosedTabsManager::Tab> closedTabs = m_closedTabsManager->allClosedTabs();

    foreach(const ClosedTabsManager::Tab & tab, closedTabs) {
        int index = addView(QUrl(), tab.title, Qz::NT_CleanSelectedTab);
        QDataStream historyStream(tab.history);
        historyStream >> *weView(index)->history();

        weView(index)->load(tab.url);
    }

    m_closedTabsManager->clearList();
}

void TabWidget::clearClosedTabsList()
{
    m_closedTabsManager->clearList();
}

bool TabWidget::canRestoreTab()
{
    return m_closedTabsManager->isClosedTabAvailable();
}

void TabWidget::aboutToShowClosedTabsMenu()
{
    if (!m_closedInsteadOpened) {
        aboutToShowTabsMenu();
    }
    else {
        m_menuTabs->clear();
        int i = 0;
        foreach(const ClosedTabsManager::Tab & tab, this->closedTabsManager()->allClosedTabs()) {
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
        WebTab* tab = qobject_cast<WebTab*>(widget(i));
        if (!tab || (!withPinned && tab->isPinned())) {
            continue;
        }
        allTabs.append(tab);
    }
    return allTabs;
}

void TabWidget::savePinnedTabs()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    QStringList tabs;
    QList<QByteArray> tabsHistory;
    for (int i = 0; i < count(); ++i) {
        if (TabbedWebView* tab = weView(i)) {
            WebTab* webTab = qobject_cast<WebTab*>(widget(i));
            if (!webTab || !webTab->isPinned()) {
                continue;
            }

            tabs.append(tab->url().toString());
            if (tab->history()->count() != 0) {
                QByteArray tabHistory;
                QDataStream tabHistoryStream(&tabHistory, QIODevice::WriteOnly);
                tabHistoryStream << *tab->history();
                tabsHistory.append(tabHistory);
            }
            else {
                tabsHistory << QByteArray();
            }
        }
        else {
            tabs.append(QString::null);
            tabsHistory.append(QByteArray());
        }
    }
    stream << tabs;
    stream << tabsHistory;
    QFile file(mApp->getActiveProfilPath() + "pinnedtabs.dat");
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
}

void TabWidget::restorePinnedTabs()
{
    QFile file(mApp->getActiveProfilPath() + "pinnedtabs.dat");
    file.open(QIODevice::ReadOnly);
    QByteArray sd = file.readAll();
    file.close();

    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd()) {
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
            addedIndex = addView(QUrl(), Qz::NT_CleanSelectedTab);
            QDataStream historyStream(historyState);
            historyStream >> *weView(addedIndex)->history();
            weView(addedIndex)->load(url);
        }
        else {
            addedIndex = addView(url);
        }
        WebTab* webTab = qobject_cast<WebTab*>(widget(addedIndex));
        if (webTab) {
            webTab->setPinned(true);
            emit pinnedTabAdded();
        }

        m_tabBar->updateCloseButton(addedIndex);
        m_tabBar->moveTab(addedIndex, i);
    }
}

QByteArray TabWidget::saveState()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    QStringList tabs;
    QList<QByteArray> tabsHistory;
    for (int i = 0; i < count(); ++i) {
        if (TabbedWebView* tab = weView(i)) {
            WebTab* webTab = qobject_cast<WebTab*>(widget(i));
            if (webTab && webTab->isPinned()) {
                continue;
            }

            tabs.append(tab->url().toString());
            if (tab->history()->count() != 0) {
                QByteArray tabHistory;
                QDataStream tabHistoryStream(&tabHistory, QIODevice::WriteOnly);
                tabHistoryStream << *tab->history();
                tabsHistory.append(tabHistory);
            }
            else {
                tabsHistory << QByteArray();
            }
        }
        else {
            tabs.append(QString::null);
            tabsHistory.append(QByteArray());
        }
    }
    stream << tabs;
    stream << currentIndex();
    stream << tabsHistory;

    return data;
}

bool TabWidget::restoreState(const QByteArray &state)
{
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd()) {
        return false;
    }

    QStringList openTabs;
    int currentTab;
    QList<QByteArray> tabHistory;
    stream >> openTabs;
    stream >> currentTab;
    stream >> tabHistory;

    for (int i = 0; i < openTabs.count(); ++i) {
        QUrl url = QUrl::fromEncoded(openTabs.at(i).toUtf8());

        QByteArray historyState = tabHistory.value(i);
        if (!historyState.isEmpty()) {
            int index = addView(QUrl(), Qz::NT_CleanSelectedTab);
            QDataStream historyStream(historyState);
            historyStream >> *weView(index)->history();
            weView(index)->load(url);
        }
        else {
            addView(url);
        }
    }

    setCurrentIndex(currentTab);
    return true;
}

void TabWidget::disconnectObjects()
{
    disconnect(this);
    disconnect(p_QupZilla);
    disconnect(mApp);
    disconnect(p_QupZilla->ipLabel());
}

TabWidget::~TabWidget()
{
}
