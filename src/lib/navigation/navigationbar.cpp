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
#include "navigationbar.h"
#include "toolbutton.h"
#include "qupzilla.h"
#include "mainapplication.h"
#include "iconprovider.h"
#include "websearchbar.h"
#include "reloadstopbutton.h"
#include "webhistorywrapper.h"
#include "enhancedmenu.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "qzsettings.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QWebHistory>
#include <QMouseEvent>

QString NavigationBar::titleForUrl(QString title, const QUrl &url)
{
    if (title.isEmpty()) {
        title = url.toString(QUrl::RemoveFragment);
    }
    if (title.isEmpty()) {
        return NavigationBar::tr("No Named Page");
    }
    if (title.length() > 40) {
        title.truncate(40);
        title += "..";
    }

    return title;
}

QIcon NavigationBar::iconForPage(const QUrl &url, const QIcon &sIcon)
{
    QIcon icon;
    icon.addPixmap(url.scheme() == QLatin1String("qupzilla") ? QIcon(":icons/qupzilla.png").pixmap(16, 16) : _iconForUrl(url).pixmap(16, 16));
    icon.addPixmap(sIcon.pixmap(16, 16), QIcon::Active);
    return icon;
}

NavigationBar::NavigationBar(QupZilla* mainClass)
    : QWidget(mainClass)
    , p_QupZilla(mainClass)
{
    setObjectName("navigationbar");
    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(3);
    m_layout->setSpacing(3);
    setLayout(m_layout);

    m_buttonBack = new ToolButton(this);
    m_buttonBack->setObjectName("navigation-button-back");
    m_buttonBack->setToolTip(tr("Back"));
    m_buttonBack->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonBack->setAutoRaise(true);
    m_buttonBack->setEnabled(false);
    m_buttonBack->setFocusPolicy(Qt::NoFocus);

    m_buttonNext = new ToolButton(this);
    m_buttonNext->setObjectName("navigation-button-next");
    m_buttonNext->setToolTip(tr("Forward"));
    m_buttonNext->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonNext->setAutoRaise(true);
    m_buttonNext->setEnabled(false);
    m_buttonNext->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* backNextLayout = new QHBoxLayout();
    backNextLayout->setContentsMargins(0, 0, 0, 0);
    backNextLayout->setSpacing(0);
    backNextLayout->addWidget(m_buttonBack);
    backNextLayout->addWidget(m_buttonNext);

    m_reloadStop = new ReloadStopButton(this);

    m_buttonHome = new ToolButton(this);
    m_buttonHome->setObjectName("navigation-button-home");
    m_buttonHome->setToolTip(tr("Home"));
    m_buttonHome->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonHome->setAutoRaise(true);
    m_buttonHome->setFocusPolicy(Qt::NoFocus);

    m_buttonAddTab = new ToolButton(this);
    m_buttonAddTab->setObjectName("navigation-button-addtab");
    m_buttonAddTab->setToolTip(tr("New Tab"));
    m_buttonAddTab->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonAddTab->setAutoRaise(true);
    m_buttonAddTab->setFocusPolicy(Qt::NoFocus);

    m_menuBack = new Menu(this);
    m_buttonBack->setMenu(m_menuBack);
    m_menuForward = new Menu(this);
    m_buttonNext->setMenu(m_menuForward);

#ifndef Q_OS_MAC
    m_supMenu = new ToolButton(this);
    m_supMenu->setObjectName("navigation-button-supermenu");
    m_supMenu->setPopupMode(QToolButton::InstantPopup);
    m_supMenu->setToolTip(tr("Main Menu"));
    m_supMenu->setAutoRaise(true);
    m_supMenu->setFocusPolicy(Qt::NoFocus);
    m_supMenu->setMenu(p_QupZilla->superMenu());
#endif

    m_searchLine = new WebSearchBar(p_QupZilla);

    m_navigationSplitter = new QSplitter(this);
    m_navigationSplitter->addWidget(p_QupZilla->tabWidget()->locationBars());
    m_navigationSplitter->addWidget(m_searchLine);

    m_navigationSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    m_navigationSplitter->setCollapsible(0, false);

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
#ifndef Q_OS_MAC
    m_layout->addWidget(m_supMenu);
#endif
    m_layout->addWidget(m_exitFullscreen);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));

    connect(m_menuBack, SIGNAL(aboutToShow()), this, SLOT(aboutToShowHistoryBackMenu()));
    connect(m_menuForward, SIGNAL(aboutToShow()), this, SLOT(aboutToShowHistoryNextMenu()));
    connect(m_buttonBack, SIGNAL(clicked()), this, SLOT(goBack()));
    connect(m_buttonBack, SIGNAL(middleMouseClicked()), this, SLOT(goBackInNewTab()));
    connect(m_buttonBack, SIGNAL(controlClicked()), this, SLOT(goBackInNewTab()));
    connect(m_buttonNext, SIGNAL(clicked()), this, SLOT(goForward()));
    connect(m_buttonNext, SIGNAL(middleMouseClicked()), this, SLOT(goForwardInNewTab()));
    connect(m_buttonNext, SIGNAL(controlClicked()), this, SLOT(goForwardInNewTab()));

    connect(m_reloadStop->buttonStop(), SIGNAL(clicked()), p_QupZilla, SLOT(stop()));
    connect(m_reloadStop->buttonReload(), SIGNAL(clicked()), p_QupZilla, SLOT(reload()));
    connect(m_buttonHome, SIGNAL(clicked()), p_QupZilla, SLOT(goHome()));
    connect(m_buttonHome, SIGNAL(middleMouseClicked()), p_QupZilla, SLOT(goHomeInNewTab()));
    connect(m_buttonHome, SIGNAL(controlClicked()), p_QupZilla, SLOT(goHomeInNewTab()));
    connect(m_buttonAddTab, SIGNAL(clicked()), p_QupZilla, SLOT(addTab()));
    connect(m_exitFullscreen, SIGNAL(clicked(bool)), p_QupZilla, SLOT(fullScreen(bool)));
}

void NavigationBar::setSplitterSizes(int locationBar, int websearchBar)
{
    QList<int> sizes;

    if (locationBar == 0) {
        int splitterWidth = m_navigationSplitter->width();
        sizes << (int)((double)splitterWidth * .80) << (int)((double)splitterWidth * .20);
    }
    else {
        sizes << locationBar << websearchBar;
    }

    m_navigationSplitter->setSizes(sizes);
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
    if (!m_menuBack || !p_QupZilla->weView()) {
        return;
    }
    m_menuBack->clear();
    QWebHistory* history = p_QupZilla->weView()->history();

    int curindex = history->currentItemIndex();
    int count = 0;
    QUrl lastUrl = history->currentItem().url();

    for (int i = curindex - 1; i >= 0; i--) {
        QWebHistoryItem item = history->itemAt(i);
        if (item.isValid() && lastUrl != item.url()) {
            QString title = titleForUrl(item.title(), item.url());

            const QIcon &icon = iconForPage(item.url(), qIconProvider->standardIcon(QStyle::SP_ArrowBack));
            Action* act = new Action(icon, title);
            act->setData(i);
            connect(act, SIGNAL(triggered()), this, SLOT(goAtHistoryIndex()));
            connect(act, SIGNAL(middleClicked()), this, SLOT(goAtHistoryIndexInNewTab()));
            m_menuBack->addAction(act);

            lastUrl = item.url();
        }

        count++;
        if (count == 20) {
            break;
        }
    }

    m_menuBack->addSeparator();
    m_menuBack->addAction(tr("Clear history"), this, SLOT(clearHistory()));
}

void NavigationBar::aboutToShowHistoryNextMenu()
{
    if (!m_menuForward || !p_QupZilla->weView()) {
        return;
    }
    m_menuForward->clear();

    QWebHistory* history = p_QupZilla->weView()->history();
    int curindex = history->currentItemIndex();
    int count = 0;
    QUrl lastUrl = history->currentItem().url();

    for (int i = curindex + 1; i < history->count(); i++) {
        QWebHistoryItem item = history->itemAt(i);
        if (item.isValid() && lastUrl != item.url()) {
            QString title = titleForUrl(item.title(), item.url());

            const QIcon &icon = iconForPage(item.url(), qIconProvider->standardIcon(QStyle::SP_ArrowForward));
            Action* act = new Action(icon, title);
            act->setData(i);
            connect(act, SIGNAL(triggered()), this, SLOT(goAtHistoryIndex()));
            connect(act, SIGNAL(middleClicked()), this, SLOT(goAtHistoryIndexInNewTab()));
            m_menuForward->addAction(act);

            lastUrl = item.url();
        }

        count++;
        if (count == 20) {
            break;
        }
    }

    m_menuForward->addSeparator();
    m_menuForward->addAction(tr("Clear history"), this, SLOT(clearHistory()));
}

void NavigationBar::clearHistory()
{
    QWebHistory* history = p_QupZilla->weView()->page()->history();
    history->clear();
    refreshHistory();
}

void NavigationBar::contextMenuRequested(const QPoint &pos)
{
    p_QupZilla->popupToolbarsMenu(mapToGlobal(pos));
}

void NavigationBar::goAtHistoryIndex()
{
    QWebHistory* history = p_QupZilla->weView()->page()->history();

    if (QAction* action = qobject_cast<QAction*>(sender())) {
        history->goToItem(history->itemAt(action->data().toInt()));
    }

    refreshHistory();
}

void NavigationBar::goAtHistoryIndexInNewTab(int index)
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        index = action->data().toInt();
    }

    if (index == -1) {
        return;
    }

    TabWidget* tabWidget = p_QupZilla->tabWidget();
    int tabIndex = tabWidget->duplicateTab(tabWidget->currentIndex());

    QWebHistory* history = p_QupZilla->weView(tabIndex)->page()->history();
    history->goToItem(history->itemAt(index));

    if (qzSettings->newTabPosition == Qz::NT_SelectedTab) {
        tabWidget->setCurrentIndex(tabIndex);
    }
}

void NavigationBar::refreshHistory()
{
    if (mApp->isClosing() || p_QupZilla->isClosing()) {
        return;
    }

    QWebHistory* history = p_QupZilla->weView()->page()->history();
    m_buttonBack->setEnabled(history->canGoBack());
    m_buttonNext->setEnabled(history->canGoForward());
}

void NavigationBar::goBack()
{
    QWebHistory* history = p_QupZilla->weView()->page()->history();
    history->back();
}

void NavigationBar::goBackInNewTab()
{
    QWebHistory* history = p_QupZilla->weView()->page()->history();

    if (!history->canGoBack()) {
        return;
    }

    int itemIndex = WebHistoryWrapper::indexOfItem(history->items(), history->backItem());
    if (itemIndex == -1) {
        return;
    }

    goAtHistoryIndexInNewTab(itemIndex);
}

void NavigationBar::goForward()
{
    QWebHistory* history = p_QupZilla->weView()->page()->history();
    history->forward();
}

void NavigationBar::goForwardInNewTab()
{
    QWebHistory* history = p_QupZilla->weView()->page()->history();

    if (!history->canGoForward()) {
        return;
    }

    int itemIndex = WebHistoryWrapper::indexOfItem(history->items(), history->forwardItem());
    if (itemIndex == -1) {
        return;
    }

    goAtHistoryIndexInNewTab(itemIndex);
}
