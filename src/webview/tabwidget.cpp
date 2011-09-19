/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "webview.h"
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

class NewTabButton : public QToolButton
{
public:
    explicit NewTabButton(QWidget* parent ) : QToolButton(parent)
    {
#ifndef Q_WS_WIN
        setIcon(QIcon::fromTheme("list-add"));
        setIconSize(QSize(16,16));
        setAutoRaise(true);
#endif
    }
    QSize sizeHint() const
    {
        QSize siz = QToolButton::sizeHint();
        siz.setWidth(26);
        return siz;
    }

#ifdef Q_WS_WIN
private:
    void paintEvent(QPaintEvent*)
    {
        QPainter p(this);
        QStyleOptionTabV3 opt;
        opt.init(this);
        style()->drawControl(QStyle::CE_TabBarTab, &opt, &p, this);

        QPixmap pix(":/icons/other/list-add.png");
        QRect r = this->rect();
        r.setHeight(r.height()+3);
        r.setWidth(r.width()+3);
        style()->drawItemPixmap(&p, r, Qt::AlignCenter, pix);
    }
#endif
};

class TabListButton : public QToolButton
{
public:
    explicit TabListButton(QWidget* parent ) : QToolButton(parent)
    {
    }

    QSize sizeHint() const
    {
        QSize siz = QToolButton::sizeHint();
        siz.setWidth(20);
        return siz;
    }

private:
    void paintEvent(QPaintEvent*)
    {
        QPainter p(this);
        QStyleOptionToolButton opt;
        opt.init(this);
        if (isDown())
            opt.state |= QStyle::State_On;
        if (opt.state & QStyle::State_MouseOver)
            opt.activeSubControls = QStyle::SC_ToolButton;
        if (!isChecked() && !isDown())
                opt.state |= QStyle::State_Raised;
        opt.state |= QStyle::State_AutoRaise;

        style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
    }
};

TabWidget::TabWidget(QupZilla* mainClass, QWidget* parent) :
    QTabWidget(parent)
  , p_QupZilla(mainClass)
  , m_lastTabIndex(0)
  , m_isClosingToLastTabIndex(false)
  , m_closedTabsManager(new ClosedTabsManager(this))
  , m_locationBars(new QStackedWidget())
{
    setObjectName("tabwidget");
    m_tabBar = new TabBar(p_QupZilla);
    setTabBar(m_tabBar);

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
    connect(this, SIGNAL(currentChanged(int)), p_QupZilla, SLOT(refreshHistory()));
//    connect(this, SIGNAL(currentChanged(int)), p_QupZilla->locationBar(), SLOT(siteIconChanged()));

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(backTab(int)), this, SLOT(backTab(int)));
    connect(m_tabBar, SIGNAL(forwardTab(int)), this, SLOT(forwardTab(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(m_tabBar, SIGNAL(stopTab(int)), this, SLOT(stopTab(int)));
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(closeAllButCurrent(int)), this, SLOT(closeAllButCurrent(int)));
    connect(m_tabBar, SIGNAL(duplicateTab(int)), this, SLOT(duplicateTab(int)));
    connect(m_tabBar, SIGNAL(tabMoved(int,int)), this, SLOT(tabMoved(int,int)));

    m_buttonListTabs = new ToolButton(this);
    m_buttonListTabs->setObjectName("tabwidget-button-opentabs");
    m_menuTabs = new QMenu();
    m_buttonListTabs->setMenu(m_menuTabs);
    m_buttonListTabs->setPopupMode(QToolButton::InstantPopup);
    m_buttonListTabs->setToolTip(tr("Show list of opened tabs"));
    m_buttonListTabs->setAutoRaise(true);

    connect(m_menuTabs, SIGNAL(aboutToShow()), this, SLOT(aboutToShowTabsMenu()));

    loadSettings();
}

void TabWidget::loadSettings()
{
    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Browser-Tabs-Settings");
    m_hideCloseButtonWithOneTab = settings.value("hideCloseButtonWithOneTab",false).toBool();
    m_hideTabBarWithOneTab = settings.value("hideTabsWithOneTab",false).toBool();
    settings.endGroup();
    settings.beginGroup("Web-URL-Settings");
    m_urlOnNewTab = settings.value("newTabUrl","").toUrl();
    settings.endGroup();

    m_tabBar->loadSettings();
}

void TabWidget::resizeEvent(QResizeEvent *e)
{
    QPoint posit;
    posit.setY(0);
    posit.setX(width() - m_buttonListTabs->width());
    m_buttonListTabs->move(posit);
    m_buttonListTabs->setVisible(getTabBar()->isVisible());

    QTabWidget::resizeEvent(e);
}

void TabWidget::aboutToShowTabsMenu()
{
    m_menuTabs->clear();
    WebView* actView = weView();
    if (!actView)
        return;
    for (int i = 0; i<count(); i++) {
        WebView* view = weView(i);
        if (!view)
            continue;
        QAction* action = new QAction(this);
        if (view == actView)
            action->setIcon(QIcon(":/icons/menu/dot.png"));
        else
            action->setIcon(_iconForUrl(view->url()));
        if (view->title().isEmpty()) {
            if (view->isLoading()) {
                action->setText(tr("Loading..."));
                action->setIcon(QIcon(":/icons/other/progress.gif"));
            }else
                action->setText(tr("No Named Page"));
        }
        else{
            QString title = view->title();
            if (title.length()>40) {
                title.truncate(40);
                title+="..";
            }
            action->setText(title);
        }
        action->setData(i);
        connect(action, SIGNAL(triggered()), this, SLOT(actionChangeIndex()));

        m_menuTabs->addAction(action);
    }
    m_menuTabs->addSeparator();
    m_menuTabs->addAction(tr("Actually You have %1 opened tabs").arg(count()))->setEnabled(false);
}

void TabWidget::actionChangeIndex()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        setCurrentIndex(action->data().toInt());
    }
}

int TabWidget::addView(QUrl url, const QString &title, OpenUrlIn openIn, bool selectLine)
{
    m_lastTabIndex = currentIndex();

    if (url.isEmpty())
        url = m_urlOnNewTab;

    LocationBar* locBar = new LocationBar(p_QupZilla);
    m_locationBars->addWidget(locBar);
    int index = addTab(new WebTab(p_QupZilla, locBar),"");
    WebView* webView = weView(index);
    locBar->setWebView(webView);

    setTabText(index, title);
    webView->animationLoading(index, true)->movie()->stop();
    webView->animationLoading(index, false)->setPixmap(_iconForUrl(url).pixmap(16,16));

    if (openIn == TabWidget::NewSelectedTab) {
        setCurrentIndex(index);
        p_QupZilla->locationBar()->setText(url.toEncoded());
        p_QupZilla->locationBar()->setCursorPosition(0);
    }
    if (count() == 1 && m_hideTabBarWithOneTab)
        tabBar()->setVisible(false);
    else tabBar()->setVisible(true);

    if (count() == 1 && m_hideCloseButtonWithOneTab)
        tabBar()->setTabsClosable(false);
    else tabBar()->setTabsClosable(true);

//    connect(weView(index), SIGNAL(siteIconChanged()), p_QupZilla->locationBar(), SLOT(siteIconChanged()));
//    connect(weView(index), SIGNAL(showUrl(QUrl)), p_QupZilla->locationBar(), SLOT(showUrl(QUrl)));
    connect(webView, SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    connect(webView, SIGNAL(changed()), mApp, SLOT(setStateChanged()));
    connect(webView, SIGNAL(ipChanged(QString)), p_QupZilla->ipLabel(), SLOT(setText(QString)));

    if (url.isValid())
        webView->load(url);

    if (selectLine)
        p_QupZilla->locationBar()->setFocus();

    if (openIn == NewSelectedTab) {
        m_isClosingToLastTabIndex = true;
        m_locationBars->setCurrentWidget(locBar);
    }

    return index;
}

void TabWidget::setTabText(int index, const QString& text)
{
    QString newtext = text + "                                                                            ";

    if (WebTab* webTab = qobject_cast<WebTab*>(p_QupZilla->tabWidget()->widget(index)) ) {
        if (webTab->isPinned())
            newtext = "";
    }
    QTabWidget::setTabText(index, newtext);
}

void TabWidget::closeTab(int index)
{
    if (count() == 1)
        return;
    if (index == -1)
        index = currentIndex();

    WebView* webView = weView(index);
    if (!webView)
        return;

    m_locationBars->removeWidget(webView->webTab()->locationBar());
//        disconnect(weView(index), SIGNAL(siteIconChanged()), p_QupZilla->locationBar(), SLOT(siteIconChanged()));
//        disconnect(weView(index), SIGNAL(showUrl(QUrl)), p_QupZilla->locationBar(), SLOT(showUrl(QUrl)));
    disconnect(webView, SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    disconnect(webView, SIGNAL(changed()), mApp, SLOT(setStateChanged()));
    disconnect(webView, SIGNAL(ipChanged(QString)), p_QupZilla->ipLabel(), SLOT(setText(QString)));
    //Save last tab url and history
    m_closedTabsManager->saveView(webView);

    if (m_isClosingToLastTabIndex && m_lastTabIndex < count())
        setCurrentIndex(m_lastTabIndex);

    delete widget(index);

    if (count() == 1 && m_hideCloseButtonWithOneTab)
        tabBar()->setTabsClosable(false);
    if (count() == 1 && m_hideTabBarWithOneTab)
        tabBar()->setVisible(false);

//    if (count() < 1)
//        p_QupZilla->close();
}

void TabWidget::tabMoved(int before, int after)
{
    Q_UNUSED(before)
    Q_UNUSED(after)
    m_isClosingToLastTabIndex = false;
}

void TabWidget::currentTabChanged(int index)
{
    if (index < 0)
        return;

    m_isClosingToLastTabIndex = false;
    WebView* webView = weView();
    LocationBar* locBar = webView->webTab()->locationBar();

    QString title = webView->title();
    if (title.isEmpty())
        title = tr("No Named Page");

    p_QupZilla->setWindowTitle(title + " - QupZilla");
//    p_QupZilla->locationBar()->showUrl(weView()->url(),false);

    if (m_locationBars->indexOf(locBar) != -1)
        m_locationBars->setCurrentWidget(locBar);
    p_QupZilla->ipLabel()->setText(webView->getIp());

    if (webView->isLoading()) {
        p_QupZilla->ipLabel()->hide();
        p_QupZilla->progressBar()->setVisible(true);
        p_QupZilla->progressBar()->setValue(webView->getLoading());
        p_QupZilla->navigationBar()->showStopButton();
    } else {
        p_QupZilla->progressBar()->setVisible(false);
        p_QupZilla->navigationBar()->showReloadButton();
        p_QupZilla->ipLabel()->show();
    }

    if (p_QupZilla->inspectorDock() && p_QupZilla->inspectorDock()->isVisible())
        p_QupZilla->showInspector();
    webView->setFocus();

    m_tabBar->updateCloseButton(index);
}

void TabWidget::reloadAllTabs()
{
    for (int i = 0; i<count(); i++) {
        reloadTab(i);
    }
}

void TabWidget::closeAllButCurrent(int index)
{
    WebTab* akt = qobject_cast<WebTab*>(widget(index));

    foreach (WebTab* tab, allTabs(false)) {
        if (akt == widget(tab->view()->tabIndex()))
            continue;
        closeTab(tab->view()->tabIndex());
    }
}

void TabWidget::duplicateTab(int index)
{
    QUrl url = weView(index)->url();
    QByteArray history;
    QDataStream tabHistoryStream(&history, QIODevice::WriteOnly);
    tabHistoryStream << *weView(index)->history();

    int id = addView(url, tr("New tab"), TabWidget::NewSelectedTab);
    QDataStream historyStream(history);
    historyStream >> *weView(id)->history();
}

void TabWidget::restoreClosedTab()
{
    if (!m_closedTabsManager->isClosedTabAvailable())
        return;

    ClosedTabsManager::Tab tab;

    QAction* action = qobject_cast<QAction*>(sender());
    if (action && action->data().toInt() != 0)
        tab = m_closedTabsManager->getTabAt(action->data().toInt());
    else
        tab = m_closedTabsManager->getFirstClosedTab();
    int index = addView(QUrl(), tab.title);
    QDataStream historyStream(tab.history);
    historyStream >> *weView(index)->history();

    weView(index)->load(tab.url);
}

void TabWidget::restoreAllClosedTabs()
{
    if (!m_closedTabsManager->isClosedTabAvailable())
        return;

    QList<ClosedTabsManager::Tab> closedTabs = m_closedTabsManager->allClosedTabs();
    foreach (ClosedTabsManager::Tab tab, closedTabs) {
        int index = addView(QUrl(), tab.title);
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

QList<WebTab*> TabWidget::allTabs(bool withPinned)
{
    QList<WebTab*> allTabs;
    for (int i = 0; i < count(); i++) {
        WebTab* tab = qobject_cast<WebTab*>(widget(i));
        if (!tab || (!withPinned && tab->isPinned()) )
            continue;
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
        if (WebView* tab = weView(i)) {
            WebTab* webTab = qobject_cast<WebTab*>(widget(i));
            if (!webTab || !webTab->isPinned())
                continue;

            tabs.append(QString::fromUtf8(tab->url().toEncoded()));
            if (tab->history()->count() != 0) {
                QByteArray tabHistory;
                QDataStream tabHistoryStream(&tabHistory, QIODevice::WriteOnly);
                tabHistoryStream << *tab->history();
                tabsHistory.append(tabHistory);
            } else {
                tabsHistory << QByteArray();
            }
        } else {
            tabs.append(QString::null);
            tabsHistory.append(QByteArray());
        }
    }
    stream << tabs;
    stream << tabsHistory;
    QFile file(mApp->getActiveProfilPath()+"pinnedtabs.dat");
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
}

void TabWidget::restorePinnedTabs()
{
    QFile file(mApp->getActiveProfilPath()+"pinnedtabs.dat");
    file.open(QIODevice::ReadOnly);
    QByteArray sd = file.readAll();
    file.close();

    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd())
        return;

    QStringList pinnedTabs;
    stream >> pinnedTabs;
    QList<QByteArray> tabHistory;
    stream >> tabHistory;

    for (int i = 0; i < pinnedTabs.count(); ++i) {
        QUrl url = QUrl::fromEncoded(pinnedTabs.at(i).toUtf8());

        QByteArray historyState = tabHistory.value(i);
        int addedIndex;
        if (!historyState.isEmpty()) {
            addedIndex= addView(QUrl());
            QDataStream historyStream(historyState);
            historyStream >> *weView(addedIndex)->history();
            weView(addedIndex)->load(url);
        } else {
            addedIndex = addView(url);
        }
        WebTab* webTab = (WebTab*)widget(addedIndex);
        if (webTab)
            webTab->setPinned(true);

        m_tabBar->moveTab(addedIndex, i);
        m_tabBar->updateCloseButton(i);
    }
}

QByteArray TabWidget::saveState()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    QStringList tabs;
    QList<QByteArray> tabsHistory;
    for (int i = 0; i < count(); ++i) {
        if (WebView* tab = weView(i)) {
            WebTab* webTab = qobject_cast<WebTab*>(widget(i));
            if (webTab && webTab->isPinned())
                continue;

            tabs.append(QString::fromUtf8(tab->url().toEncoded()));
            if (tab->history()->count() != 0) {
                QByteArray tabHistory;
                QDataStream tabHistoryStream(&tabHistory, QIODevice::WriteOnly);
                tabHistoryStream << *tab->history();
                tabsHistory.append(tabHistory);
            } else {
                tabsHistory << QByteArray();
            }
        } else {
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
    if (stream.atEnd())
        return false;

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
            int index = addView(QUrl());
            QDataStream historyStream(historyState);
            historyStream >> *weView(index)->history();
            weView(index)->load(url);
        } else {
            addView(url);
        }
    }

    setCurrentIndex(currentTab);
    return true;
}

TabWidget::~TabWidget()
{
    delete m_menuTabs;
    delete m_buttonListTabs;
}
