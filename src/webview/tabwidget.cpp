#include "webview.h"
#include "webpage.h"
#include "qupzilla.h"
#include "tabwidget.h"
#include "tabbar.h"
#include "locationbar.h"
#include "mainapplication.h"
#include "webtab.h"

TabWidget::TabWidget(QupZilla* mainClass, QWidget *parent) :
    QTabWidget(parent)
    ,p_QupZilla(mainClass)
    ,m_canRestoreTab(false)
    ,m_lastTabIndex(0)
    ,m_lastTabUrl(0)
    ,m_lastTabHistory(0)
{
    m_tabBar = new TabBar(p_QupZilla);
    setTabBar(m_tabBar);
    setObjectName("tabWidget");
    setStyleSheet(" QTabBar::tab{max-width:250px; height: 28px;}");

    loadSettings();

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(this, SIGNAL(currentChanged(int)), p_QupZilla, SLOT(refreshHistory()));
    connect(this, SIGNAL(currentChanged(int)), p_QupZilla->locationBar(), SLOT(siteIconChanged()));

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(backTab(int)), this, SLOT(backTab(int)));
    connect(m_tabBar, SIGNAL(forwardTab(int)), this, SLOT(forwardTab(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(m_tabBar, SIGNAL(stopTab(int)), this, SLOT(stopTab(int)));
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(closeAllButCurrent(int)), this, SLOT(closeAllButCurrent(int)));

    m_buttonListTabs = new QToolButton(this);
    m_menuTabs = new QMenu();
    m_buttonListTabs->setMenu(m_menuTabs);
    m_buttonListTabs->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_buttonListTabs->setPopupMode(QToolButton::InstantPopup);
    m_buttonListTabs->setAutoRaise(true);
    m_buttonListTabs->setToolTip(tr("Show list of opened tabs"));
    connect(m_menuTabs, SIGNAL(aboutToShow()), this, SLOT(aboutToShowTabsMenu()));
    setCornerWidget(m_buttonListTabs);

    m_buttonAddTab = new QToolButton(this);
    m_buttonAddTab->setIcon(QIcon(":/icons/other/plus.png"));
    m_buttonAddTab->setToolTip(tr("Add Tab"));
    m_buttonAddTab->setAutoRaise(true);
    connect(m_buttonAddTab, SIGNAL(clicked()), p_QupZilla, SLOT(addTab()));
    setCornerWidget(m_buttonAddTab, Qt::TopLeftCorner);
}

void TabWidget::loadSettings()
{
    QSettings settings(MainApplication::getInstance()->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Browser-Tabs-Settings");
    m_hideCloseButtonWithOneTab = settings.value("hideCloseButtonWithOneTab",false).toBool();
    m_hideTabBarWithOneTab = settings.value("hideTabsWithOneTab",false).toBool();
    settings.endGroup();
    settings.beginGroup("Web-URL-Settings");
    m_urlOnNewTab = settings.value("newTabUrl","").toUrl();
    settings.endGroup();

    m_tabBar->loadSettings();
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
            action->setIcon(QIcon(":/icons/menu/circle.png"));
        else
            action->setIcon(LocationBar::icon(view->url()));
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
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        setCurrentIndex(action->data().toInt());
    }
}

int TabWidget::addView(QUrl url, QString title, OpenUrlIn openIn, bool selectLine)
{
    if (url.isEmpty())
        url = m_urlOnNewTab;

    int index = addTab(new WebTab(p_QupZilla),"");
    setTabText(index, title);
    weView(index)->animationLoading(index, true)->movie()->stop();
    weView(index)->animationLoading(index, false)->setPixmap(LocationBar::icon(url).pixmap(16,16));

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

    connect(weView(index), SIGNAL(siteIconChanged()), p_QupZilla->locationBar(), SLOT(siteIconChanged()));
    connect(weView(index), SIGNAL(showUrl(QUrl)), p_QupZilla->locationBar(), SLOT(showUrl(QUrl)));
    connect(weView(index), SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
    connect(weView(index), SIGNAL(changed()), p_QupZilla->getMainApp(), SLOT(setChanged()));
    connect(weView(index), SIGNAL(ipChanged(QString)), p_QupZilla->ipLabel(), SLOT(setText(QString)));

    if (url.isValid())
        weView(index)->load(url);
    if (selectLine)
        p_QupZilla->locationBar()->setFocus();

    return index;
}

void TabWidget::setTabText(int index, const QString& text)
{
    QString newtext = text + "                                                               ";
    QTabWidget::setTabText(index, newtext);
}

void TabWidget::closeTab(int index)
{
    if (count() == 1)
        return;
    if (index == -1)
        index = currentIndex();
    else m_lastTabIndex-=1;

    if (weView(index)) {
        disconnect(weView(index), SIGNAL(siteIconChanged()), p_QupZilla->locationBar(), SLOT(siteIconChanged()));
        disconnect(weView(index), SIGNAL(showUrl(QUrl)), p_QupZilla->locationBar(), SLOT(showUrl(QUrl)));
        disconnect(weView(index), SIGNAL(wantsCloseTab(int)), this, SLOT(closeTab(int)));
        disconnect(weView(index), SIGNAL(changed()), p_QupZilla->getMainApp(), SLOT(setChanged()));
        disconnect(weView(index), SIGNAL(ipChanged(QString)), p_QupZilla->ipLabel(), SLOT(setText(QString)));
        //Save last tab url and history
        if (!weView(index)->url().isEmpty()) {
            m_lastTabUrl = weView(index)->url().toString();
            QDataStream tabHistoryStream(&m_lastTabHistory, QIODevice::WriteOnly);
            tabHistoryStream << *weView(index)->history();
            m_canRestoreTab = true;
        }
        //weView(index)->page()->~QWebPage();
        //weView(index)->~QWebView();
        delete weView(index);
        removeTab(index);

        if (count() == 1 && m_hideCloseButtonWithOneTab)
            tabBar()->setTabsClosable(false);
        if (count() == 1 && m_hideTabBarWithOneTab)
            tabBar()->setVisible(false);
    }
//    if (count() < 1)
//        p_QupZilla->close();
}

void TabWidget::tabChanged(int index)
{
    if (index<0)
        return;

    QString title = p_QupZilla->weView()->title();
    if (title.isEmpty())
        title = tr("No Named Page");

    p_QupZilla->setWindowTitle(title + " - QupZilla");
    p_QupZilla->locationBar()->showUrl(weView()->url(),false);
    p_QupZilla->ipLabel()->setText(weView()->getIp());

    if (p_QupZilla->inspectorDock() && p_QupZilla->inspectorDock()->isVisible())
        p_QupZilla->showInspector();

    weView()->setFocus();

    m_lastTabIndex = index;
}

void TabWidget::reloadAllTabs()
{
    for (int i = 0;i<count();i++) {
        reloadTab(i);
    }
}

void TabWidget::closeAllButCurrent(int index)
{
    WebView* akt = qobject_cast<WebView*>(widget(index));

    int cycleCounter = 0;          // Only tab count * 1.6 attempts to
    int maxCycles = count()*1.6;   // close tabs -> it sometimes hangs here
    while(count()!=1) {
        for (int i = 0;i<=count();i++) {
            if (widget(i) == akt)
                continue;
            closeTab(i);
            cycleCounter++;

            if (cycleCounter >= maxCycles)
                break;
        }
        if (cycleCounter >= maxCycles)
            break;
    }
}

void TabWidget::restoreClosedTab()
{
    if (m_lastTabUrl.isEmpty())
        return;
    int index = addView(QUrl());
    QDataStream historyStream(m_lastTabHistory);
    historyStream >> *weView(index)->history();
    weView(index)->load(m_lastTabUrl);
    m_canRestoreTab = false;
}

QByteArray TabWidget::saveState()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    QStringList tabs;
    QList<QByteArray> tabsHistory;
    for (int i = 0; i < count(); ++i) {
        if (WebView *tab = weView(i)) {
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
    stream >> openTabs;

    int currentTab;
    stream >> currentTab;
    setCurrentIndex(currentTab);
    QList<QByteArray> tabHistory;
    stream >> tabHistory;


    for (int i = 0; i < openTabs.count(); ++i) {
        QUrl url = QUrl::fromEncoded(openTabs.at(i).toUtf8());
        //TabWidget::OpenUrlIn tab =

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
    return true;
}

TabWidget::~TabWidget()
{
    int index = currentIndex();
    closeAllButCurrent(index);
    closeTab(index);
    delete m_menuTabs;
    delete m_buttonAddTab;
    delete m_buttonListTabs;
}
