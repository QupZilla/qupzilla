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
#include "navigationbar.h"
#include "toolbutton.h"
#include "qupzilla.h"
#include "iconprovider.h"
#include "websearchbar.h"
#include "reloadstopbutton.h"

NavigationBar::NavigationBar(QupZilla *mainClass, QWidget *parent)
    : QWidget(parent)
    , p_QupZilla(mainClass)
{
    setObjectName("navigationbar");
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(9, 3, 9, 3);
    setLayout(m_layout);

    m_buttonBack = new ToolButton();
    m_buttonBack->setObjectName("navigation-button-back");
    m_buttonBack->setToolTip(tr("Back"));
    m_buttonBack->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonBack->setAutoRaise(true);
    m_buttonBack->setEnabled(false);

    m_buttonNext = new ToolButton();
    m_buttonNext->setObjectName("navigation-button-next");
    m_buttonNext->setToolTip(tr("Forward"));
    m_buttonNext->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonNext->setAutoRaise(true);
    m_buttonNext->setEnabled(false);

    QHBoxLayout* backNextLayout = new QHBoxLayout();
    backNextLayout->setSpacing(0);
    backNextLayout->addWidget(m_buttonBack);
    backNextLayout->addWidget(m_buttonNext);

    m_reloadStop = new ReloadStopButton();

    m_buttonHome = new ToolButton();
    m_buttonHome->setObjectName("navigation-button-home");
    m_buttonHome->setToolTip(tr("Home"));
    m_buttonHome->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonHome->setAutoRaise(true);

    m_buttonAddTab = new ToolButton();
    m_buttonAddTab->setObjectName("navigation-button-addtab");
    m_buttonAddTab->setToolTip(tr("New Tab"));
    m_buttonAddTab->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonAddTab->setAutoRaise(true);

    m_menuBack = new QMenu();
    m_buttonBack->setMenu(m_menuBack);
    m_menuForward = new QMenu();
    m_buttonNext->setMenu(m_menuForward);

    m_supMenu = new ToolButton(this);
    m_supMenu->setObjectName("navigation-button-supermenu");
    m_supMenu->setPopupMode(QToolButton::InstantPopup);
    m_supMenu->setToolTip(tr("Main Menu"));
    m_supMenu->setAutoRaise(true);
//    m_supMenu->setVisible(false);
    m_supMenu->setMenu(p_QupZilla->superMenu());

    m_searchLine = new WebSearchBar(p_QupZilla);

    m_navigationSplitter = new QSplitter(this);
    m_navigationSplitter->addWidget(p_QupZilla->tabWidget()->locationBars());
    m_navigationSplitter->addWidget(m_searchLine);

    m_navigationSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    m_navigationSplitter->setCollapsible(0, false);

    int splitterWidth = m_navigationSplitter->width();
    QList<int> sizes;
    sizes << (int)((double)splitterWidth * .85) << (int)((double)splitterWidth * .15);
    m_navigationSplitter->setSizes(sizes);

    m_exitFullscreen = new ToolButton();
    m_exitFullscreen->setText(tr("Exit Fullscreen"));
    m_exitFullscreen->setToolTip(tr("Exit Fullscreen"));
    m_exitFullscreen->setAutoRaise(true);
    m_exitFullscreen->setVisible(false);

    m_layout->addLayout(backNextLayout);
    m_layout->addWidget(m_reloadStop);
    m_layout->addWidget(m_buttonHome);
    m_layout->addWidget(m_buttonAddTab);
    m_layout->addWidget(m_navigationSplitter);
    m_layout->addWidget(m_supMenu);
    m_layout->addWidget(m_exitFullscreen);

    connect(m_menuBack, SIGNAL(aboutToShow()), this, SLOT(aboutToShowHistoryBackMenu()));
    connect(m_menuForward, SIGNAL(aboutToShow()), this, SLOT(aboutToShowHistoryNextMenu()));
    connect(m_buttonBack, SIGNAL(clicked()), p_QupZilla, SLOT(goBack()));
    connect(m_buttonNext, SIGNAL(clicked()), p_QupZilla, SLOT(goNext()));
    connect(m_reloadStop->buttonStop(), SIGNAL(clicked()), p_QupZilla, SLOT(stop()));
    connect(m_reloadStop->buttonReload(), SIGNAL(clicked()), p_QupZilla, SLOT(reload()));
    connect(m_buttonHome, SIGNAL(clicked()), p_QupZilla, SLOT(goHome()));
    connect(m_buttonAddTab, SIGNAL(clicked()), p_QupZilla, SLOT(addTab()));
    connect(m_exitFullscreen, SIGNAL(clicked(bool)), p_QupZilla, SLOT(fullScreen(bool)));
}

void NavigationBar::showReloadButton()
{
    m_reloadStop->showReloadButton();
}

void NavigationBar::showStopButton()
{
    m_reloadStop->showStopButton();
}

void NavigationBar::aboutToShowHistoryBackMenu()
{
    if (!m_menuBack || !p_QupZilla->weView())
        return;
    m_menuBack->clear();
    QWebHistory* history = p_QupZilla->weView()->history();
    int curindex = history->currentItemIndex();
    int count = 0;

    for (int i = curindex-1; i >= 0; i--) {
        QWebHistoryItem item = history->itemAt(i);
        if (item.isValid()) {
            QString title = item.title();
            if (title.length() > 40) {
                title.truncate(40);
                title += "..";
            }
            QAction* action = m_menuBack->addAction(_iconForUrl(item.url()),title, this, SLOT(goAtHistoryIndex()));
            action->setData(i);
            count++;
        }
        if (count == 20)
            break;
    }
}

void NavigationBar::aboutToShowHistoryNextMenu()
{
    if (!m_menuForward || !p_QupZilla->weView())
        return;
    m_menuForward->clear();
    QWebHistory* history = p_QupZilla->weView()->history();
    int curindex = history->currentItemIndex();
    int count = 0;

    for (int i = curindex+1; i < history->count(); i++) {
        QWebHistoryItem item = history->itemAt(i);
        if (item.isValid()) {
            QString title = item.title();
            if (title.length() > 40) {
                title.truncate(40);
                title += "..";
            }
            QAction* action = m_menuForward->addAction(_iconForUrl(item.url()),title, this, SLOT(goAtHistoryIndex()));
            action->setData(i);
            count++;
        }
        if (count == 20)
            break;
    }
}

void NavigationBar::goAtHistoryIndex()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        p_QupZilla->weView()->page()->history()->goToItem(p_QupZilla->weView()->page()->history()->itemAt(action->data().toInt()));
    }
    refreshHistory();
}

void NavigationBar::refreshHistory()
{
    if (mApp->isClosing())
        return;

    QWebHistory* history = p_QupZilla->weView()->page()->history();
    m_buttonBack->setEnabled(history->canGoBack());
    m_buttonNext->setEnabled(history->canGoForward());
}

NavigationBar::~NavigationBar()
{
    delete m_buttonBack;
    delete m_buttonNext;
    delete m_buttonHome;
    delete m_reloadStop;
    delete m_exitFullscreen;
    delete m_searchLine;
    delete m_navigationSplitter;
}
